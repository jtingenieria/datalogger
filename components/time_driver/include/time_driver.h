/**
 * @file time_driver.h
 * @author Juan Tinazzo (juantinazzoingenieria@gmail.com)
 * @brief
 * @version 0.1
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TIME_DRIVER_H
#define TIME_DRIVER_H

#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

bool time_driver_obtain_time();
void time_driver_initialize();
void time_driver_initialize_sntp(void);
void time_driver_initialize_from_sleep(void);

#endif
