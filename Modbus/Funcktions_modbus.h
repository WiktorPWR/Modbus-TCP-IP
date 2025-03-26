

const char* modbus_function_name(int function_code) {
    switch (function_code) {
        case 1: return "Read Coils";
        case 2: return "Read Discrete Inputs";
        case 3: return "Read Multiple Holding Registers";
        case 4: return "Read Input Registers";
        case 5: return "Write Single Coil";
        case 6: return "Write Single Holding Register";
        case 7: return "Read Exception Status";
        case 8: return "Diagnostic";
        case 11: return "Get Com Event Counter";
        case 12: return "Get Com Event Log";
        case 15: return "Write Multiple Coils";
        case 16: return "Write Multiple Holding Registers";
        case 17: return "Report Slave ID";
        case 20: return "Read File Record";
        case 21: return "Write File Record";
        case 22: return "Mask Write Register";
        case 23: return "Read/Write Multiple Registers";
        case 24: return "Read FIFO Queue";
        case 43: return "Read Device Identification / Encapsulated Interface Transport";
        default: return "Unknown Function";
    }
}