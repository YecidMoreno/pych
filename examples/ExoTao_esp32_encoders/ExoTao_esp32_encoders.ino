#include "esp32-hal-log.h"
#include "abs_encoder.h"
#include "inc_encoder.h"


// #define TO_SERIAL_PLOTTER


#ifndef TO_SERIAL_PLOTTER
  #define WAIT_TIME_US 1000
#else
  #define WAIT_TIME_US 100
#endif

#define N_ABS 2
gpio_num_t abs_pin_cs[] = {GPIO_NUM_27,GPIO_NUM_15};

Abs_encoder enc_abs[N_ABS];
Inc_encoder inc1(GPIO_NUM_2, GPIO_NUM_4, 1024.0f);

float angles[8] = { 0 };

struct __attribute__((packed)) {
  uint8_t header = '@';
  float angles[8] = { 0 };
  uint8_t tail = '\n';
} tx_frame;

void *angles_ptr;

void setup() {

  memset(angles, 0, 4 * 8);
  angles_ptr = &tx_frame;

  Serial.begin(115200);

  Serial.printf("tx_frame sz: %ld\n", sizeof(tx_frame));
  delay(1000);

  spi_setup();
  for (int i = 0; i < N_ABS; i++) {
    enc_abs[i].begin(abs_pin_cs[i], SPI_CLOCK);
    enc_abs[i].read_angle();
    enc_abs[i].setZero();
  }


  Inc_encoder::activateGlobalInterruptions();
  inc1.init_isr();
}

void loop() {

  static unsigned long next = micros();
  unsigned long now = micros();

  if ((long)(now - next) >= 0) {
    next += WAIT_TIME_US;
    if((now - next)>200){
      delayMicroseconds(200);
    }
    
  }else{
    return;
  }

  for (int i = 0; i < N_ABS; i++) {
    enc_abs[i].read_angle();
    tx_frame.angles[i] = enc_abs[i].getAngle();
  }
   
  tx_frame.angles[6] = inc1.getAngle();

#ifndef TO_SERIAL_PLOTTER
  Serial.write(static_cast<const char *>(angles_ptr), sizeof(tx_frame));
  // delay(1);
#else
  for (int i = 0; i < 8; i++) {
    Serial.printf("%f\t", tx_frame.angles[i]);
  }
  // Serial.printf("%f\t%f\n",1.0f,-1.0f);
  Serial.print("\n");
  delay(100);
#endif
}
