# Modbus TCP Server on STM32F4 with W5500 Ethernet Module

A complete implementation of a Modbus TCP server running on STM32F4 microcontroller using the W5500 Ethernet module. This project enables remote control and monitoring of GPIO pins and registers through the Modbus TCP protocol.

## 📋 Table of Contents

- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Features](#features)
- [Pin Configuration](#pin-configuration)
- [Network Configuration](#network-configuration)
- [Supported Modbus Functions](#supported-modbus-functions)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
- [Build and Flash](#build-and-flash)

## 🎯 Overview

This project implements a Modbus TCP server that allows network-based control of an STM32F4 microcontroller. The W5500 Ethernet module handles the TCP/IP stack, while the STM32 processes Modbus protocol requests and controls physical I/O.

**Key Capabilities:**
- Read/write digital outputs (coils)
- Read digital inputs
- Read holding registers
- Read input registers
- Full Modbus exception handling
- Automatic connection management
- UART debug logging

## 🔧 Hardware Requirements

- **Microcontroller**: STM32F4 series (tested on STM32F401/STM32F411)
- **Ethernet Module**: W5500 Ethernet module
- **Connections**:
  - SPI1 for W5500 communication
  - UART2 for debug output (115200 baud)
  - GPIO pins for digital I/O

## ✨ Features

### Modbus Protocol Support
- ✅ Function 0x01: Read Coils
- ✅ Function 0x02: Read Discrete Inputs
- ✅ Function 0x03: Read Holding Registers
- ✅ Function 0x04: Read Input Registers
- ✅ Function 0x05: Write Single Coil
- ✅ Function 0x0F: Write Multiple Coils
- ✅ Function 0x10: Write Multiple Registers

### Network Features
- TCP socket management with automatic reconnection
- Configurable timeout and retransmission
- Support for multiple sequential requests
- Connection state monitoring

### Debug Features
- Comprehensive UART logging
- Socket state tracking
- Modbus PDU inspection
- Network configuration verification

## 📌 Pin Configuration

### W5500 SPI Interface
```
STM32F4          W5500
--------------------------
PA5       -->    CS (Chip Select)
PA6       <--    MISO
PA7       -->    MOSI
PB3       -->    SCK
PB5       -->    RST (Reset)
```

### Digital I/O Mapping
```c
// Output Coils (Modbus addresses 0-1)
PC10  -->  Coil 0
PC12  -->  Coil 1

// Input Contacts (Modbus addresses 0-1)
PC2   <--  Input 0
PC3   <--  Input 1
```

### Debug UART
```
PA2   -->  UART2 TX (Debug output)
PA3   <--  UART2 RX
```

## 🌐 Network Configuration

Default network settings (configurable in `TCP.h`):

```c
IP Address:    192.168.137.100
Subnet Mask:   255.255.255.0
Gateway:       192.168.137.1
MAC Address:   00:08:DC:12:34:56
Listen Port:   5000
```

To modify, edit the `W5500_Configure_Network()` function in `TCP.h`.

## 📡 Supported Modbus Functions

### 0x01 - Read Coils
Read the status of output coils (digital outputs).

**Request:**
- Starting address: 2 bytes
- Quantity of coils: 2 bytes (1-2000)

**Response:**
- Byte count: 1 byte
- Coil status: N bytes (packed bits)

### 0x02 - Read Discrete Inputs
Read the status of digital inputs.

**Request:**
- Starting address: 2 bytes
- Quantity of inputs: 2 bytes (1-2000)

**Response:**
- Byte count: 1 byte
- Input status: N bytes (packed bits)

### 0x03 - Read Holding Registers
Read holding register values.

**Request:**
- Starting address: 2 bytes
- Quantity of registers: 2 bytes (1-125)

**Response:**
- Byte count: 1 byte (quantity × 2)
- Register values: N × 2 bytes

### 0x04 - Read Input Registers
Read input register values.

**Request:**
- Starting address: 2 bytes
- Quantity of registers: 2 bytes (1-125)

**Response:**
- Byte count: 1 byte (quantity × 2)
- Register values: N × 2 bytes

### 0x05 - Write Single Coil
Write a single output coil.

**Request:**
- Coil address: 2 bytes
- Coil value: 2 bytes (0xFF00 = ON, 0x0000 = OFF)

**Response:**
- Echo of request

### 0x0F - Write Multiple Coils
Write multiple output coils.

**Request:**
- Starting address: 2 bytes
- Quantity of coils: 2 bytes (1-1968)
- Byte count: 1 byte
- Coil values: N bytes (packed bits)

**Response:**
- Starting address: 2 bytes
- Quantity written: 2 bytes

### 0x10 - Write Multiple Registers
Write multiple holding registers.

**Request:**
- Starting address: 2 bytes
- Quantity of registers: 2 bytes (1-123)
- Byte count: 1 byte
- Register values: N × 2 bytes

**Response:**
- Starting address: 2 bytes
- Quantity written: 2 bytes

### Exception Responses
All functions implement proper exception handling:
- `0x01`: Illegal Function
- `0x02`: Illegal Data Address
- `0x03`: Illegal Data Value
- `0x04`: Server Device Failure

## 📁 Project Structure

```
├── main.c                    # Main application loop and Modbus processing
├── main.h                    # Main header file
├── TCP.h                     # W5500 driver and TCP socket management
├── stm32f4xx_hal_conf.h     # HAL configuration
├── stm32f4xx_hal_msp.c      # MSP initialization
├── stm32f4xx_it.c           # Interrupt handlers
├── stm32f4xx_it.h           # Interrupt handler headers
├── syscalls.c               # System calls (malloc, etc.)
├── sysmem.c                 # Memory management
└── system_stm32f4xx.c       # System initialization
```

### Key Files

**main.c**
- Main event loop with socket state machine
- Modbus PDU processing functions
- GPIO control logic
- Data mapping (coils, inputs, registers)

**TCP.h**
- W5500 register definitions and SPI communication
- Socket initialization and management
- TCP connection handling
- Network configuration
- Send/receive operations

## 🔄 How It Works

### 1. Initialization Sequence
```c
HAL_Init()                    // Initialize HAL library
SystemClock_Config()          // Configure system clock
MX_GPIO_Init()                // Initialize GPIO pins
MX_DMA_Init()                 // Initialize DMA
MX_USART2_UART_Init()        // Initialize debug UART
MX_SPI1_Init()               // Initialize SPI for W5500

W5500_Configure_Network()     // Set IP, MAC, gateway, subnet
Socket_START()                // Open socket in TCP mode
Socket_LISTEN()               // Start listening on port 5000
```

### 2. Main Loop State Machine
```
SOCK_INIT
   ↓
SOCK_LISTEN ←────────────┐
   ↓                      │
SOCK_ESTABLISHED          │
   ↓                      │
   • Receive data         │
   • Process Modbus       │
   • Send response        │
   • Check for FIN        │
   ↓                      │
SOCK_CLOSE_WAIT           │
   ↓                      │
SOCK_CLOSED ──────────────┘
```

### 3. Modbus Processing Flow
```
Receive TCP packet
   ↓
Extract Modbus TCP header (7 bytes)
   ↓
Extract PDU (function code + data)
   ↓
Process function:
   • Validate address range
   • Validate data values
   • Execute operation
   • Build response or exception
   ↓
Wrap response in TCP header
   ↓
Send TCP response
```

### 4. W5500 Communication
All W5500 operations use SPI protocol with 3-byte addressing:
```
[Address MSB][Address LSB][Control Byte][Data...]
```

Control byte format:
- Bits 7-3: Block Select Bits (BSB)
- Bit 2: Read/Write bit (0=read, 1=write)
- Bits 1-0: Operating mode (00=VDM)

## 🔨 Build and Flash

### Using STM32CubeIDE
1. Import project into STM32CubeIDE
2. Build the project (Ctrl+B)
3. Connect ST-Link debugger
4. Flash to target (F11)

### Using Command Line (arm-none-eabi-gcc)
```bash
# Build
make

# Flash using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
        -c "program build/project.elf verify reset exit"
```

## 💡 Usage Examples

### Python Example - Read Coils
```python
from pymodbus.client import ModbusTcpClient

client = ModbusTcpClient('192.168.137.100', port=5000)
client.connect()

# Read coils 0-1
result = client.read_coils(0, 2)
print(f"Coil 0: {result.bits[0]}")
print(f"Coil 1: {result.bits[1]}")

client.close()
```

### Python Example - Write Single Coil
```python
from pymodbus.client import ModbusTcpClient

client = ModbusTcpClient('192.168.137.100', port=5000)
client.connect()

# Turn on coil 0 (PC10 high)
client.write_coil(0, True)

# Turn off coil 1 (PC12 low)
client.write_coil(1, False)

client.close()
```

### Python Example - Read Input Registers
```python
from pymodbus.client import ModbusTcpClient

client = ModbusTcpClient('192.168.137.100', port=5000)
client.connect()

# Read input registers 0-1
result = client.read_input_registers(0, 2)
print(f"Register 0: {result.registers[0]}")  # Default: 12345
print(f"Register 1: {result.registers[1]}")  # Default: 54321

client.close()
```

### Command Line - Read Holding Registers
```bash
# Using modpoll tool
modpoll -m tcp -a 0 -r 1 -c 2 -t 4 192.168.137.100 -p 5000
```




---

**Project Status**: Working prototype - tested with multiple Modbus TCP clients including Python pymodbus and modpoll command-line tool.
