#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pico/cyw43_arch.h"


// #define BAUD_RATE 115200
// #define BAUD_RATE 115200 * 8
#define BAUD_RATE 115200 * 8 * 8
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// module
// #define UART_ID uart1
// picow
#define UART_ID uart0

// module
// #define UART_TX_PIN 24
// #define UART_RX_PIN 25
// picow
#define UART_TX_PIN 16
#define UART_RX_PIN 17

// module
// #define RS485_RX_EN 4
// #define RS485_TX_EN 3
// picow
#define RS485_RX_EN 18
#define RS485_TX_EN 19

// pico
// #define LED_PIN PICO_DEFAULT_LED_PIN
// picow
#define LED_PIN CYW43_WL_GPIO_LED_PIN



void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

static inline uint8_t uart_has_error(uart_inst_t *uart) {
    uint8_t status = (uart_get_hw(uart)->rsr & (UART_UARTRSR_BE_BITS | UART_UARTRSR_PE_BITS | UART_UARTRSR_FE_BITS));
    // uart_get_hw(uart)->rsr = UART_UARTRSR_BE_BITS | UART_UARTRSR_PE_BITS | UART_UARTRSR_FE_BITS;
    return status;
}

int main() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // low = enabled
    gpio_init(RS485_RX_EN);
    gpio_set_dir(RS485_RX_EN, GPIO_OUT);
    gpio_put(RS485_RX_EN, 0);

    // low = disabled
    gpio_init(RS485_TX_EN);
    gpio_set_dir(RS485_TX_EN, GPIO_OUT);
    gpio_put(RS485_TX_EN, 0);

    // const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    // gpio_init(LED_PIN);
    // gpio_set_dir(LED_PIN, GPIO_OUT);

    // picow, only for LED
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

    stdio_init_all(); 

    int led_state = 0;
    // char tx1[] = "(11,99)\0";
    char tx1[] = "(11,AAAA,BBBB,CCCC,DDDD,EEEE,FFFF,GGGG,HHHH)\0";
    char *tx;
    char rx[64];
    int i;
    bool msg = false;
    i = 0;
    uint8_t rx_err;

    while (1) {
        
        while (uart_is_readable(UART_ID)) {
            rx[i] = uart_getc(UART_ID);
            rx_err = uart_has_error(UART_ID);
            // printf("s RX[%d] err %x : ", i, rx_err);
            // DumpHex(rx, i+1);
            if (rx_err) {
                i = 0;
            } else {
                if (rx[i] == ')') {
                    i++;
                    rx[i] = '\0';
                    
                    // printf("s RX[%d]: ", i);
                    // DumpHex(rx, i);
                    // printf("s RX[%d]: %s\n", i, rx);

                    i = 0;
                    if (rx[1] == '1' && rx[2] == '1') {
                        msg = true;
                    }
                    break;
                } else {
                    i++;
                }
            }
            if (i > 63) {
                printf("s err RX[%d]: ", i);
                DumpHex(rx, i);
                i = 0;
            }
        }

        // sleep_ms(20);

        if (msg) {
            // gpio_put(LED_PIN, led_state);
            // cyw43_arch_gpio_put(LED_PIN, led_state);
            led_state = 1 - led_state;

            tx = tx1;
            // printf("s TX: %s\n", tx);

            // disable rx, enable tx
            gpio_put(RS485_RX_EN, 1);
            gpio_put(RS485_TX_EN, 1);

            while (*tx) {
                uart_putc_raw(UART_ID, *tx++);
            }
            uart_tx_wait_blocking(UART_ID);

            // disable tx, enable rx (NOTE: must first disable TX otherwise junk is seen on RX)
            gpio_put(RS485_TX_EN, 0);
            gpio_put(RS485_RX_EN, 0);

            msg = false;
            i = 0;
        }
    }
}
