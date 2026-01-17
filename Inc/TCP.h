/*
 * TCP.h
 *
 *  Created on: Jun 14, 2025
 *      Author: ostro
 */

#ifndef INC_TCP_H_
#define INC_TCP_H_


#define LISTEN_PORT 5000

#define W5500_CS_GPIO    GPIOA
#define W5500_CS_PIN     GPIO_PIN_5

// Common Registers
#define W5500_PHYCFGR           0x002E
#define W5500_RXMEM_SIZE_REG    0x001E
#define W5500_TXMEM_SIZE_REG    0x001F
#define W5500_GAR				0x0001
#define W5500_SUBR				0x0005
#define W5500_SHAR              0x0009  // MAC address base address
#define W5500_SIPR				0x000F

// Socket 0 Registers (BSB = 0x01 for Socket 0)
#define W5500_S0_MR             0x0000  // Mode Register
#define W5500_S0_CR             0x0001  // Command Register
#define W5500_S0_IR             0x0002  // Interrupt Register
#define W5500_S0_SR             0x0003  // Status Register
#define W5500_S0_PORT           0x0004  // Source Port (0x0004 & 0x0005)
#define W5500_S0_TX_FSR         0x0020  // TX Free Size Register
#define W5500_S0_TX_RD          0x0022  // TX Read Pointer Register
#define W5500_S0_TX_WR          0x0024  // TX Write Pointer Register
#define W5500_S0_RX_RSR         0x0026  // RX Received Size Register
#define W5500_S0_RX_RD          0x0028  // RX Read Pointer Register

// Block Address
#define BSB_COMMON        0x00
#define BSB_SOCKET0       0x01
#define BSB_SOCKET0_TX    0x02
#define BSB_SOCKET0_RX    0x03
// Socket Commands
#define CMD_OPEN         0x01
#define CMD_LISTEN       0x02
#define CMD_CONNECT      0x04
#define CMD_DISCON       0x08
#define CMD_CLOSE        0x10
#define CMD_SEND         0x20
#define CMD_RECV         0x40

// Socket Status
#define SOCK_CLOSED       0x00
#define SOCK_INIT         0x13
#define SOCK_LISTEN       0x14
#define SOCK_SYNRECV	  0x16
#define SOCK_ESTABLISHED  0x17
#define SOCK_CLOSE_WAIT   0x1C

#define W5500_GAR   0x0001  // Gateway Address (4 bajty)
#define W5500_SUBR  0x0005  // Subnet Mask (4 bajty)
#define W5500_SHAR  0x0009  // MAC Address (6 bajtów)
#define W5500_SIPR  0x000F  // Source IP Address (4 bajty)


void W5500_Select(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO, W5500_CS_PIN, GPIO_PIN_RESET);
}

void W5500_Unselect(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO, W5500_CS_PIN, GPIO_PIN_SET);
}

void W5500_Reset_set(void){
	HAL_GPIO_WritePin(GPIOB, 5, GPIO_PIN_SET);
}
void W5500_Reset_reset(void){
	HAL_GPIO_WritePin(GPIOB, 5, GPIO_PIN_RESET);
}


void W5500_Write(uint16_t offset, uint8_t bsb, uint8_t* data, uint8_t size_of_data) {
    uint8_t message_to_send[3 + size_of_data];

    // Ustawianie nagłówka SPI (2 bajty offsetu + 1 bajt control)
    message_to_send[0] = (offset >> 8) & 0xFF;  // MSB adresu
    message_to_send[1] = offset & 0xFF;         // LSB adresu
    message_to_send[2] = (bsb << 3) | (1 << 2) | 0x00;  // RWB=0, OM=00 (VDM)

    // Kopiowanie danych do ramki
    memcpy(&message_to_send[3],data,size_of_data);

    // Wysyłanie przez SPI z HAL
    W5500_Select();
    HAL_SPI_Transmit(&hspi1, message_to_send, 3 + size_of_data,HAL_MAX_DELAY);
    W5500_Unselect();  // jeśli DMA nie ruszyło, zwolnij CS
}

void W5500_Read(uint16_t offset, uint8_t bsb, uint8_t *buf, uint16_t len) {
    uint8_t header[3] = {
        (uint8_t)(offset >> 8),
        (uint8_t)offset,
        (bsb << 3) | (0 << 2) | 0x00  // RWB=0 (read), OM=00
    };

    W5500_Select();
	HAL_SPI_Transmit(&hspi1, header, 3, HAL_MAX_DELAY);

	HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);

	W5500_Unselect();
}

void W5500_PrintMAC(void) {
    uint8_t mac[6];
    char out_buf[64];

    W5500_Read(W5500_SHAR, 0x00, &mac[0], 6);  // offset = 0x0009, BSB = 0x00 (Common Reg)

    snprintf(out_buf, sizeof(out_buf),
             "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    HAL_UART_Transmit(&huart2, (uint8_t*)out_buf, strlen(out_buf), HAL_MAX_DELAY);
}

void W5500_PrintIP(void) {
    uint8_t ip[4];
    char out_buf[64];

    W5500_Read(W5500_SIPR, 0x00, ip, 4);  // offset = 0x000F, BSB = 0x00 (Common Reg, SIPR - Source IP Address)

    snprintf(out_buf, sizeof(out_buf),
             "IP Address: %d.%d.%d.%d\r\n",
             ip[0], ip[1], ip[2], ip[3]);

    HAL_UART_Transmit(&huart2, (uint8_t*)out_buf, strlen(out_buf), HAL_MAX_DELAY);

}

void W5500_PrintSubnet(void) {
    uint8_t subnet[4];
    char out_buf[64];

    W5500_Read(W5500_SUBR, 0x00, subnet, 4);  // SUBR - Subnet Mask Address

    snprintf(out_buf, sizeof(out_buf),
             "Subnet Mask: %d.%d.%d.%d\r\n",
             subnet[0], subnet[1], subnet[2], subnet[3]);

    HAL_UART_Transmit(&huart2, (uint8_t*)out_buf, strlen(out_buf), HAL_MAX_DELAY);
}

void W5500_PrintGateway(void) {
    uint8_t gateway[4];
    char out_buf[64];

    W5500_Read(W5500_GAR, 0x00, gateway, 4);  // GAR - Gateway Address

    snprintf(out_buf, sizeof(out_buf),
             "Gateway: %d.%d.%d.%d\r\n",
             gateway[0], gateway[1], gateway[2], gateway[3]);

    HAL_UART_Transmit(&huart2, (uint8_t*)out_buf, strlen(out_buf), HAL_MAX_DELAY);
}


void W5500_TestVersion(void) {
    uint8_t version;
    W5500_Read(0x0039, 0x00, &version, 1);

    char buf[64];
    snprintf(buf, sizeof(buf), "W5500 VERSIONR: 0x%02X\r\n", version);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}


void W5500_Set_GATEWAYIP(uint8_t* gateway_ip){
	for(uint8_t i = 0; i < 4; i++){
		W5500_Write(W5500_GAR + i, 0x00, &gateway_ip[i], 1);
	}
}

void W5500_Set_MASKIP(uint8_t* mask_ip) {
    for (uint8_t i = 0; i < 4; i++) {
        W5500_Write(W5500_SUBR + i, 0x00, &mask_ip[i], 1);
    }
}


void W5500_SetMAC(uint8_t *mac) {
    for (uint8_t i = 0; i < 6; i++) {
        W5500_Write(W5500_SHAR + i, 0x00, &mac[i], 1);
    }
}

void W5500_SetIP(uint8_t *ip){
	for(uint8_t i = 0; i < 4; i++){
		W5500_Write(W5500_SIPR + i, 0x00, &ip[i],1);
	}
}


void Socket_START(){
    uint8_t buffer;

    // 1. Set TCP mode
    buffer = 0x01;  // TCP
    W5500_Write(W5500_S0_MR, BSB_SOCKET0, &buffer, sizeof(buffer));

    // 2. Set local port (np. 502)
    uint16_t port_number = LISTEN_PORT;
    uint8_t port_bytes[2];
    port_bytes[0] = (port_number >> 8) & 0xFF;
    port_bytes[1] = port_number & 0xFF;
    W5500_Write(W5500_S0_PORT, BSB_SOCKET0, port_bytes, 2);

    // (opcjonalnie) 3. Set TX/RX buffer size to 2 KB
    // Ustaw RX i TX buffer na 2 KB
    buffer = 0x08;  // 2KB
    W5500_Write(0x001E, BSB_SOCKET0, &buffer, 1);  // RXBUF_SIZE
    W5500_Write(0x001F, BSB_SOCKET0, &buffer, 1);  // TXBUF_SIZE


    // 4. Issue OPEN command
    buffer = CMD_OPEN;
    W5500_Write(W5500_S0_CR, BSB_SOCKET0, &buffer, 1);

    // 5. Wait until SOCK_INIT
    uint8_t status = 0;
    do {
        W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, 1);
    } while (status != SOCK_INIT);


    char debug_msg[64];
    sprintf(debug_msg, "Stan: SOCKet Start udalo sie\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

}

void Socket_LISTEN() {
    uint8_t status = 0;
    char debug_msg[72];

    // Odczytaj obecny status socketu
    W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, 1);
    sprintf(debug_msg, "Socket_LISTEN: obecny stan = 0x%02X\r\n", status);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // Jeśli socket jest w stanie SOCK_INIT, możemy go ustawić w tryb nasłuchu
    if (status == SOCK_INIT) {
        uint8_t cmd = CMD_LISTEN;
        W5500_Write(W5500_S0_CR, BSB_SOCKET0, &cmd, 1);

        // Czekaj aż socket przejdzie do innego stanu niż SOCK_INIT (np. do SOCK_LISTEN)
        uint32_t timeout = 100000; // prosty timeout zapobiegający zawieszeniu
        do {
            W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, 1);
            timeout--;
        } while (status == SOCK_INIT && timeout > 0);

        // Sprawdź, czy udało się przejść w tryb LISTEN
        if (status == SOCK_LISTEN) {
            sprintf(debug_msg, "Socket_LISTEN: OK, socket w stanie SOCK_LISTEN (0x%02X)\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
        } else {
            sprintf(debug_msg, "Socket_LISTEN: BŁĄD! oczekiwano SOCK_LISTEN, otrzymano 0x%02X\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
            // (Opcjonalnie) zamknij socket i zainicjuj ponownie
            // Socket_CLOSE();
            // Socket_INIT();
        }
    } else {
        // Socket nie był gotowy do nasłuchu
        sprintf(debug_msg, "Socket_LISTEN: nie w stanie SOCK_INIT (0x%02X) - LISTEN pominięte\r\n", status);
        HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    }
}

void Socket_ESTABLISHED() {
    uint8_t status = 0;
    do {
        W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, 1);
    } while (status != SOCK_ESTABLISHED);

    char debug_msg[64];
    sprintf(debug_msg, "Stan: SOCKET ESTABLISHED\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
}

uint16_t Socket_check_received_data() {
    uint8_t received_data[2];// 0 - false , 1 - true
    W5500_Read(W5500_S0_RX_RSR, BSB_SOCKET0, received_data, sizeof(received_data));
    uint16_t size_of_data_received = (received_data[0] << 8) | received_data[1];
    return size_of_data_received;
}

uint8_t* Socket_receiving_process(uint16_t rx_len) {
    uint8_t ptr_buf[2];
    W5500_Read(W5500_S0_RX_RD, BSB_SOCKET0, ptr_buf, 2);
    uint16_t rx_rd = (ptr_buf[0] << 8) | ptr_buf[1];

    uint8_t* data_buffer = malloc(rx_len);
    if (data_buffer == NULL) return NULL;

    uint16_t rx_buf_mask = 0x07FF;

    if ((rx_rd + rx_len) <= (rx_buf_mask + 1)) {
        W5500_Read(rx_rd, BSB_SOCKET0_RX, data_buffer, rx_len);
    } else {
        uint16_t first_part = (rx_buf_mask + 1) - rx_rd;
        W5500_Read(rx_rd, BSB_SOCKET0_RX, data_buffer, first_part);
        W5500_Read(0, BSB_SOCKET0_RX, data_buffer + first_part, rx_len - first_part);
    }

    // zaktualizuj RX_RD
    uint16_t new_rd = (rx_rd + rx_len) & rx_buf_mask;
    uint8_t new_rd_buf[2] = { new_rd >> 8, new_rd & 0xFF };
    W5500_Write(W5500_S0_RX_RD, BSB_SOCKET0, new_rd_buf, 2);

    // wyślij CMD_RECV
    uint8_t cmd = CMD_RECV;
    W5500_Write(W5500_S0_CR, BSB_SOCKET0, &cmd, 1);
    do {
        W5500_Read(W5500_S0_CR, BSB_SOCKET0, &cmd, 1);
    } while (cmd != 0);

    // opcjonalne czyszczenie flagi RECV
    uint8_t clear_ir = 0x04;
    W5500_Write(W5500_S0_IR, BSB_SOCKET0, &clear_ir, 1);

    return data_buffer;
}

void Socket_send_command(uint8_t cmd) {
    // Zapisz CMD_SEND do rejestru Sn_CR (offset W5500_S0_CR) w bloku BSB_SOCKET0
    W5500_Write(W5500_S0_CR, BSB_SOCKET0, &cmd, 1);

    // Czekaj aż polecenie się wykona - rejestr Sn_CR wróci do 0
    uint8_t cr = 0xFF;
    do {
        W5500_Read(W5500_S0_CR, BSB_SOCKET0, &cr, 1);
    } while(cr != 0);
}

#define IR_SEND_OK         0x10

void Socket_send_proces(uint8_t* data, uint16_t len) {
	char debug_msg[128];
    // 0. Weryfikacja długości
    if (data == NULL || len == 0 || len > 2048) {
        sprintf(debug_msg, "Send: Invalid data or len = %u\r\n", len);
        HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
        return;
    }

    //sprintf(debug_msg, "Send: Requested len = %u\r\n", len);
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 1. Sprawdź, czy socket jest w stanie SOCK_ESTABLISHED
    uint8_t sr;
    W5500_Read(W5500_S0_SR, BSB_SOCKET0, &sr, 1);
    //sprintf(debug_msg, "Socket Status: 0x%02X\r\n", sr);
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    if (sr != SOCK_ESTABLISHED) return;

    // 2. Sprawdź, czy jest wystarczająco miejsca w buforze TX
    uint8_t tx_fsr_buf[2];
    W5500_Read(W5500_S0_TX_FSR, BSB_SOCKET0, tx_fsr_buf, 2);
    uint16_t tx_fsr = (tx_fsr_buf[0] << 8) | tx_fsr_buf[1];
    //sprintf(debug_msg, "TX Free Size: %u\r\n", tx_fsr);
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    if (tx_fsr < len) return;

    // 3. Odczytaj wskaźnik TX_WR (Write Pointer)
    uint8_t tx_wr_buf[2];
    W5500_Read(W5500_S0_TX_WR, BSB_SOCKET0, tx_wr_buf, 2);
    uint16_t tx_wr = (tx_wr_buf[0] << 8) | tx_wr_buf[1];
    //sprintf(debug_msg, "TX_WR before: 0x%04X\r\n", tx_wr);
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 4. Oblicz offset w buforze (wraparound na 2KB buforze)
    uint16_t tx_offset = tx_wr & 0x1FFF;
    uint16_t tx_addr   = 0x4000 + tx_offset;
    //sprintf(debug_msg, "TX Offset: 0x%04X, TX Addr: 0x%04X\r\n", tx_offset, tx_addr);
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 5. Zapisz dane do bufora TX, z obsługą wraparound
    uint16_t space_to_end = 8192 - tx_offset;
    if (len <= space_to_end) {
        W5500_Write(tx_addr, BSB_SOCKET0_TX, data, len);
        sprintf(debug_msg, "TX Write: No wrap, len = %u\r\n", len);
    } else {
        W5500_Write(tx_addr, BSB_SOCKET0_TX, data, space_to_end);
        W5500_Write(0x4000, BSB_SOCKET0_TX, data + space_to_end, len - space_to_end);
        sprintf(debug_msg, "TX Write: Wrap, first = %u, second = %u\r\n", space_to_end, len - space_to_end);
    }
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 6. Aktualizuj TX_WR
    tx_wr += len;
    tx_wr_buf[0] = (tx_wr >> 8) & 0xFF;
    tx_wr_buf[1] = tx_wr & 0xFF;
    W5500_Write(W5500_S0_TX_WR, BSB_SOCKET0, tx_wr_buf, 2);
    //sprintf(debug_msg, "TX_WR updated: 0x%04X\r\n", tx_wr);
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 7. Wydaj komendę SEND
    uint8_t cmd = CMD_SEND;
    W5500_Write(W5500_S0_CR, BSB_SOCKET0, &cmd, 1);
    //sprintf(debug_msg, "SEND command issued\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 8. Czekaj na SEND_OK
    uint8_t ir;
    do {
        W5500_Read(W5500_S0_IR, BSB_SOCKET0, &ir, 1);
    } while (!(ir & IR_SEND_OK));
    //sprintf(debug_msg, "SEND_OK received\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    // 9. Wyczyść flagę SEND_OK
    ir = IR_SEND_OK;
    W5500_Write(W5500_S0_IR, BSB_SOCKET0, &ir, 1);
    //sprintf(debug_msg, "SEND_OK cleared\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
}

void Socket_close() {
    uint8_t cmd = CMD_CLOSE;
    W5500_Write(W5500_S0_CR, BSB_SOCKET0, &cmd, 1);

    // Czekaj aż faktycznie przejdzie do SOCK_CLOSED
    uint8_t status = 0;
    uint32_t timeout = HAL_GetTick() + 1000;
    do {
        W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, 1);
        if (HAL_GetTick() > timeout) break;
    } while (status != SOCK_CLOSED);

    char debug_msg[64];
    sprintf(debug_msg, "Socket zamknięty, stan: 0x%02X\r\n", status);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
}

uint8_t Socket_check_disconnect_request() {
    uint8_t status;
    W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, sizeof(status));
    uint8_t sock_close_wait = 0;
    if (status == SOCK_CLOSE_WAIT) {
        sock_close_wait = 1;
    }
    return sock_close_wait;
}

void Socket_disconnecting_process() {
    uint8_t buffer = CMD_DISCON;
    W5500_Write(W5500_S0_CR, BSB_SOCKET0, &buffer, sizeof(buffer));
}

uint8_t Socket_check_closed() {
    uint8_t status;
    W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, sizeof(status));
    uint8_t sock_closed = 0;
    if (status == SOCK_CLOSED) {
    	sock_closed = 1;
    }
    return sock_closed;
}

uint8_t Socket_check_timeout() {
    uint8_t status;
    W5500_Read(W5500_S0_SR, BSB_SOCKET0, &status, sizeof(status));
    uint8_t sock_closed = 0;
    if(status == SOCK_CLOSED){
    	sock_closed = 1;
    }
    return sock_closed;

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        W5500_Unselect();  // ✅ CS puszczany po zakończeniu DMA
    }
}

void W5500_Configure_Network(){
    uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56};
    W5500_SetMAC(mac);

    uint8_t ip[4] = {192, 168, 137, 100};
    W5500_SetIP(ip);

    uint8_t subnet[4] = {255, 255, 255, 0};
    W5500_Set_MASKIP(subnet);

    uint8_t gateway[4] = {192, 168, 137, 1};
    W5500_Set_GATEWAYIP(gateway);

    // 🔧 Dodaj konfigurację timeoutu i retransmisji:
    uint8_t RTR[2] = { 0x27, 0x10 };  // 10000 (czyli 1 sekunda)
    W5500_Write(0x0019, BSB_COMMON, RTR, 2);

    uint8_t RCR = 10;  // maks. 10 retransmisji
    W5500_Write(0x001B, BSB_COMMON, &RCR, 1);
}

void W5500_Chec_Network_Configuration(){
	  W5500_PrintIP();
	  W5500_PrintMAC();
	  W5500_PrintSubnet();
	  W5500_PrintGateway();
}

#endif /* INC_TCP_H_ */
