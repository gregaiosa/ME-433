#include <stdio.h> // set pico_enable_stdio_usb to 1 in CMakeLists.txt 
#include "pico/stdlib.h" // CMakeLists.txt must have pico_stdlib in target_link_libraries
#include "hardware/pwm.h" // CMakeLists.txt must have hardware_pwm in target_link_libraries

#define PWMPIN 16
#define PWMDIV 60 // must be between 1-255
#define WRAP 50000 // when to rollover, must be less than 65535

#define DUTY_MIN 0.024
#define DUTY_MAX 0.128

int main()
{
    stdio_init_all();

    // turn on the pwm, in this example to 10kHz with a resolution of 1500
    gpio_set_function(PWMPIN, GPIO_FUNC_PWM); // Set the Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(PWMPIN); // Get PWM slice number
    pwm_set_clkdiv(slice_num, PWMDIV); // sets the clock speed
    pwm_set_wrap(slice_num, WRAP); 
    pwm_set_enabled(slice_num, true); // turn on the PWM

    pwm_set_gpio_level(PWMPIN, WRAP/2); // set the duty cycle to 50%

    while (true) {
        for (float duty = DUTY_MIN; duty <= DUTY_MAX; duty += 0.01) {
            pwm_set_gpio_level(PWMPIN, duty * WRAP); // set the duty cycle
            sleep_ms(100);
        }
        for (float duty = DUTY_MAX; duty >= DUTY_MIN; duty -= 0.01) {
            pwm_set_gpio_level(PWMPIN, duty * WRAP); // set the duty cycle
            sleep_ms(100);
        }
    }
}