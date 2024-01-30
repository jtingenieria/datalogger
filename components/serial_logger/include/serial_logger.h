#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


typedef struct
{
	int float_data_qty;
	int received_data_qty;
	int id;
	int size_of_packet;
	float float_data_p[];
} serial_packet_t;

int serial_logger_init(int float_data_expected, int gpio_serial_port);

void serial_logger_get_data(TickType_t xTicksToWait, serial_packet_t ** packet);

#endif
