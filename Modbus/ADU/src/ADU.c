#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "D:/Pulpit/PWR/Modbus/Modbus-TCP-IP/Modbus/ADU/include/ADU.h"  // Nagłówek z definicjami struktur i funkcji

/**
 * Inicjalizuje strukturę FREE_IDENTIFIERS.
 *
 * @param identifiers Wskaźnik na strukturę zawierającą listę wolnych identyfikatorów transakcji.
 * @return Wskaźnik na zainicjalizowaną strukturę FREE_IDENTIFIERS.
 */
struct FREE_IDENTIFIERS* init_IDENTIFIERS(struct FREE_IDENTIFIERS* identifiers) {
    // Zerujemy pamięć zajmowaną przez listę wolnych identyfikatorów transakcji
    memset(identifiers->list_of_free_transaction_identifiers, 0, sizeof(identifiers->list_of_free_transaction_identifiers));

    // Ustawiamy wszystkie identyfikatory jako wolne (wartość 1)
    for (int i = 0; i < BUFFERSIZE; i++) {
        identifiers->list_of_free_transaction_identifiers[i] = 1;
    }
    return identifiers;
}

/**
 * Inicjalizuje strukturę ADU na potrzeby wysyłania ramki.
 *
 * @param adu             Wskaźnik na strukturę ADU do wypełnienia.
 * @param identifiers     Wskaźnik na strukturę identyfikatorów transakcji.
 * @param length          Długość całkowita danych (łącznie z Unit_ID i Function_Code).
 * @param unit_id         Identyfikator jednostki (slave).
 * @param function_code   Kod funkcji Modbus.
 * @param data            Wskaźnik na dane do skopiowania do ADU.
 * @return Wskaźnik na zainicjalizowaną strukturę ADU lub NULL, jeśli alokacja pamięci się nie powiedzie.
 */
struct ADU* init_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers, uint16_t length, uint8_t unit_id, uint8_t function_code, int8_t* data) {
    // Szukamy wolnego identyfikatora transakcji
    for (uint16_t i = 0; i < BUFFERSIZE; i++) {
        if (identifiers->list_of_free_transaction_identifiers[i] == 1) {
            // Ustawiamy identyfikator transakcji w ADU (2 bajty)
            adu->Transaction_Identifier[0] = (i >> 8) & 0xFF; // bajt starszy
            adu->Transaction_Identifier[1] = i & 0xFF;        // bajt młodszy

            // Oznaczamy identyfikator jako zajęty
            identifiers->list_of_free_transaction_identifiers[i] = 0;
            break;
        }
    }

    // Ustawiamy Protocol Identifier (dla Modbus TCP to zawsze 0x0000)
    adu->Protocol_Identifier[0] = 0x00;
    adu->Protocol_Identifier[1] = 0x00;

    // Długość (Length Field) – zawiera długość dalszej części (Unit_ID + Function_Code + Data)
    adu->Length_Field[0] = (length >> 8) & 0xFF;
    adu->Length_Field[1] = length & 0xFF;

    // Ustawiamy Unit ID i Function Code
    adu->Unit_ID = unit_id;
    adu->Function_Code = function_code;

    // Alokujemy miejsce na dane (bez Unit ID i Function Code, stąd -2)
    adu->Data = (int8_t*)malloc(length - 2);
    if (adu->Data == NULL) {
        return NULL;
    }

    // Kopiujemy dane do struktury
    memcpy(adu->Data, data, length - 2);
    return adu;
}

/**
 * Zwalnia pamięć zajmowaną przez strukturę ADU i zwalnia identyfikator transakcji.
 *
 * @param adu          Wskaźnik na strukturę ADU do zwolnienia.
 * @param identifiers  Wskaźnik na strukturę identyfikatorów – by oznaczyć ID jako dostępne.
 */
void destroy_ADU(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers) {
    if (adu) {
        // Odzyskujemy Transaction ID z bajtów
        uint16_t id = (adu->Transaction_Identifier[0] << 8) | adu->Transaction_Identifier[1];

        // Oznaczamy identyfikator jako wolny
        identifiers->list_of_free_transaction_identifiers[id] = 1;

        // Zwalniamy dane
        free(adu->Data);
        adu->Data = NULL;
    }
}

/**
 * Odbiera ramkę ADU i wypełnia strukturę na podstawie odebranych bajtów.
 *
 * @param adu           Wskaźnik na strukturę ADU, która ma zostać wypełniona.
 * @param identifiers   Wskaźnik na strukturę identyfikatorów (opcjonalnie do obsługi).
 * @param data          Surowe dane odebrane w formie binarnej (ramka ADU w Modbus TCP).
 */
void received_frame(struct ADU* adu, struct FREE_IDENTIFIERS* identifiers, uint8_t* data) {
    // Wypełniamy Transaction Identifier (2 bajty)
    printf("Wchodzimy\n");
    adu->Transaction_Identifier[0] = data[0];
    adu->Transaction_Identifier[1] = data[1];

    // Wyciągamy Transaction ID jako liczba
    uint16_t Transaction_id = (data[0] << 8) | data[1];

    // Wypełniamy Protocol Identifier
    adu->Protocol_Identifier[0] = data[2];
    adu->Protocol_Identifier[1] = data[3];

    // Wypełniamy Length Field
    adu->Length_Field[0] = data[4];
    adu->Length_Field[1] = data[5];

    // Ustawiamy Unit ID i Function Code
    adu->Unit_ID = data[6];
    adu->Function_Code = data[7];
    printf("Klasyka\n");
    // Ustawiamy wskaźnik na dane – UWAGA: tutaj przypisujesz wskaźnik, nie kopiujesz danych!
    adu->Data = data;
    printf("Niespodzinaka\n");
    // Zaznaczenie w liscie kotre id jest zajete do komunikacji
    identifiers->list_of_free_transaction_identifiers[Transaction_id] = 0;
    printf("Cyli tutaj sie pierdoli\n");
}
