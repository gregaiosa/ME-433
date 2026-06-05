#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define ENCODER_ADDR 0x36

#define SCK_PIN 3
#define DT_PIN 2



#define DEG_MULTIPLIER (360.0f / 4096.0f) 

void setup_i2c() {
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void hx711_init(void) {
    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_init(DT_PIN);
    gpio_set_dir(DT_PIN, GPIO_IN);
}

uint16_t encoder_read_angle() {
    uint8_t reg = 0x0C; // 
    uint8_t buf[2];

    i2c_write_blocking(I2C_PORT, ENCODER_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, ENCODER_ADDR, buf, 2, false);

    uint16_t angle = (buf[0] << 8) | buf[1];

    return angle;
}

int32_t hx711_read(void) {
    while (gpio_get(DT_PIN) == 1) {
        // wait until DT pin is low
    }
    
    int32_t count = 0;
    for (int i = 0; i < 24; i++) {
        gpio_put(SCK_PIN, 1);
        busy_wait_us(1); // Small delay for clock to settle
        count = count << 1;
        if (gpio_get(DT_PIN)) {
            count++;
        }
        gpio_put(SCK_PIN, 0);
        busy_wait_us(1);
    }
    
    // 25th pulse for gain of 128
    gpio_put(SCK_PIN, 1);
    busy_wait_us(1);
    gpio_put(SCK_PIN, 0);
    busy_wait_us(1);
    
    // sign-extend 24-bit two's complement to 32-bit signed int
    if (count & 0x800000) { 
        count |= 0xFF000000; 
    }
    return count;
}

float to_degrees(uint16_t raw_value) {
    return (float)raw_value * DEG_MULTIPLIER;
}

float to_radians(uint16_t raw_value) {
    return ((float)raw_value / 4096.0f) * (2.0f * M_PI);
}

int main()
{
    stdio_init_all();
    setup_i2c();
    hx711_init();

    float filter_val = 0.0f;
    bool is_first_sample = true;

    while (true) {
        uint16_t angle = encoder_read_angle();
        angle = to_degrees(angle);

        int32_t raw_force = hx711_read();

            if (is_first_sample) {
                filter_val = raw_force;
                is_first_sample = false;
            } else {
                filter_val = 0.9f * filter_val + 0.1f * raw_force;
            }

        printf("Angle: %u\t Force: %d\r\n", angle, (int)filter_val);
    }
}
