#include <stdio.h>

#include "Funckje.h"

char charToHex(char input, char *output) {
    sprintf(output, "0x%X", input);
    return *output;
}

void charToBinary(char input, char *output) {
    for (int i = 7; i >= 0; i--) {
        output[7 - i] = (input & (1 << i)) ? '1' : '0';
    }
    output[8] = '\0';
}

void robienie_bajtu_modbusa(char input, char nowa_ramka[12]) {
    char binarny_znak[9];
    for (int i = 7; i >= 0; i--) {
        binarny_znak[7 - i] = (input & (1 << i)) ? '1' : '0';
    }
    binarny_znak[8] = '\0';
    nowa_ramka[0] = '0';
    for (int i = 0; i < 8; i++) {
        nowa_ramka[i + 1] = binarny_znak[i];
    }
    int licznik_jedynek = 0;
    for (int i = 0; i < 8; i++) {
        if (binarny_znak[i] == '1') {
            licznik_jedynek++;
        }
    }
    nowa_ramka[9] = (licznik_jedynek % 2 == 0) ? '1' : '0';
    nowa_ramka[10] = '1';
    nowa_ramka[11] = '\0';
}




int main() {
    char testChar = 'F';
    char hexStr[5];
    char binStr[9];
    char ramka[12];
    charToHex(testChar, hexStr);
    charToBinary(testChar, binStr);
    robienie_bajtu_modbusa(testChar, ramka);
    printf("Znak: %c, Hex: %s, Bin: %s, Ramka: %s\n", testChar, hexStr, binStr, ramka);
    
    int error_code = 2;
    printf("Modbus Error %d: %s\n", error_code, modbus_error_message(error_code));
    
    int function_code = 3;
    printf("Modbus Function %d: %s\n", function_code, modbus_function_name(function_code));
    
    return 0;
}
