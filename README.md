# nrf9160_serial_app
Enable communication on two UARTs

## Details
Started with sample app: v2.7.0/zephyr/samples/drivers/uart/echo_bot

Trying to add second UART. 

Tried to reference device tree node names (uart0 & uart1) are found in 
   /opt/nordic/ncs/v2.7.0/zephyr/boards/nordic/nrf9160dk/nrf9160dk_nrf9160_common.dtsi

Trying to use overlay file to modify. Not supposed to edit board devicetree files directly.
Overlay file copied from ncs/v2.7.0/nrf/samples/cellular/modem_shell/bt.overlay

## Setup Environment 
  Anyone else will likely need to regenerate env.sh for your system from the Nordic nRF Connect for Desktop Toolchain manager

```
source env.sh
```

## Build With

```
west build ./ --board nrf9160dk_nrf9160_ns -d ./build -DCONF_FILE="prj.conf"  -DEXTRA_DTC_OVERLAY_FILE="uart_dt.overlay" 
```

## Pin connections
### ON Fanstel EVB

```
 "uart1"
  (nRF91) P0.01/APP2_TXD
  (nRF91) P0.00/APP2_RXD
     GOES TO CONNECTOR J2

 "uart0"
  (nRF91) P0.29/APP1_TXD 
  (nRF91) P0.28/APP1_RXD 
    CONNECTS TO 
  (nRF53) LTE-TXD  P0.03   nRF52_TXD
  (nRF53) LTE-RXD  P0.05   nRF52_RXD
```

### NEW BOARD

```
  (nRF91)  LTE-APP1-TXD (pin 74)  P0.29
  (nRF91) LTE-APP1-RXD (pin 72)  P0.28
    OR
  (nRF53)BLE-UART-TXD / BLE-TXD (pin 35) P1.02
  (nRF53)BLE-UART-RXD / BLE-RXD (pin 26) P1.01
	GOES TO MCU	UART

  (nRF91)  LTE-APP2-TXD (pin 69) P0.01
  (nRF91)  LTE-APP2-RXD (pin 67) P0.00
     CONNECTS TO 
  (nRF53) nRF53-TXD (pin 63) P0.03
  (nRF53) nRF54-RXD (pin 47) P0.05
```

# Config
1. Add this to kconfig file
 CONFIG_SERIAL=y
 CONFIG_UART_ASYNC_API=y	

2. Put this in a devicetree overlay file
     Apparently the number can't conflict, even between other driver types like spi and uart
     example, you can't have spi2 and uart2. weird. 

```
&uart0 {
	status = "okay";
	current-speed = <19200>;
	tx-pin = <29>;
	rx-pin = <28>;
	rts-pin = <0xFFFFFFFF>;
	cts-pin = <0xFFFFFFFF>;
};

&uart1 {
	status = "okay";
	current-speed = <19200>;
	tx-pin = <1>;
	rx-pin = <0>;
	rts-pin = <0xFFFFFFFF>;
	cts-pin = <0xFFFFFFFF>;
};
```

Note: there is also a way to dynamically set uart configuration in the code instead of using the device tree overlay
  https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-4-serial-communication-uart/topic/uart-driver/



## USEFUL INFO
https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-2-reading-buttons-and-controlling-leds/topic/devicetree/

