#include <stdio.h>

#include "Funcktions_modbus.h"
#include "Errors_modbus.h"

char charToHex(char input, char *output) {
    sprintf(output, "0x%X", input);
    return *output;
}

void robienie_bajtu_modbusa(char input, char nowa_ramka[12]) {
    char binarny_znak[9]; // Bufor na 8-bitową reprezentację binarną + znak końca

    // Konwersja bajtu wejściowego na binarną reprezentację ASCII ('0' lub '1')
    for (int i = 7; i >= 0; i--) {
        binarny_znak[7 - i] = (input & (1 << i)) ? '1' : '0';
    }
    binarny_znak[8] = '\0'; // Dodanie znaku końca stringa

    // Dodanie bitu startu (zawsze 0) na początek ramki
    nowa_ramka[0] = '0';

    // Kopiowanie 8-bitowego ciągu do ramki
    for (int i = 0; i < 8; i++) {
        nowa_ramka[i + 1] = binarny_znak[i];
    }

    // Liczenie liczby jedynek dla bitu parzystości
    int licznik_jedynek = 0;
    for (int i = 0; i < 8; i++) {
        if (binarny_znak[i] == '1') {
            licznik_jedynek++;
        }
    }

    // Dodanie bitu parzystości (1 dla parzystej liczby jedynek, 0 dla nieparzystej)
    nowa_ramka[9] = (licznik_jedynek % 2 == 0) ? '1' : '0';

    // Dodanie bitu stopu (zawsze 1)
    nowa_ramka[10] = '1';

    // Dodanie znaku końca stringa
    nowa_ramka[11] = '\0';
}

void create_PDU(int function,)



int main() {
    
    
    
    return 0;
}
