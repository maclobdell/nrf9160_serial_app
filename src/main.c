/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <string.h>


/* change this to any other UART peripheral if desired */
#define UART0_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define UART1_DEVICE_NODE DT_CHOSEN(zephyr_console1)

//alternative?
//#define UART0_DEVICE_NODE DT_CHOSEN(uart0)
//#define UART1_DEVICE_NODE DT_CHOSEN(uart1)

//another alternative?
//#define UART0_DEVICE_NODE DT_NODELABEL(uart0);
//#define UART1_DEVICE_NODE DT_NODELABEL(uart1);

#define MSG_SIZE 32

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart0_dev = DEVICE_DT_GET(UART0_DEVICE_NODE);
static const struct device *const uart1_dev = DEVICE_DT_GET(UART1_DEVICE_NODE);

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

//if (!device_is_ready(uart0_dev)) {
//    return;
//}

//if (!device_is_ready(uart1_dev)) {
//    return;
//}

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart0_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart0_dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart0_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart0_dev, buf[i]);
	}
}

int main(void)
{
	char tx_buf[MSG_SIZE];

	if (!device_is_ready(uart0_dev)) {
		printk("UART device not found!");
		return 0;
	}

	/* configure interrupt and callback to receive data */
	int ret = uart_irq_callback_user_data_set(uart0_dev, serial_cb, NULL);

	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart0_dev);

	print_uart("Hello! I'm your echo bot.\r\n");
	print_uart("Tell me something and press enter:\r\n");

	/* indefinitely wait for input from the user */
	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
		print_uart("Echo: ");
		print_uart(tx_buf);
		print_uart("\r\n");
	}
	return 0;
}
