#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "ssd1306.h"
#include "font.h"
#include "MPU_6050.h"

#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13

#define IMU_ADDR 0b1101000
#define X_CENTER 64
#define Y_CENTER 16

#define HEARTBEAT_PIN CYW43_WL_GPIO_LED_PIN

struct repeating_timer timer;

static unsigned int last_time = 0;

bool repeating_timer_callback(struct repeating_timer *t) {
    static bool led_state = false;
    led_state = !led_state;
    cyw43_arch_gpio_put(HEARTBEAT_PIN, led_state);
    return true; // keep repeating
}

void frames_per_second() {
    unsigned int current_time = to_us_since_boot(get_absolute_time());

    char message[10];
    unsigned int fps = 1000000 / (current_time - last_time);
    sprintf(message, "FPS: %d", fps);
    drawString(0, 24, message);
    last_time = current_time;
}

unsigned char read_register(unsigned char reg) {
    unsigned char data;
    i2c_write_blocking(I2C_PORT, IMU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, IMU_ADDR, &data, 1, false);
    return data;
}

int16_t get_data(unsigned char reg) {
    uint8_t buffer[2];

    i2c_write_blocking(I2C_PORT, IMU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, IMU_ADDR, buffer, 2, false);
    return (int16_t)((buffer[0] << 8) | buffer[1]);
}

void read_all_sensors(float accel[3], float gyro[3], float *temp) {
    uint8_t buffer[14];

    unsigned char start_reg = ACCEL_XOUT_H; // 0x3B
    i2c_write_blocking(I2C_PORT, IMU_ADDR, &start_reg, 1, true);
    i2c_read_blocking(I2C_PORT, IMU_ADDR, buffer, 14, false);

    // Reconstruct Accelerometer Data
    accel[0] = (float)((int16_t)((buffer[0] << 8) | buffer[1])) * 0.000061;  // X in G's
    accel[1] = (float)((int16_t)((buffer[2] << 8) | buffer[3])) * 0.000061;  // Y in G's
    accel[2] = (float)((int16_t)((buffer[4] << 8) | buffer[5])) * 0.000061;  // Z in G's
    
    // Reconstruct temperature data
    int16_t raw_temp = (buffer[6] << 8) | buffer[7];
    *temp = ((float)raw_temp / 340.0) + 36.53;

    // Reconstruct Gyroscope Data
    gyro[0] = (float)((int16_t)((buffer[8] << 8)  | buffer[9])) * 0.007630 * 8.0;   // X in deg/s
    gyro[1] = (float)((int16_t)((buffer[10] << 8) | buffer[11])) * 0.007630 * 8.0;  // Y in deg/s
    gyro[2] = (float)((int16_t)((buffer[12] << 8) | buffer[13])) * 0.007630 * 8.0;  // Z in deg/s
}


void write_register(unsigned char reg, unsigned char value) {
    unsigned char data[2];
    data[0] = reg;
    data[1] = value;
    i2c_write_blocking(I2C_PORT, IMU_ADDR, data, 2, false);
}

void init_imu() {
    write_register(0x6B, 0x00); // Turn on the chip
    write_register(0x1C, 0x00); // Set Accel to +/- 2g
    write_register(0x1B, 0x18); // Set Gyro to +/- 2000 dps
}

int main()
{
    stdio_init_all();
    cyw43_arch_init();
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);


    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    init_imu();

    float accel[3], gyro[3];
    float temp;
    while (true) {
        ssd1306_clear();

        read_all_sensors(accel, gyro, &temp);
        int x_length = -1*(int)(accel[0] * 64.0);
        int y_length = (int)(accel[1] * 16.0);

        if (x_length >= 0) {
            for (int ii = 0; ii < x_length; ii++) {
               ssd1306_drawPixel(X_CENTER + ii, Y_CENTER, 1);
            }
        }
        else {
            for (int ii = 0; ii > x_length; ii--) {
               ssd1306_drawPixel(X_CENTER + ii, Y_CENTER, 1);
            }
        }

        if (y_length >= 0) {
            for (int ii = 0; ii < y_length; ii++) {
               ssd1306_drawPixel(X_CENTER, Y_CENTER + ii, 1);
            }
        }
        else {
            for (int ii = 0; ii > y_length; ii--) {
               ssd1306_drawPixel(X_CENTER, Y_CENTER + ii, 1);
            }
        }

        ssd1306_update();
    }
}

void drawChar(unsigned char x, unsigned char y, unsigned char letter) {
    unsigned int row_count = 0;
    for (int ii = 0; ii < 5; ii++) {
        for (unsigned char mask = 0x01; mask != 0; mask<<= 1) {
            if (ASCII[letter-32][ii] & mask) {
                ssd1306_drawPixel(x + ii, y + row_count, 1);   
            }
            else {
                ssd1306_drawPixel(x + ii, y + row_count, 0);
            }
            row_count++;
        }
        row_count = 0;
    }
}

void drawString(unsigned char x, unsigned char y, char* string) {
    for (int ii = 0; string[ii] != '\0'; ii++){
        drawChar(x + ii*6, y, string[ii]);
    }
}
