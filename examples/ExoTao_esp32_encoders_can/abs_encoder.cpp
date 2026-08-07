#include "HardwareSerial.h"
#include "esp_err.h"
#include "abs_encoder.h"
#include <cstring>

Abs_encoder::Abs_encoder(){

}
void Abs_encoder::print_status(){
    ESP_LOGI("Abs_encoder", "CYCLES: %d", encoder_cycles);
    ESP_LOGI("Abs_encoder", "buffer: %d", raw_data);  
    ESP_LOGI("Abs_encoder","Angle: %f",angle);
}
void Abs_encoder::begin(gpio_num_t _cs, long int _spi_freq){

      this->cs = _cs;
    this->spi_freq= _spi_freq;

    // set device configuration: 
    dev_config.command_bits = 0;
    dev_config.address_bits = 8;
    dev_config.dummy_bits = 0;
    dev_config.mode = 1;
    dev_config.duty_cycle_pos = 128;  // default 128 = 50%/50% duty
    dev_config.cs_ena_pretrans = 0;  // 0 not used
    dev_config.cs_ena_posttrans = 0;  // 0 not used
    dev_config.clock_speed_hz = this->spi_freq;
    dev_config.spics_io_num = this->cs;
    dev_config.flags = SPI_DEVICE_NO_DUMMY|SPI_DEVICE_HALFDUPLEX;  // 0 not used
    dev_config.queue_size = 1;
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;
    spi_bus_add_device(host, &dev_config, &device);

    // set transmission configuration
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = 0x00 | 0x80 ;
    transaction.length = 2 * 8;
    transaction.rxlength = 2 * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = (void*) &raw_data;

    ENABLE = true;
    
}
bool Abs_encoder::read_raw(){
    return spi_device_transmit(device, &transaction)==ESP_OK;
}
float Abs_encoder::read_angle(){
    if(!ENABLE) return 0.0f;
    
    this->read_raw();

    raw_data = (~0x8000) & (int)raw_data; 

    // if(raw_data == 0){
    //     raw_data = angle_buffer[0];
    // }

    // 1. Calcular el ángulo de la muestra actual (-PI a PI)
    angle_aux = (raw_data * 2.0f * M_PI / MAX_RESOLUTION) - M_PI;

    // 2. DETECCIÓN DE VUELTAS
    angle_buffer[1] = angle_buffer[0]; // Ángulo crudo anterior
    angle_buffer[0] = angle_aux;       // Ángulo crudo actual

    if (angle_buffer[0] < -M_PI/2.0f && angle_buffer[1] > M_PI/2.0f) {
        encoder_cycles++;
    }
    else if (angle_buffer[0] > M_PI/2.0f && angle_buffer[1] < -M_PI/2.0f) {
        encoder_cycles--;
    }

    // 3. Calcular el ángulo total candidato
    float candidato_angle = angle_aux + (float)encoder_cycles * (2.0f * M_PI);

    // 4. FILTRO DE RUIDO CON CONFIRMACIÓN DE REPETICIÓN
    if (primera_lectura) {
        angle = candidato_angle;
        candidato_anterior = candidato_angle; // Guardamos el candidato para comparar después
        primera_lectura = false;
        contador_repetidos = 0;
    } else {
        float diferencia = fabs(candidato_angle - angle);
        
        if (diferencia <= NOISE_THRESHOLD) {
            // Caso Normal: El movimiento es suave y válido.
            angle = candidato_angle;
            contador_repetidos = 0; // Reseteamos el contador de anomalías
        } else {
            // Caso Anómalo: El salto es mayor a 10 grados. ¿Es ruido o movimiento real?
            
            // Revisamos si este candidato sospechoso es IGUAL al candidato de la lectura anterior.
            // Nota: En floats, usar fabs(a - b) < 0.001f es más seguro que "a == b" por precisión.
            if (fabs(candidato_angle - candidato_anterior) < 0.001f) {
                contador_repetidos++;
            } else {
                contador_repetidos = 1; // Es un valor anómalo nuevo
            }

            // SI SE HA REPETIDO DURANTE LAS ÚLTIMAS 2 LECTURAS (la anterior y esta):
            if (contador_repetidos >= 1) { 
                // Lo aceptamos como un valor real y actualizamos el ángulo.
                angle = candidato_angle;
                contador_repetidos = 0; // Reseteamos
            } else {
                // PRIMERA VEZ QUE APARECE: Lo tratamos temporalmente como ruido.
                // Mantenemos el 'angle' anterior y protegemos el buffer crudo.
                angle_buffer[0] = angle_buffer[1]; 
            }
        }
        
        // Guardamos este candidato para la comparativa del próximo ciclo
        candidato_anterior = candidato_angle; 
    }

    return angle;
}

void Abs_encoder::setZero(){
    angle_0 = angle;
}

float Abs_encoder::getAngle(){
    return angle-angle_0;
}

void spi_setup(){
    spi_bus_config_t config;
    memset(&config, 0, sizeof(spi_bus_config_t));
    config.mosi_io_num = MOSI_PIN;
    config.miso_io_num = MISO_PIN;
    config.sclk_io_num = SCLK_PIN;
    config.quadwp_io_num = -1;  // -1 not used
    config.quadhd_io_num = -1;  // -1 not used
    config.max_transfer_sz = 16;
    spi_bus_initialize(host, &config, SPI_DMA_DISABLED);
}
