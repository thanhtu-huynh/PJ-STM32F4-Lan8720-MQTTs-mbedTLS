# STM32F4 LAN8720 MQTTs with mbedTLS

This repository contains an STM32CubeIDE project demonstrating how to connect an STM32F4 microcontroller to an MQTT broker securely over TLS (MQTTs) using a LAN8720 Ethernet PHY, LwIP, and mbedTLS.

## Hardware Requirements
- **MCU**: STM32F4 series (Configured for STM32F407VGTX)
- **Ethernet PHY**: LAN8720
- **Debug Interface**: USART2 (115200 8-N-1) for `printf` output

## Software Components
- **RTOS**: FreeRTOS
- **Network Stack**: LwIP (Lightweight IP)
- **Security**: mbedTLS
- **Development Environment**: STM32CubeIDE

## Project Structure
The project is located under the `CB_HuynhThanhTu` directory.
- `Core/`: Application logic, `main.c`, hardware and system initialization.
- `LWIP/`: LwIP stack integration and configuration.
- `MBEDTLS/`: mbedTLS integration for secure communication.
- `Middlewares/`: Third-party libraries like FreeRTOS.
- `Drivers/`: STM32 HAL (Hardware Abstraction Layer) drivers.

## Getting Started

### Building the Project
1. Open **STM32CubeIDE**.
2. Select **File > Import > General > Existing Projects into Workspace**.
3. Browse to the `CB_HuynhThanhTu` directory and select it.
4. Click **Finish**.
5. Build the project by navigating to **Project > Build Project** or pressing the hammer icon.

### Configuration
To connect to your specific MQTT broker, you may need to update the configuration. Check the `Core/Src/` directory for configuration headers (such as `config.h` or `mbedtls_config.h`) where you can set:
- MQTT Broker IP / Hostname
- MQTT Port (Typically 8883 for MQTTs)
- Wi-Fi/Ethernet settings
- Root CA certificate, client certificate, and private key for mbedTLS.

### Debugging
The application redirects `printf` output to `USART2`. Connect a USB-to-TTL serial adapter to the configured USART2 TX/RX pins to view runtime logs, connection status, and debugging information.
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

## License
This project is provided as-is. Please check individual source files for specific license information (e.g., STMicroelectronics licenses for HAL drivers, FreeRTOS license, etc.).
