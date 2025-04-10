#include <stdio.h>
#include "D:/Pulpit/PWR/Modbus/Modbus-TCP-IP/Modbus/ADU/include/modbus_functions.h"

void handle_modbus_function(struct ADU* adu) {
    switch (adu->Function_Code) {
        case 1: handle_read_coils(adu); break;
        case 2: handle_read_discrete_inputs(adu); break;
        case 3: handle_read_holding_registers(adu); break;
        case 4: handle_read_input_registers(adu); break;
        case 5: handle_write_single_coil(adu); break;
        case 6: handle_write_single_register(adu); break;
        case 7: handle_read_exception_status(adu); break;
        case 8: handle_diagnostic(adu); break;
        case 11: handle_get_comm_event_counter(adu); break;
        case 12: handle_get_comm_event_log(adu); break;
        case 15: handle_write_multiple_coils(adu); break;
        case 16: handle_write_multiple_registers(adu); break;
        case 17: handle_report_slave_id(adu); break;
        case 20: handle_read_file_record(adu); break;
        case 21: handle_write_file_record(adu); break;
        case 22: handle_mask_write_register(adu); break;
        case 23: handle_read_write_multiple_registers(adu); break;
        case 24: handle_read_fifo_queue(adu); break;
        case 43: handle_encapsulated_interface_transport(adu); break;
        default:
            printf("Unknown Modbus function code: %d\n", adu->Function_Code);
            break;
    }
}

// Funkcje obsługujące konkretne funkcje Modbus
void handle_read_coils(struct ADU* adu) {
    printf("Handling Modbus Function 1: Read Coils\n");
}

void handle_read_discrete_inputs(struct ADU* adu) {
    printf("Handling Modbus Function 2: Read Discrete Inputs\n");
}

void handle_read_holding_registers(struct ADU* adu) {
    printf("Handling Modbus Function 3: Read Holding Registers\n");
}

void handle_read_input_registers(struct ADU* adu) {
    printf("Handling Modbus Function 4: Read Input Registers\n");
}

void handle_write_single_coil(struct ADU* adu) {
    printf("Handling Modbus Function 5: Write Single Coil\n");
}

void handle_write_single_register(struct ADU* adu) {
    printf("Handling Modbus Function 6: Write Single Register\n");
}

void handle_read_exception_status(struct ADU* adu) {
    printf("Handling Modbus Function 7: Read Exception Status\n");
}

void handle_diagnostic(struct ADU* adu) {
    printf("Handling Modbus Function 8: Diagnostic\n");
}

void handle_get_comm_event_counter(struct ADU* adu) {
    printf("Handling Modbus Function 11: Get Com Event Counter\n");
}

void handle_get_comm_event_log(struct ADU* adu) {
    printf("Handling Modbus Function 12: Get Com Event Log\n");
}

void handle_write_multiple_coils(struct ADU* adu) {
    printf("Handling Modbus Function 15: Write Multiple Coils\n");
}

void handle_write_multiple_registers(struct ADU* adu) {
    printf("Handling Modbus Function 16: Write Multiple Registers\n");
}

void handle_report_slave_id(struct ADU* adu) {
    printf("Handling Modbus Function 17: Report Slave ID\n");
}

void handle_read_file_record(struct ADU* adu) {
    printf("Handling Modbus Function 20: Read File Record\n");
}

void handle_write_file_record(struct ADU* adu) {
    printf("Handling Modbus Function 21: Write File Record\n");
}

void handle_mask_write_register(struct ADU* adu) {
    printf("Handling Modbus Function 22: Mask Write Register\n");
}

void handle_read_write_multiple_registers(struct ADU* adu) {
    printf("Handling Modbus Function 23: Read/Write Multiple Registers\n");
}

void handle_read_fifo_queue(struct ADU* adu) {
    printf("Handling Modbus Function 24: Read FIFO Queue\n");
}

void handle_encapsulated_interface_transport(struct ADU* adu) {
    printf("Handling Modbus Function 43: Encapsulated Interface Transport (Read Device Identification)\n");
}
