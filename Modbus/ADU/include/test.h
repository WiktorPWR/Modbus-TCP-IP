#include "D:/Pulpit/PWR/Modbus/Modbus-TCP-IP/Modbus/ADU/include/ADU.h"
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



void test_received_frame() {
    printf("Początek testu received_frame()\n");

    struct FREE_IDENTIFIERS identifiers;
    struct ADU adu;

    // Przykładowa ramka ADU:
    // Transaction ID: 0x1234
    // Protocol ID: 0x0000
    // Length: 0x0006 (czyli 6 bajtów = Unit ID + Function Code + 4 bajty danych)
    // Unit ID: 0x01
    // Function Code: 0x03
    // Dane: 0x11 0x22 0x33 0x44
    uint8_t frame[] = {
        0x00, 0x12,         // Transaction ID
        0x00, 0x00,         // Protocol ID
        0x00, 0x06,         // Length
        0x01,               // Unit ID
        0x03,               // Function Code
        0x11, 0x22, 0x33, 0x44 // Dane
    };

    // Inicjalizacja listy identyfikatorów
    init_IDENTIFIERS(&identifiers);

    // Wywołujemy funkcję, która przetwarza odebraną ramkę
    received_frame(&adu, &identifiers, frame);

    printf("Wyszlismy\n");
    // Oczekiwany Transaction ID w liczbie: 0x1234 = 4660
    uint16_t expected_id = 0x0012;

    // Sprawdzanie wyników
    printf("Transaction Identifier: %02X%02X\n", adu.Transaction_Identifier[0], adu.Transaction_Identifier[1]);
    printf("Protocol Identifier: %02X%02X\n", adu.Protocol_Identifier[0], adu.Protocol_Identifier[1]);
    printf("Length Field: %02X%02X\n", adu.Length_Field[0], adu.Length_Field[1]);
    printf("Unit ID: %02X\n", adu.Unit_ID);
    printf("Function Code: %02X\n", adu.Function_Code);

    printf("Data: ");
    for (int i = 8; i < 12; i++) {
        printf("%02X ", adu.Data[i]);
    }
    printf("\n");

    // Sprawdzenie, czy Transaction ID został oznaczony jako zajęty
    if (identifiers.list_of_free_transaction_identifiers[expected_id] == 0) {
        printf("ID 0x%04X zostało poprawnie oznaczone jako zajęte.\n", expected_id);
    } else {
        printf("BŁĄD: ID 0x%04X nie zostało oznaczone jako zajęte!\n", expected_id);
    }

    // Sprawdzenie danych
    if (memcmp(&adu.Data[8], &frame[8], 4) == 0) {
        printf("Dane odebrane poprawnie.\n");
    } else {
        printf("BŁĄD: dane nie zgadzają się!\n");
    }

    printf("Koniec testu received_frame()\n\n");
}
