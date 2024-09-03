# nrf9160_serial_app
Enable communication on two UARTs

## Details
Started with sample app: v2.7.0/zephyr/samples/drivers/uart/echo_bot

Device tree node names (uart0 & uart1) are found in 
   /opt/nordic/ncs/v2.7.0/zephyr/boards/nordic/nrf9160dk/nrf9160dk_nrf9160_common.dtsi

## Setup Environment 
  Generate a env.sh for your system with the Nordic nRF Connect for Desktop Toolchain manager

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

Note: there is also a way to dynamically set uart configuration in the code instead of using the device tree overlay

https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-4-serial-communication-uart/topic/uart-driver/
