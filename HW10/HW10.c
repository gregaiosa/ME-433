#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MPU_6050.h"

#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13
#define BUTTON_PIN 15
#define MPU_ADDR 0x68

void mpu6050_init() {
    uint8_t buf[2];
    // Wake up MPU6050 (write 0 to PWR_MGMT_1)
    buf[0] = PWR_MGMT_1;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    uint8_t val = ACCEL_XOUT_H;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &val, 1, true); // true to keep master control of bus
    
    uint8_t buffer[14];
    i2c_read_blocking(I2C_PORT, MPU_ADDR, buffer, 14, false);

    accel[0] = (buffer[0] << 8) | buffer[1];
    accel[1] = (buffer[2] << 8) | buffer[3];
    accel[2] = (buffer[4] << 8) | buffer[5];
    *temp = (buffer[6] << 8) | buffer[7];
    gyro[0] = (buffer[8] << 8) | buffer[9];
    gyro[1] = (buffer[10] << 8) | buffer[11];
    gyro[2] = (buffer[12] << 8) | buffer[13];
}

int main() {
    stdio_init_all();

    // Init I2C at 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Init button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN); // Internal pull-up

    // Small delay to allow IMU to power up
    sleep_ms(100);
    mpu6050_init();

    int16_t accel[3], gyro[3], temp;

    while (true) {
        mpu6050_read_raw(accel, gyro, &temp);
        
        // Button is connected to ground, so 0 means pressed, 1 means released.
        // We invert it so 1 = pressed, 0 = released
        int button_state = gpio_get(BUTTON_PIN) ? 0 : 1;
        
        // Output format: accel_x,accel_y,button_state
        printf("%d,%d,%d\n", accel[0], accel[1], button_state);
        
        sleep_ms(20); // 50 Hz refresh rate
    }
}
