#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5



int main()
{
    stdio_init_all();

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    while (true) {
        // 1. Read from the computer (via USB stdio) and send to STM32 (via UART1)
        int c = getchar_timeout_us(0); // non-blocking read
        if (c != PICO_ERROR_TIMEOUT) {
            uart_putc(UART_ID, c);
        }

        // 2. Read from STM32 (via UART1) and send to the computer (via USB stdio)
        if (uart_is_readable(UART_ID)) {
            uint8_t ch = uart_getc(UART_ID);
            putchar(ch);
            fflush(stdout); // Ensure it prints immediately
        }
    }
}
