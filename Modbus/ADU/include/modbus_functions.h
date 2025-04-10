#ifndef MODBUS_FUNCTIONS_H
#define MODBUS_FUNCTIONS_H

#include "ADU.h"

// Główna funkcja dispatcher
void handle_modbus_function(struct ADU* adu);

// Deklaracje wszystkich funkcji Modbus (1–43)
void handle_read_coils(struct ADU* adu);                          // 1
void handle_read_discrete_inputs(struct ADU* adu);               // 2
void handle_read_holding_registers(struct ADU* adu);             // 3
void handle_read_input_registers(struct ADU* adu);               // 4
void handle_write_single_coil(struct ADU* adu);                  // 5
void handle_write_single_register(struct ADU* adu);              // 6
void handle_read_exception_status(struct ADU* adu);              // 7
void handle_diagnostic(struct ADU* adu);                          // 8
void handle_get_comm_event_counter(struct ADU* adu);             // 11
void handle_get_comm_event_log(struct ADU* adu);                 // 12
void handle_write_multiple_coils(struct ADU* adu);               // 15
void handle_write_multiple_registers(struct ADU* adu);           // 16
void handle_report_slave_id(struct ADU* adu);                    // 17
void handle_read_file_record(struct ADU* adu);                   // 20
void handle_write_file_record(struct ADU* adu);                  // 21
void handle_mask_write_register(struct ADU* adu);                // 22
void handle_read_write_multiple_registers(struct ADU* adu);      // 23
void handle_read_fifo_queue(struct ADU* adu);                    // 24
void handle_encapsulated_interface_transport(struct ADU* adu);   // 43

#endif // MODBUS_FUNCTIONS_H
