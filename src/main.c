/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

#include <string.h>

#define UART0_DEVICE_NODE DT_CHOSEN(stupid_uart)
#define UART1_DEVICE_NODE DT_CHOSEN(dumb_uart)

#define MSG_SIZE 32

#define SLEEP_TIME_MS   1000
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart0_msgq, MSG_SIZE, 10, 4);
K_MSGQ_DEFINE(uart1_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart0_dev = DEVICE_DT_GET(UART0_DEVICE_NODE);
static const struct device *const uart1_dev = DEVICE_DT_GET(UART1_DEVICE_NODE);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

/* receive buffer used in UART ISR callback */
static char rx_buf0[MSG_SIZE];
static int rx_buf0_pos;

/* receive buffer used in UART ISR callback */
static char rx_buf1[MSG_SIZE];
static int rx_buf1_pos;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void uart0_serial_cb(const struct device *dev, void *user_data)
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
		if ((c == '\n' || c == '\r') && rx_buf0_pos > 0) {
			/* terminate string */
			rx_buf0[rx_buf0_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart0_msgq, &rx_buf0, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf0_pos = 0;
		} else if (rx_buf0_pos < (sizeof(rx_buf0) - 1)) {
			rx_buf0[rx_buf0_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}
/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void uart1_serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart1_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart1_dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart1_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf1_pos > 0) {
			/* terminate string */
			rx_buf1[rx_buf1_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart1_msgq, &rx_buf1, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf1_pos = 0;
		} else if (rx_buf1_pos < (sizeof(rx_buf1) - 1)) {
			rx_buf1[rx_buf1_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart0(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart0_dev, buf[i]);
	}
}
/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart1(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart1_dev, buf[i]);
	}
}


int main(void)
{
	int ret;
	
	char tx_buf0[MSG_SIZE];
	char tx_buf1[MSG_SIZE];

	if (!device_is_ready(uart0_dev)) {
		printk("UART0 device not found!");
		return 0;
	}

	if (!device_is_ready(uart1_dev)) {
		printk("UART1 device not found!");
		return 0;
	}

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	/* configure interrupt and callback to receive data */
	ret = uart_irq_callback_user_data_set(uart0_dev, uart0_serial_cb, NULL);

	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("UART0: Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART0: UART device does not support interrupt-driven API\n");
		} else {
			printk("UART0: Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart0_dev);

	/* configure interrupt and callback to receive data */
	ret = uart_irq_callback_user_data_set(uart1_dev, uart1_serial_cb, NULL);

	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("UART1: Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART1: UART device does not support interrupt-driven API\n");
		} else {
			printk("UART1: Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart1_dev);


	print_uart0("UART0: Hello! I'm your echo bot.\r\n");
	print_uart0("UART0: Tell me something and press enter:\r\n");

	print_uart1("UART1: Hello! I'm your echo bot.\r\n");
	print_uart1("UART1: Tell me something and press enter:\r\n");

    //todo - create a separate thread for each uart

	/* indefinitely wait for input from the user */
	//while (k_msgq_get(&uart1_msgq, &tx_buf1, K_FOREVER) == 0) {
	while (1) {
		gpio_pin_toggle_dt(&led);
		print_uart1("ABC");
		//print_uart1(tx_buf1);
		//print_uart1("\r\n");
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
