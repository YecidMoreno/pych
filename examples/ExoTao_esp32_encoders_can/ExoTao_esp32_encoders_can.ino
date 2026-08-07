#include "esp32-hal-log.h"
#include "abs_encoder.h"
#include "inc_encoder.h"


#define SEND_OVER_SERIAL 0
#define SEND_OVER_CAN 1
#define SEND_OVER_SERIAL_PLOTTER 2

// #define TX_PROTOCOL SEND_OVER_SERIAL
#define TX_PROTOCOL SEND_OVER_CAN
// #define TX_PROTOCOL SEND_OVER_SERIAL_PLOTTER

#if (TX_PROTOCOL == SEND_OVER_SERIAL)
#define WAIT_TIME_US 1000
#elif (TX_PROTOCOL == SEND_OVER_SERIAL_PLOTTER)
#define WAIT_TIME_US 1000
#elif (TX_PROTOCOL == SEND_OVER_CAN)
#include "driver/twai.h"
#define CAN_RX_PIN 4
#define CAN_TX_PIN 5
#define CAN_NODE_ID 0x10
static bool driver_installed = false;
#define WAIT_TIME_US 500000
#endif

#define N_ABS 2
gpio_num_t abs_pin_cs[] = { GPIO_NUM_27, GPIO_NUM_15 };

Abs_encoder enc_abs[N_ABS];
// Inc_encoder inc1(GPIO_NUM_2, GPIO_NUM_4, 1024.0f);

// Estructura de datos
struct __attribute__((packed)) Frame_t {
  uint8_t header = '@';
  float angles[8] = { 0 };
  uint8_t tail = '\n';
};

// Handle para la cola de FreeRTOS
QueueHandle_t tx_queue;

// Prototipo de la función que correrá en el Core 0
void telemetryTask(void* pvParameters);

void setup() {
  // Aumentamos la velocidad del Serial para evitar cuellos de botella
  Serial.begin(115200);

#if (TX_PROTOCOL == SEND_OVER_CAN)
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  //Look in the api-reference for other speed sets.
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    return;
  }

  // Reconfigure alerts to detect TX alerts and Bus-Off errors
  uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN Alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
    return;
  }

  // TWAI driver is now successfully installed and started
  driver_installed = true;
#endif

  spi_setup();
  for (int i = 0; i < N_ABS; i++) {
    enc_abs[i].begin(abs_pin_cs[i], SPI_CLOCK);
    delay(200);
    enc_abs[i].read_angle();
    // enc_abs[i].setZero();
    delay(200);
    enc_abs[i].getAngle();
    delay(200);
  }

  // Inc_encoder::activateGlobalInterruptions();
  // inc1.init_isr();

  // 1. Crear la cola para pasar datos entre núcleos (capacidad para 5 frames)
  tx_queue = xQueueCreate(5, sizeof(Frame_t));
  if (tx_queue == NULL) {
    Serial.println("Error al crear la cola");
    while (1)
      ;
  }

  // 2. Crear la tarea de telemetría en el CORE 0
  xTaskCreatePinnedToCore(
    telemetryTask,  // Función de la tarea
    "Telemetry",    // Nombre de la tarea
    4096,           // Tamaño de la pila (Stack size)
    NULL,           // Parámetros de entrada
    1,              // Prioridad de la tarea
    NULL,           // Task handle
    0               // NÚCLEO 0
  );
}

// El loop() corre por defecto en el CORE 1 en ESP32 Arduino
Frame_t local_frame;
void loop() {
  static unsigned long next = micros();
  unsigned long now = micros();
  bool can_send = false;
  bool read_encoders = false;

#if (TX_PROTOCOL == SEND_OVER_CAN)
  twai_message_t message;
  if (twai_receive(&message, 0) == ESP_OK) {
    if (message.identifier == 0x80) {
      can_send = true;
      read_encoders = true;
    }
    else if (message.identifier == 0x00) {
      for (int i = 0; i < N_ABS; i++) {
        enc_abs[i].read_angle();
        // enc_abs[i].setZero();
      }
      can_send = true;
      read_encoders = true;
    }
  }
#endif


  if (!read_encoders) {
    if ((long)(now - next) >= 0) {
      read_encoders = true;
      next += WAIT_TIME_US;
      if ((now - next) > 50) {
        delayMicroseconds(50);
      }
    }
  } else {
    next = micros() + WAIT_TIME_US;
  }

  if (!read_encoders) {
    return;
  }

  // memset(&local_frame, 0, sizeof(local_frame));

  for (int i = 0; i < N_ABS; i++) {
    enc_abs[i].read_angle();
    local_frame.angles[i] = enc_abs[i].getAngle();
  }

  if (can_send) {
#if (TX_PROTOCOL == SEND_OVER_CAN)
    xQueueSend(tx_queue, &local_frame, 0);
#endif
  }

#if ((TX_PROTOCOL == SEND_OVER_SERIAL) || (TX_PROTOCOL == SEND_OVER_SERIAL_PLOTTER))
  xQueueSend(tx_queue, &local_frame, 0);
#endif

  can_send = false;
}

// === TAREA DEL NÚCLEO 0: ENVÍO DE DATOS ===
void telemetryTask(void* pvParameters) {
  Frame_t received_frame;

  for (;;) {
    // Se bloquea aquí de forma eficiente hasta que el Core 1 envíe datos a la cola
    if (xQueueReceive(tx_queue, &received_frame, portMAX_DELAY) == pdPASS) {

#if (TX_PROTOCOL == SEND_OVER_SERIAL)
      // Envío binario puro
      Serial.write(reinterpret_cast<const uint8_t*>(&received_frame), sizeof(Frame_t));
#elif (TX_PROTOCOL == SEND_OVER_SERIAL_PLOTTER)
      // Envío en texto para Serial Plotter
      for (int i = 0; i < 8; i++) {
        Serial.printf("%f\t", received_frame.angles[i]);
      }
      Serial.print("\n");

#elif (TX_PROTOCOL == SEND_OVER_CAN)
      twai_message_t message;
      message.identifier = 0x180 + CAN_NODE_ID;
      message.data_length_code = 8;
      memcpy(&message.data[0], &received_frame.angles[0], 8);
      // Queue message for transmission
      if (twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK) {
      }
#endif
    }
  }
}