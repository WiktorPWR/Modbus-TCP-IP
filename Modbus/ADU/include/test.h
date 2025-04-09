#include "D:/Pulpit/PWR/Modbus/Modbus-TCP-IP/Modbus/ADU/ADU.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
void test_init_ADU() {
    printf("Poczatek testu\n");
    struct FREE_IDENTIFIERS identifiers;
    struct ADU adu;
    int8_t data[] = {0x01, 0x02, 0x03, 0x04};
    
    init_IDENTIFIERS(&identifiers);
    
    struct ADU* adu_ptr = init_ADU(&adu, &identifiers, 6, 1, 3, data);
    
    if (adu_ptr != NULL) {
        printf("Transaction Identifier: %02X%02X\n", adu_ptr->Transaction_Identifier[0], adu_ptr->Transaction_Identifier[1]);
        printf("Protocol Identifier: %02X%02X\n", adu_ptr->Protocol_Identifier[0], adu_ptr->Protocol_Identifier[1]);
        printf("Length Field: %02X%02X\n", adu_ptr->Length_Field[0], adu_ptr->Length_Field[1]);
        printf("Unit ID: %02X\n", adu_ptr->Unit_ID);
        printf("Function Code: %02X\n", adu_ptr->Function_Code);
        printf("Data: ");
        for (int i = 0; i < 4; i++) {
            printf("%02X ", adu_ptr->Data[i]);
        }
        printf("\n");

        // Sprawdzamy długość i zawartość danych
        if (memcmp(adu_ptr->Data, data, 4) == 0) {
            printf("Dane sa poprawne.\n");
        } else {
            printf("Dane sa niepoprawne.\n");
        }

        destroy_ADU(adu_ptr, &identifiers);
    } else {
        printf("Błąd alokacji pamięci.\n");
    }
}

void test_destroy_ADU() {
    struct FREE_IDENTIFIERS identifiers;
    struct ADU* adu_ptr;
    int8_t data[] = {0x01, 0x02, 0x03, 0x04};
    
    init_IDENTIFIERS(&identifiers);
    
    adu_ptr = init_ADU(NULL, &identifiers, 6, 1, 3, data);
    
    // Sprawdzamy, czy udało się stworzyć obiekt ADU
    if (adu_ptr != NULL) {
        printf("Obiekt ADU został stworzony.\n");

        // Teraz zwalniamy pamięć
        destroy_ADU(adu_ptr, &identifiers);
        printf("Pamięć obiektu ADU została zwolniona.\n");
    } else {
        printf("Błąd alokacji pamięci.\n");
    }
}

void test_transaction_identifier_management() {
    struct FREE_IDENTIFIERS identifiers;
    struct ADU adu1, adu2;
    int8_t data[] = {0x01, 0x02, 0x03, 0x04};
    
    init_IDENTIFIERS(&identifiers);

    // Inicjalizujemy pierwszy obiekt ADU
    struct ADU* adu_ptr1 = init_ADU(&adu1, &identifiers, 6, 1, 3, data);
    printf("Pierwszy identyfikator transakcji: %02X%02X\n", adu_ptr1->Transaction_Identifier[0], adu_ptr1->Transaction_Identifier[1]);

    // Zwalniamy pamięć po pierwszym obiekcie ADU
    destroy_ADU(adu_ptr1, &identifiers);
    printf("Pamięć po pierwszym ADU została zwolniona.\n");

    // Inicjalizujemy drugi obiekt ADU, powinien dostać ten sam identyfikator
    struct ADU* adu_ptr2 = init_ADU(&adu2, &identifiers, 6, 1, 3, data);
    printf("Drugi identyfikator transakcji: %02X%02X\n", adu_ptr2->Transaction_Identifier[0], adu_ptr2->Transaction_Identifier[1]);

    // Zwalniamy pamięć po drugim obiekcie ADU
    destroy_ADU(adu_ptr2, &identifiers);
}