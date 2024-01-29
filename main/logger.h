/*
 * logger.h
 *
 *  Created on: 3 ene. 2024
 *      Author: juant
 */

#ifndef MAIN_LOGGER_H_
#define MAIN_LOGGER_H_

#include "stdbool.h"

void logger_init(void);

void logger_log(bool block_caller_task);


#endif /* MAIN_LOGGER_H_ */
