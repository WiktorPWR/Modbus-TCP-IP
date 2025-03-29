#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFERSIZE 600

struct ADU{
    int8_t Unit_ID;                   // 1 bajt (8-bitowy)
    int8_t Function_Code;             // 1 bajt (8-bitowy)
    int8_t Transaction_Identifier[2];  // 2 bajty (16-bitowe)
    int8_t Protocol_Identifier[2];     // Zawsze 0, więc 2 bajty (16-bitowe)
    int8_t Length_Field[2];            // 2 bajty  
    int8_t *Data;                     // Dynamicznie alokowana tablica
};

struct FREE_IDENTIFIERS{
    int16_t list_of_free_transaction_identifiers[600];
};

struct FREE_IDENTIFIERS* init_IDENTIFIERS(struct FREE_IDENTIFIERS* identifiers){
    memset(identifiers->list_of_free_transaction_identifiers, 0, sizeof(identifiers->list_of_free_transaction_identifiers));
    printf("INITIALIZED IDENTIFIERS:\n");
    for (int i = 0; i < 600; i++) {  // Print first 10 elements for brevity
        identifiers->list_of_free_transaction_identifiers[i] = 1;
        //printf("identifier[%d] = %d\n", i, identifiers->list_of_free_transaction_identifiers[i]);
    }
    return identifiers;
}

struct ADU* init_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers, uint16_t lenght, uint8_t unit_id, uint8_t function_code, int8_t* data)
{
    // Szukamy wolnego adresu transakcji
    printf("Initializing ADU...\n");
    for (uint16_t i = 0; i < 600; i++) {
        if (identifiers->list_of_free_transaction_identifiers[i] == 1) {
            adu->Transaction_Identifier[0] = (i >> 8) & 0xFF;
            adu->Transaction_Identifier[1] = i & 0xFF;
            printf("Found free transaction identifier: %d\n", i);
            break;
        }
    }

    // Zawsze zero dla Modbusa
    adu->Protocol_Identifier[0] = 0x00;
    adu->Protocol_Identifier[1] = 0x00;
    printf("Protocol Identifier: %02X %02X\n", adu->Protocol_Identifier[0], adu->Protocol_Identifier[1]);

    // Ustawienie długości całej ramki (DANE + UNIT_ID + FUNCTION)
    adu->Length_Field[0] = (lenght >> 8) & 0xFF;
    adu->Length_Field[1] = lenght & 0xFF;
    printf("Length Field: %02X %02X\n", adu->Length_Field[0], adu->Length_Field[1]);

    // Ustawienie ID jednostki
    adu->Unit_ID = unit_id;
    printf("Unit ID: %02X\n", adu->Unit_ID);

    // Funkcja, którą użytkownik ustala
    adu->Function_Code = function_code;
    printf("Function Code: %02X\n", adu->Function_Code);

    // Alokacja pamięci na dane
    adu->Data = (int8_t*)malloc(lenght - 2); // Alokujemy pamięć na dane
    if (adu->Data == NULL) {
        // Obsługa błędu alokacji pamięci
        printf("Memory allocation for data failed!\n");
        return NULL;
    }
    printf("Memory allocated for Data.\n");

    // Kopiowanie danych
    memcpy(adu->Data, data, lenght);
    printf("Data copied to ADU.\n");
    return adu;
}

void destroy_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers){
    if (adu) {
        uint16_t id = (adu->Transaction_Identifier[1] << 8) | (adu->Transaction_Identifier[0]);
        printf("Wartosc 0 -> 0x%02X \n",adu->Transaction_Identifier[0]);
        printf("Wartosc 1 -> 0x%02X \n",adu->Transaction_Identifier[1]);
        printf("Destroying ADU with Transaction ID: %d\n", id);
        identifiers->list_of_free_transaction_identifiers[id] = 1; // Zwalnianie miejsca, można znowu używać
        printf("Transaction ID %d marked as free.\n", id);
        
        free(adu->Data);
        printf("Data memory freed.\n");

        //free(adu);
        printf("ADU memory freed.\n");
        //adu = NULL;
    }
}
