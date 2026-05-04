#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define PIN_CS_DAC 17
#define PIN_CS_RAM 9
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_MOSI 19
#define PIN_SCK 18

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

void update_dac_from_ram(int);

void spi_ram_init();
void spi_ram_write(uint16_t, uint8_t *, int);
void spi_ram_read(uint16_t, uint8_t *, int);

void ram_write_sine();

void writeDAC(int channel, float voltage) {
    uint8_t data[2];

    
    data[0] = 0b01110000;

    data[0] = data[0] | (channel&0b1) << 7; // set the channel bit

    uint16_t myV = voltage / 3.3 * 1023; // convert voltage to 10-bit value (0-1023)

    data[0] = data[0] | (myV >> 6) & 0b00001111; // set the upper 2 bits of the voltage

    data[1] = (myV<<2) & 0xFF; // set the lower 6 bits of the voltage, shifted to the left by 2

    // data[1] = 0b11111100;

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_DAC);
}

int main()
{
    stdio_init_all();

    spi_init(spi_default, 1000 * 1000 * 20); // the baud, or bits per second
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Initialize the CS pin as an output and set it high (deselected) by default
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);

    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    spi_ram_init();
    ram_write_sine();

    int i = 0;

    while (true) {
        for (i = 0; i < 1024; i++) {
            update_dac(0, (float)i * 3.3 /1024);
            sleep_ms(1);
        }

        for (i=0; i <1024*2; i=i+2){
            update_dac_from_ram(i);
            sleep_ms(1);
        }
     }
}

void spi_ram_init() {
    uint8_t data[2];
    int len = 2;
    data[0] = 0b00000001;
    data[1] = 0b01000000; // sequential mode
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_write(uint16_t addr, uint8_t * data, int len) {
    uint8_t packet[5];
    packet[0] = 0b00000010; // instruction, write
    packet[1] = addr>>8; // addrr
    packet[2] = addr&0xFF; // addr
    packet[3] = data[0];
    packet[4] = data[1];

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, packet, 5); // where data is
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_read(uint16_t addr, uint8_t * data, int len) {
    uint8_t packet[5];
    packet[0] = 0b00000011; // instruction, read
    packet[1] = addr<<8; // addrr
    packet[2] = addr&0xFF; // addr
    packet[3] = 0;
    packet[4] = 0;

    uint8_t dst[5];

    cs_select(PIN_CS_RAM);
    spi_write_read_blocking(SPI_PORT, packet, dst, 5); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_RAM);
    data[0] = dst[3];
    data[1] = dst[4];
}

void ram_write_sine() {
    int i = 0;
    uint8_t data[2];
    uint16_t data_short = 0;
    uint8_t channel = 0b0;
    float voltage = 0;
    uint16_t addr = 0;

    for (i=0; i<1024; i++){
        data_short = (channel&0b1)<<15;
        data_short = data_short | (0b111<<12);

        voltage = (sin(2*M_PI*i/1024.0)+1)*512;

        uint16_t v = voltage;

        data_short = data_short | (0b111111111111 & v);

        data[0] = data_short >> 8;
        data[1] = data_short & 0xFF;

        spid_ram_write(addr, data, 2);
        addr = addr + 2;
    }
}

void update_dac(uint8_t channel, float voltage) {
    uint8_t data[2];
    uint16_t data_short = 0;

    data_short = (channel&0b1)<<15;
    data_short = data_short | (0b111<<12);

    uint16_t v = voltage / 3.3 * 1024;

    data_short = data_short | (v << 2);

    data[0] = data_short >> 8;
    data[1] = data_short & 0xFF;

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a
    cs_deselect(PIN_CS_DAC);
}

void update_dac_from_ram(int i){
    uint8_t data[2];
    spi_ram_read(i, data, 2);

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2); // where data is a
    cs_deselect(PIN_CS_DAC);
}