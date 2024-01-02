/**
 * @file mcp_7940.h
 * @author Juan Tinazzo (juantinazzoingenieria@gmail.com)
 * @brief
 * @version 0.1
 * @copyright Copyright (c) 2023
 *
 */

/** \addtogroup mcp_7940
 *  @{
 */

#ifndef MCP_7940
#define MCP_7940

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct tm tm;

/**
 * @brief Structure to use with RTC times.
 */
typedef struct
{
	uint8_t secten;
	uint8_t secone;
	uint8_t minten;
	uint8_t minone;
	uint8_t hrten;
	uint8_t hrone;
	uint8_t wkday;
	uint8_t dateten;
	uint8_t dateone;
	uint8_t mthten;
	uint8_t mthone;
	uint8_t yrten;
	uint8_t yrone;
	bool mode_24;
	bool pm;
}rtc_time_t;

/**
 * Initialize communication with RTC via I2C
 * @return ESP_OK if init OK.
 */
esp_err_t rtc_begin_i2c();

/**
 * @brief Sets the time of the RTC
 * @param time
 * @return ESP_OK if OK.
 */
esp_err_t rtc_set_time(const rtc_time_t *time);

/**
 * @brief Reads the time from the RTC
 * @param time
 * @return ESP_OK if OK.
 */
esp_err_t rtc_read_time(rtc_time_t *time);

/**
 * @brief Check of the battery of the RTC is enabled.
 * @return true if enabled.
 */
bool rtc_is_battery_enabled();

/**
 * @brief Sets the RTC backup battery.
 * @param enabled set to true to enable.
 */
void rtc_set_battery_backup(bool enabled);

/**
 * @brief Convert between tm format and rtc_time_t format.
 * @param from time in tm format.
 * @param to time in rtc_time_t format.
 */
void rtc_tm_2_rtc(const tm *from, rtc_time_t *to);

/**
 * @brief Convert between rtc_time_t format and tm format.
 * @param from time in rtc_time_t format.
 * @param to time in tm format.
 */
void rtc_rtc_2_tm(const rtc_time_t *from, tm *to);

/**
 * @brief Enables the RTC oscillator
 * @param enabled set to true to enable.
 */
void rtc_set_osc(bool enabled);

/**
 * @brief Enables the external RTC oscillator.
 * @param enabled set to true to enable.
 */
void rtc_set_external_osc(bool enabled);

/**
 * @brief Set a number of bits in a specified register in the RTC.
 * @param address Address of the register to set.
 * @param mask	Mask of the bits to be set.
 * @param values Values of the bits that are going to be written.
 */

void rtc_set_bits(uint8_t address, uint8_t mask, uint8_t values);
/**
 * @brief Reads a byte from the RTC at the specified address.
 * @param address Address to be read.
 * @return The read byte.
 */
uint8_t rtc_get_byte(uint8_t address);

/**
 * @brief Gets a bit from a specified register of the RTC.
 * @param address Address of the register.
 * @param mask Mask of the bit to read (1<< bit_number).
 * @return true if set, false if clear.
 */
bool rtc_get_flag(uint8_t address, uint8_t mask);

/**
 * @brief Get value of external oscillator enabled bit.
 * @return true if set.
 */
bool rtc_get_external_osc();

/**
 * @brief Get value of oscillator enabled bit.
 * @return true if set.
 */
bool rtc_get_osc();
#endif

/** @}*/

