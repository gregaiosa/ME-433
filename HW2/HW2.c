#include <stdio.h> // set pico_enable_stdio_usb to 1 in CMakeLists.txt 
#include "pico/stdlib.h" // CMakeLists.txt must have pico_stdlib in target_link_libraries
#include "hardware/pwm.h" // CMakeLists.txt must have hardware_pwm in target_link_libraries

#define PWMPIN 16
#define PWMDIV 60 // must be between 1-255
#define WRAP 50000 // when to rollover, must be less than 65535

#define DUTY_MIN 0.024
#define DUTY_MAX 0.128

void set_angle(float angle) {
    float duty = (angle / 180.0) * (DUTY_MAX - DUTY_MIN) + DUTY_MIN; // Map angle to duty cycle
    pwm_set_gpio_level(PWMPIN, duty * WRAP); // Set the PWM
}

int main()
{
    stdio_init_all();

    gpio_set_function(PWMPIN, GPIO_FUNC_PWM); // Set the Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(PWMPIN); // Get PWM slice number
    pwm_set_clkdiv(slice_num, PWMDIV); // sets the clock speed
    pwm_set_wrap(slice_num, WRAP); 
    pwm_set_enabled(slice_num, true); // turn on the PWM

    pwm_set_gpio_level(PWMPIN, WRAP/2); // set the duty cycle to 50%

    while (true) {
        for (float angle = 0.0; angle  <= 180.0; angle += 1.0) {
            set_angle(angle);
            sleep_ms(20);
        }
        for (float angle = 180.0; angle >= 0.0; angle -= 1.0) {
            set_angle(angle);
            sleep_ms(20);
        }
    }
}