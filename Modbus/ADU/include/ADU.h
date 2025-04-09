#ifndef ADU_H
#define ADU_H

#include <stdint.h>

// Makrodefinicje
#define BUFFERSIZE 600

// Struktura reprezentująca Application Data Unit (ADU)
struct ADU {
    int8_t Unit_ID;                      // 1 bajt (8-bitowy)
    int8_t Function_Code;               // 1 bajt (8-bitowy)
    int8_t Transaction_Identifier[2];   // 2 bajty (16-bitowe)
    int8_t Protocol_Identifier[2];      // Zawsze 0, więc 2 bajty (16-bitowe)
    int8_t Length_Field[2];             // 2 bajty
    int8_t *Data;                       // Wskaźnik na dane (dynamicznie alokowane)
};

// Struktura przechowująca dostępne identyfikatory transakcji
struct FREE_IDENTIFIERS {
    int16_t list_of_free_transaction_identifiers[BUFFERSIZE];
};

// Funkcje
#ifdef __cplusplus
extern "C" {
#endif

struct FREE_IDENTIFIERS* init_IDENTIFIERS(struct FREE_IDENTIFIERS* identifiers);

struct ADU* init_ADU(
    struct ADU* adu,
    struct FREE_IDENTIFIERS* identifiers,
    uint16_t length,
    uint8_t unit_id,
    uint8_t function_code,
    int8_t* data
);

void destroy_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers);

#ifdef __cplusplus
}
#endif

#endif // ADU_H
