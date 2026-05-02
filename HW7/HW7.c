#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define PIN_CS 17
#define SPI_PORT 19

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void writeDAC(int channel, float voltage) {
    uint8_t data[2];

    
    data[0] = 0b01110000;

    data[0] = data[0] | (channel&0b1) << 7; // set the channel bit

    uint16_t myV = voltage / 3.3 * 1023; // convert voltage to 10-bit value (0-1023)

    data[0] = data[0] | (myV >> 6) & 0b00001111; // set the upper 2 bits of the voltage

    data[1] = (myV<<2) & 0xFF; // set the lower 6 bits of the voltage, shifted to the left by 2

    // data[1] = 0b11111100;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}

int main()
{
    stdio_init_all();

    spi_init(spi_default, 12 * 1000); // the baud, or bits per second
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    int channel1 = 0;
    int channel2 = 1;

    while (true) {
        // call writeDAC
        static float t = 0;
        t += 0.01;
        float voltage = (sin(2*M_PI*2*t)+1)/2.0*3.3;
        writeDAC(channel1, voltage);
        float voltage2 = (abs(2*(t/1 - floor(t/1 + 0.5)))) * 3.3;
        writeDAC(channel2, voltage2);
        sleep_ms(10);
    }
}