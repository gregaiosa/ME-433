#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"


// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13

#define HEARTBEAT_PIN CYW43_WL_GPIO_LED_PIN

#define ADDR 0x20
#define REG_IODIR 0x00
#define REG_OLAT 0x0A
#define REG_GPIO 0x09


void setPin(unsigned char address, unsigned char reg, unsigned char value)
{
    unsigned char buf[2] = {reg, value};
    i2c_write_blocking(i2c_default, address, buf, 2, false);
}

unsigned char readPin(unsigned char address, unsigned char reg)
{
    sleep_ms(1);
    i2c_write_blocking(i2c_default, address, &reg, 1, true);
    unsigned char buf = 0;
    i2c_read_blocking(i2c_default, address, &buf, 1, false);
    
    return buf;
}

int main()
{
    stdio_init_all();

    cyw43_arch_init();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);  // ON
    sleep_ms(80);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);  // OFF
    sleep_ms(80);

    setPin(ADDR, REG_IODIR, 0x7F);
    // setPin(ADDR, REG_IODIR, 0x00);
    sleep_ms(10);
    setPin(ADDR, REG_OLAT, 0x80);
    // setPin(ADDR, REG_IODIR, 0x01);

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);  // ON
        sleep_ms(80);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);  // OFF
        sleep_ms(80);

        unsigned char buf = readPin(ADDR, REG_GPIO);
        if (buf & 0x01) {
            printf("1");
            setPin(ADDR, REG_OLAT, 0x00);
        }
        else {
            printf("0");
            setPin(ADDR, REG_OLAT, 0x80);
        }
    }
}
