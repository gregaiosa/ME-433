#include <stdio.h>
#include "pico/stdlib.h"

#define SCK_PIN 19
#define DT_PIN 18
#define MAX_SAMPLES 1000

void hx711_init(void) {
    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_init(DT_PIN);
    gpio_set_dir(DT_PIN, GPIO_IN);
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

int32_t raw_data[MAX_SAMPLES];
int32_t filtered_data[MAX_SAMPLES];
uint32_t time_ms[MAX_SAMPLES];

int main() {
    stdio_init_all();
    hx711_init();

    while (true) {
        // Wait for computer to send a number of samples to collect
        int num_samples = 0;
        int res = scanf("%d", &num_samples);
        
        if (res == 1 && num_samples > 0 && num_samples <= MAX_SAMPLES) {
            float filter_val = 0.0f;
            
            // Collect the data
            for (int i = 0; i < num_samples; i++) {
                int32_t raw = hx711_read();
                uint32_t t = to_ms_since_boot(get_absolute_time());
                
                // IIR filter y[n] = 0.9 * y[n-1] + 0.1 * x[n]
                if (i == 0) {
                    filter_val = (float)raw;
                } else {
                    filter_val = 0.9f * filter_val + 0.1f * raw;
                }
                
                raw_data[i] = raw;
                filtered_data[i] = (int32_t)filter_val;
                time_ms[i] = t;
            }
            
            // Print data back over serial (time_ms, raw, filtered)
            for (int i = 0; i < num_samples; i++) {
                printf("%u %d %d\r\n", time_ms[i], raw_data[i], filtered_data[i]);
            }
        }
    }
}
