/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "TCP.h"

#define MAX_COILS        2
#define MAX_INPUTS       2
#define MAX_REGISTERS    2

// Mapowania pinów
uint16_t numer_pinow_wejsc[MAX_INPUTS] = {GPIO_PIN_2, GPIO_PIN_3};
uint16_t numer_pinow_wyjsc[MAX_COILS] = {GPIO_PIN_10, GPIO_PIN_12};

// Bufory danych
uint16_t mapa_cewek[MAX_COILS] = {0};
uint16_t mapa_wejsc[MAX_INPUTS] = {0};
uint16_t rejestry_wejsciowe[MAX_REGISTERS] = {12345, 54321};


#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION    0x01
#define MODBUS_EXCEPTION_ILLEGAL_ADDRESS     0x02
#define MODBUS_EXCEPTION_ILLEGAL_VALUE       0x03
#define MODBUS_EXCEPTION_DEVICE_FAILURE      0x04

// Czytanie stanu fizycznych wejść
void read_inputs() {
    for (uint8_t i = 0; i < MAX_INPUTS; i++) {
        mapa_wejsc[i] = HAL_GPIO_ReadPin(GPIOC, numer_pinow_wejsc[i]);
    }
}

// np. pomocnicza funkcja do wypisania numeru pinu:
int get_pin_number(uint16_t pin_mask) {
    for (int i = 0; i < 16; i++) {
        if (pin_mask == (1 << i)) return i;
    }
    return -1; // niepoprawny pin
}


// Ustawianie stanu wyjść
void set_outputs() {
    char msg[64];

    for (uint8_t i = 0; i < MAX_COILS; i++) {
        int pin_num = get_pin_number(numer_pinow_wyjsc[i]);
        if (pin_num == -1) {
            snprintf(msg, sizeof(msg), "BŁĄD: Nieprawidłowy pin wyjścia [%u]\r\n", i);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            continue;
        }

        if (mapa_cewek[i] > 0) {
            HAL_GPIO_WritePin(GPIOC, numer_pinow_wyjsc[i], GPIO_PIN_SET);
            snprintf(msg, sizeof(msg), "PIN PC%d -> HIGH (SET)\r\n", pin_num);
        } else {
            HAL_GPIO_WritePin(GPIOC, numer_pinow_wyjsc[i], GPIO_PIN_RESET);
            snprintf(msg, sizeof(msg), "PIN PC%d -> LOW (RESET)\r\n", pin_num);
        }
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
}

int modbus_exception_response(uint8_t function_code, uint8_t exception_code,
                              uint8_t *response, uint16_t *response_len,
                              const char *msg)
{
    char buf[128];

    // Function code with error bit set (bit 7 = 1)
    response[0] = function_code | 0x80;
    response[1] = exception_code;
    *response_len = 2;

    // Opcjonalne logowanie na UART dla debugowania
    sprintf(buf, "Modbus Exception: Func=0x%02X -> 0x%02X, Code=0x%02X (%s)\r\n",
            function_code, response[0], exception_code, msg);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);

    return 1;
}



int process_modbus_pdu(uint8_t *request, uint16_t request_len,
                       uint8_t *response, uint16_t *response_len)
{
    char debug_msg[100];
    if (request_len < 1)
        return -1;

    uint8_t function = request[0];
    uint16_t pos = 1;
    response[0] = function;

    sprintf(debug_msg, "PDU: Func=0x%02X Len=%d\r\n", function, request_len);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    switch (function) {

        case 0x01: { // Read Coils
            if (request_len < 5)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t qty  = (request[3] << 8) | request[4];

            sprintf(debug_msg, "Read Coils: Addr=%u Qty=%u\r\n", addr, qty);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

            if (qty == 0 || qty > 2000 || (addr + qty) > MAX_COILS)
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or quantity");

            uint8_t byte_count = (qty + 7) / 8;
            response[pos++] = byte_count;

            memset(&response[pos], 0, byte_count);

            for (uint16_t i = 0; i < qty; i++) {
                uint8_t bit = mapa_cewek[addr + i] & 1;
                response[pos + (i / 8)] |= (bit << (i % 8));
            }
            pos += byte_count;
            break;
        }

        case 0x02: { // Read Discrete Inputs
            read_inputs();
            if (request_len < 5)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t qty  = (request[3] << 8) | request[4];

            sprintf(debug_msg, "Read Inputs: Addr=%u Qty=%u\r\n", addr, qty);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

            if (qty == 0 || qty > 2000 || (addr + qty) > MAX_INPUTS)
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or quantity");

            uint8_t byte_count = (qty + 7) / 8;
            response[pos++] = byte_count;

            memset(&response[pos], 0, byte_count);

            for (uint16_t i = 0; i < qty; i++) {
                uint8_t bit = mapa_wejsc[addr + i] & 1;
                response[pos + (i / 8)] |= (bit << (i % 8));
            }
            pos += byte_count;
            break;
        }

        case 0x03: { // Read Holding Registers
            if (request_len < 5)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t qty  = (request[3] << 8) | request[4];

            sprintf(debug_msg, "Read Holding Registers: Addr=%u Qty=%u\r\n", addr, qty);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

            if (qty == 0 || qty > 125 || (addr + qty) > MAX_REGISTERS)
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or quantity");

            response[pos++] = qty * 2;

            for (uint16_t i = 0; i < qty; i++) {
                uint16_t val = rejestry_wejsciowe[addr + i];
                response[pos++] = (val >> 8) & 0xFF;
                response[pos++] = val & 0xFF;
            }
            break;
        }

        case 0x04: { // Read Input Registers
            if (request_len < 5)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t qty  = (request[3] << 8) | request[4];

            sprintf(debug_msg, "Read Input Registers: Addr=%u Qty=%u\r\n", addr, qty);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

            if (qty == 0 || qty > 125 || (addr + qty) > MAX_REGISTERS)
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or quantity");

            response[pos++] = qty * 2;

            for (uint16_t i = 0; i < qty; i++) {
                uint16_t val = rejestry_wejsciowe[addr + i];
                response[pos++] = (val >> 8) & 0xFF;
                response[pos++] = val & 0xFF;
            }
            break;
        }

        case 0x05: { // Write Single Coil
            if (request_len < 5)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t val  = (request[3] << 8) | request[4];

            sprintf(debug_msg, "Write Single Coil: Addr=%u Val=0x%04X\r\n", addr, val);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

            if (addr >= MAX_COILS || (val != 0xFF00 && val != 0x0000))
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or value");

            mapa_cewek[addr] = (val == 0xFF00) ? 1 : 0;
            set_outputs();

            for (int i = 1; i <= 4; ++i) response[pos++] = request[i];
            break;
        }

        case 0x0F: { // Write Multiple Coils
            if (request_len < 6)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t qty = (request[3] << 8) | request[4];
            uint8_t byte_count = request[5];

            sprintf(debug_msg, "Write Multiple Coils: Addr=%u Qty=%u ByteCount=%u\r\n", addr, qty, byte_count);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

            if (qty == 0 || qty > 1968 || byte_count != (qty + 7) / 8 || (addr + qty) > MAX_COILS)
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or quantity");
            if (request_len < 6 + byte_count)
                return modbus_exception_response(function, 0x03, response, response_len, "Incomplete request");

            const uint8_t *data = &request[6];
            for (uint16_t i = 0; i < qty; i++) {
                uint8_t bit = (data[i / 8] >> (i % 8)) & 1;
                mapa_cewek[addr + i] = bit;
            }

            set_outputs();
            for (int i = 1; i <= 4; ++i) response[pos++] = request[i];
            break;
        }

        case 0x10: { // Write Multiple Registers
            if (request_len < 6)
                return modbus_exception_response(function, 0x03, response, response_len, "Malformed PDU");

            uint16_t addr = (request[1] << 8) | request[2];
            uint16_t qty  = (request[3] << 8) | request[4];
            uint8_t byte_count = request[5];

            if (qty == 0 || qty > 123 || byte_count != qty * 2 || (addr + qty) > MAX_REGISTERS)
                return modbus_exception_response(function, 0x02, response, response_len, "Invalid address or quantity");
            if (request_len < 6 + byte_count)
                return modbus_exception_response(function, 0x03, response, response_len, "Incomplete request");

            const uint8_t *data = &request[6];
            for (uint16_t i = 0; i < qty; i++) {
                rejestry_wejsciowe[addr + i] = (data[2 * i] << 8) | data[2 * i + 1];
            }

            for (int i = 1; i <= 4; ++i) response[pos++] = request[i];
            break;
        }

        default:
            return modbus_exception_response(function, 0x01, response, response_len, "Illegal function");
    }

    *response_len = pos;

    sprintf(debug_msg, "PDU Response (%d bytes): ", pos);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    for (uint16_t i = 0; i < pos; i++) {
        sprintf(debug_msg, "[%02d]:%02X ", i, response[i]);
        HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);

    return 1;
}


int process_modbus_tcp(uint8_t *tcp_req, uint16_t tcp_len,
                       uint8_t *tcp_resp, uint16_t *tcp_resp_len)
{
    char debug_msg[128];

    if (tcp_len < 8) {
        sprintf(debug_msg, "Modbus TCP: pakiet za krótki, len=%u\r\n", tcp_len);
        HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
        return -1;
    }

    uint16_t len_field = (tcp_req[4] << 8) | tcp_req[5];
    if (tcp_len != len_field + 6) {
        sprintf(debug_msg, "Modbus TCP: niespójna długość: tcp_len=%u, len_field=%u\r\n", tcp_len, len_field);
        HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
        return -1;
    }

    uint8_t unit_id = tcp_req[6];
    uint8_t *pdu_req = &tcp_req[7];
    uint16_t pdu_req_len = tcp_len - 7;

    sprintf(debug_msg, "Modbus TCP: Otrzymano %u bajtów, Unit ID=0x%02X\r\n", tcp_len, unit_id);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    sprintf(debug_msg, "Modbus TCP: PDU żądanie (len=%u): ", pdu_req_len);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    uint8_t pdu_resp[256];
    uint16_t pdu_resp_len = 0;

    int result = process_modbus_pdu(pdu_req, pdu_req_len, pdu_resp, &pdu_resp_len);
    if (result <= 0) {
        sprintf(debug_msg, "Modbus TCP: Błąd w przetwarzaniu PDU, kod=%d\r\n", result);
        HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
        return result;
    }


    memcpy(tcp_resp, tcp_req, 4); // transaction ID + protocol ID


    uint16_t resp_len_field = pdu_resp_len + 1;  // PDU + Unit ID

    tcp_resp[4] = (resp_len_field >> 8) & 0xFF;
    tcp_resp[5] = resp_len_field & 0xFF;
    tcp_resp[6] = unit_id;


    memcpy(&tcp_resp[7], pdu_resp, pdu_resp_len);
    *tcp_resp_len = 7 + pdu_resp_len ;


    return 1;
}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  W5500_Configure_Network();
  W5500_Chec_Network_Configuration();
  W5500_TestVersion();
  HAL_Delay(1000);
  Socket_close();
  Socket_START();
  Socket_LISTEN();
  char message[64];

  sprintf(message, "Przed wartoscia petli");
  HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
  uint8_t prev_Sn_SR = 0xFF;
  //Socket_ESTABLISHED();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t Sn_SR = 0;
		  W5500_Read(W5500_S0_SR, BSB_SOCKET0, &Sn_SR, 1);

		  static uint8_t prev_Sn_SR = 0xFF;
		  char debug_msg[64];

		  if (Sn_SR != prev_Sn_SR) {
			  sprintf(debug_msg, "Stan zmienił się: 0x%02X\r\n", Sn_SR);
			  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
			  prev_Sn_SR = Sn_SR;
		  }

		  switch (Sn_SR) {
			  case SOCK_INIT:
				  Socket_LISTEN();
				  sprintf(debug_msg, "Stan: SOCK_INIT -> przejście do LISTEN\r\n");
				  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
				  break;

			  case SOCK_LISTEN:
				  // Można tu dodać co jakiś czas debug
				  break;

			  case SOCK_ESTABLISHED: {
				  sprintf(debug_msg, "Stan: SOCK_ESTABLISHED - połączenie nawiązane\r\n");
				  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

				  // ✅ Odbieranie danych
				  uint16_t rx_len = Socket_check_received_data();
				  if (rx_len > 0) {
					  sprintf(debug_msg, "Odebrano dane: %d bajtów\r\n", rx_len);
					  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

					  uint8_t* data = Socket_receiving_process(rx_len);

					  for (uint16_t i = 0; i < rx_len; i++) {
					      char debug_msg[64];
					      sprintf(debug_msg, "Byte[%02d] = 0x%02X\r\n", i, data[i]);
					      HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
					  }

					  //swiecenie dioda

					  //koniec siwecenia dioda

					  if (data != NULL) {
						  sprintf(debug_msg, "Modbus: odebrano %d bajtów\r\n", rx_len);
						  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

						  // 🔧 PRZETWARZANIE MODBUS PDU
						  uint8_t response[256];
						  uint16_t response_len = 0;
						  process_modbus_tcp(data, rx_len, response, &response_len);

						  if (response_len > 0) {
							  // ✉️ Odeślij odpowiedź przez W5500
							  // 📡 Debug: wypisz zawartość odpowiedzi przed wysłaniem
							  char debug_msg[512];
							  int offset = 0;

							  offset += sprintf(debug_msg + offset, "TX Response (len=%u): ", response_len);
							  for (uint16_t i = 0; i < response_len; i++) {
								  offset += sprintf(debug_msg + offset, "%02X", response[i]);
							  }
							  offset += sprintf(debug_msg + offset, "\r\n");

							  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, offset, HAL_MAX_DELAY);
							  Socket_send_proces(response, response_len);
							  HAL_UART_Transmit(&huart2, response, response_len, HAL_MAX_DELAY);


						  } else {
							  const char* err = "Błąd przetwarzania PDU\r\n";
							  HAL_UART_Transmit(&huart2, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
						  }

						  free(data);
					  }
				  }

				  // 🔚 Sprawdzenie żądania rozłączenia (FIN)
				  if (Socket_check_disconnect_request()) {
					  sprintf(debug_msg, "Otrzymano żądanie FIN - rozłączanie...\r\n");
					  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
					  Socket_disconnecting_process();
					  Socket_close();
					  Socket_LISTEN();
				  }
				  break;
			  }

			  case SOCK_CLOSE_WAIT:
				  sprintf(debug_msg, "Stan: SOCK_CLOSE_WAIT - czekam na rozłączenie\r\n");
				  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
				  Socket_disconnecting_process();
				  break;

			  case SOCK_CLOSED:
				  sprintf(debug_msg, "Stan: SOCK_CLOSED - zamykam gniazdo i restartuję\r\n");
				  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
				  Socket_close();
				  Socket_START();
				  break;

			  default:
				  break;
		  }

		  // ⏱️ Sprawdzenie timeoutu
		  if (Socket_check_timeout()) {
			  sprintf(debug_msg, "Timeout - zamykam gniazdo\r\n");
			  HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
			  Socket_close();
			  Socket_START();
		  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC10 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
