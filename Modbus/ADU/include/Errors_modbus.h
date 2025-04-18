const char* modbus_error_message(int error_code) {
    switch (error_code) {
        case 1: return "Illegal Function";
        case 2: return "Illegal Data Address";
        case 3: return "Illegal Data Value";
        case 4: return "Slave Device Failure";
        case 5: return "Acknowledge";
        case 6: return "Slave Device Busy";
        case 7: return "Negative Acknowledge";
        case 8: return "Memory Parity Error";
        case 10: return "Gateway Path Unavailable";
        case 11: return "Gateway Target Device Failed to Respond";
        default: return "Unknown Error";
    }
}