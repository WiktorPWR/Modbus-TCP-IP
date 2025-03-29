


char* conversion_to_binary(const char* hex_text, int size_of_data_in_bytes) {
    if (strlen(hex_text) / 2 >= size_of_data_in_bytes) {
        printf("Error: Provided size does not match hex data length.\n");
        return NULL;
    }
    
    char *array = (char*)malloc(8 * size_of_data_in_bytes + 1);
    if (!array) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    memset(array, '0', 8 * size_of_data_in_bytes);
    array[8 * size_of_data_in_bytes] = '\0'; // Null-terminator

    int index = 8 * size_of_data_in_bytes - 1;
    for (int i = 0; i <= size_of_data_in_bytes - 1; i++) {
        unsigned char value;
        sscanf(&hex_text[i * 2], "%2hhx", &value); // Konwersja z hex do bajta
        for (int bit = 7; bit >= 0; bit--) {
            array[index--] = (value & 1) ? '1' : '0';
            value >>= 1;
        }
    }
    
    return array;
}



// Funkcja konwertująca liczby szesnastkowe na binarne
void conversion_to_binary_and_save(const char* hex_text, char* array, int size_of_data_in_bytes) {
    int index = 0;
    for (int i = 0; i < size_of_data_in_bytes; i++) {
        unsigned int value;
        sscanf(&hex_text[i * 2], "%2x", &value); // Odczytujemy 2 znaki hex jako 1 bajt
        for (int bit = 7; bit >= 0; bit--) {
            array[index++] = (value & (1 << bit)) ? '1' : '0'; // Przekształcamy każdy bit na '0' lub '1'
        }
    }
    array[index] = '\0'; // Null-terminator
}
