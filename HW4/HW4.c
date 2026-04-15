#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "pico/cyw43_arch.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13

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

int main()
{
    stdio_init_all();
    cyw43_arch_init();
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 800*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        uint16_t adc_value = adc_read();
        char message[50];
        sprintf(message, "ADC Value: %d", adc_value);
        drawString(0, 0, message);
        frames_per_second();
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
    // ssd1306_update();
}

void drawString(unsigned char x, unsigned char y, char* string) {
    for (int ii = 0; string[ii] != '\0'; ii++){
        drawChar(x + ii*6, y, string[ii]);
    }
}
