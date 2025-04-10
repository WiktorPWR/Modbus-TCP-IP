#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "D:/Pulpit/PWR/Modbus/Modbus-TCP-IP/Modbus/ADU/include/ADU.h"  // <-- Twój nagłówek z deklaracjami struktur i funkcji

struct FREE_IDENTIFIERS* init_IDENTIFIERS(struct FREE_IDENTIFIERS* identifiers) {
    memset(identifiers->list_of_free_transaction_identifiers, 0, sizeof(identifiers->list_of_free_transaction_identifiers));
    for (int i = 0; i < BUFFERSIZE; i++) {
        identifiers->list_of_free_transaction_identifiers[i] = 1;
    }
    return identifiers;
}

struct ADU* init_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers, uint16_t length, uint8_t unit_id, uint8_t function_code, int8_t* data) {
    for (uint16_t i = 0; i < BUFFERSIZE; i++) {
        if (identifiers->list_of_free_transaction_identifiers[i] == 1) {
            adu->Transaction_Identifier[0] = (i >> 8) & 0xFF;
            adu->Transaction_Identifier[1] = i & 0xFF;
            identifiers->list_of_free_transaction_identifiers[i] = 0;  // oznaczamy jako zajęty
            break;
        }
    }

    adu->Protocol_Identifier[0] = 0x00;
    adu->Protocol_Identifier[1] = 0x00;

    adu->Length_Field[0] = (length >> 8) & 0xFF;
    adu->Length_Field[1] = length & 0xFF;

    adu->Unit_ID = unit_id;
    adu->Function_Code = function_code;

    adu->Data = (int8_t*)malloc(length - 2);
    if (adu->Data == NULL) {
        return NULL;
    }

    memcpy(adu->Data, data, length);
    return adu;
}

void destroy_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers) {
    if (adu) {
        uint16_t id = (adu->Transaction_Identifier[0] << 8) | adu->Transaction_Identifier[1];
        identifiers->list_of_free_transaction_identifiers[id] = 1;

        free(adu->Data);
        adu->Data = NULL;
    }
}
