/**
 * @file mcp_7940.c
 * @author Juan Tinazzo (juantinazzoingenieria@gmail.com)
 * @brief
 * @version 0.1
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"

#include "mcp_7940.h"

static const char TAG[] = "mcp_7940";

static const uint8_t SDA = 21;
static const uint8_t SCL = 22;

#define MCP_I2C_NUM I2C_NUM_0
#define I2C_MAX_TIMEOUT 5000
#define APB_I2C_MAX_TIMEOUT 80000000/I2C_MAX_TIMEOUT
#define MCP_ADDR 0B1101111

esp_err_t rtc_begin_i2c(){
	i2c_config_t conf = { };
	conf.mode = I2C_MODE_MASTER;
	conf.scl_io_num = (gpio_num_t)SCL;
	conf.sda_io_num = (gpio_num_t)SDA;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;

	esp_err_t ret = i2c_param_config((i2c_port_t)MCP_I2C_NUM, &conf);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"i2c_param_config failed");
		return ret;
	} else {
		ret = i2c_driver_install((i2c_port_t)MCP_I2C_NUM, conf.mode, 0, 0, 0);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG,"i2c_driver_install failed");
			return ret;
		} else {
			ret = i2c_set_timeout((i2c_port_t)MCP_I2C_NUM,APB_I2C_MAX_TIMEOUT);
			ESP_LOGI(TAG,"RTC initialized");

		}
	}
	vTaskDelay(pdMS_TO_TICKS(20));
	return ret;
}

esp_err_t rtc_set_time(const rtc_time_t *time)
{
	uint8_t addr_to_read = 0;
	uint8_t time_data[7];
	esp_err_t ret;

	ret = i2c_master_write_read_device(MCP_I2C_NUM, MCP_ADDR, &addr_to_read, 1, time_data, 7, APB_I2C_MAX_TIMEOUT);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"RTC set time failed while reading. Error: %d", (int)ret);
		return ret;
	}
	// Clear ST and EXTOSC during write operation, these were read above so will be reset to original values after the write
	rtc_set_bits(0x00, 0B10000000, 0);
	rtc_set_bits(0x07, 0B100, 0);

	// Wait for OSCRUN to go low
	while( rtc_get_byte(0x03) & 0B100000 )
	{
	}

	// Save flags but clear all date/time data
	time_data[0] = time_data[0] & 0B10000000;
	time_data[1] = time_data[1] & 0B10000000;
	time_data[2] = time_data[2] & 0B11000000;
	time_data[3] = time_data[3] & 0B11111000;
	time_data[4] = time_data[4] & 0B11000000;
	time_data[5] = time_data[5] & 0B11100000;
	time_data[6] = time_data[6] & 0B00000000;

	// Seconds
	time_data[0] |= ((time->secten) << 4) & 0B1110000;
	time_data[0] |= time->secone & 0B1111;

	// Minutes
	time_data[1] |= ((time->minten) << 4) & 0B1110000;
	time_data[1] |= time->minone & 0B1111;

	// Hours
	if( time->mode_24 )
	{
		time_data[2] |= ((time->hrten) << 4) & 0B110000;
		time_data[2] |= time->hrone & 0B1111;
	}
	else
	{
		time_data[2] |= time->pm << 5;
		time_data[2] |= ((time->hrten) << 4) & 0B10000;
		time_data[2] |= time->hrone & 0B1111;
	}

	// Week day
	time_data[3] |= time->wkday & 0B111;

	// Date
	time_data[4] |= ((time->dateten) << 4) & 0B110000;
	time_data[4] |= time->dateone & 0B1111;

	// Month
	time_data[5] |= ((time->mthten) << 4) & 0B10000;
	time_data[5] |= time->mthone & 0B1111;

	// Year
	time_data[6] |= (time->yrten << 4) & 0B11110000;
	time_data[6] |= time->yrone & 0B1111;

	// Write new data into chip
	uint8_t write_buf[8];
	write_buf[0]=0;
	for( int a = 0; a < 7; a++ )
	{
		write_buf[a+1]=time_data[a];
	}

	ret = i2c_master_write_to_device(MCP_I2C_NUM, MCP_ADDR, write_buf, sizeof(write_buf), APB_I2C_MAX_TIMEOUT);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"RTC set time failed while writing. Error: %d", (int)ret);
		return ret;
	}
	return ESP_OK;
}

#define ESP_SLAVE_ADDR CONFIG_I2C_SLAVE_ADDRESS /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

esp_err_t rtc_read_time(rtc_time_t *time)
{
	// Set read address to 0
	uint8_t addr_to_read = 0;
	uint8_t time_data[7]={};
	esp_err_t ret;

	ret=i2c_master_write_read_device(MCP_I2C_NUM, MCP_ADDR, &addr_to_read, 1, time_data, 7, APB_I2C_MAX_TIMEOUT);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"RTC read time failed. Error: %d", (int)ret);
		return ret;
	}
	time->secten = (time_data[0] & 0B1110000) >> 4;
	time->secone = time_data[0] & 0B1111;
	time->minten = (time_data[1] & 0B111000) >> 4;
	time->minone = time_data[1] & 0B1111;
	time->hrten = (time_data[2] & 0B110000) >> 4;
	time->hrone = time_data[2] & 0B1111;
	time->wkday = time_data[3] & 0B111;
	time->dateten = (time_data[4] & 0B110000) >> 4;
	time->dateone = time_data[4] & 0B1111;
	time->mthten = (time_data[5] & 0B10000) >> 4;
	time->mthone = time_data[5] & 0B1111;
	time->yrten = (time_data[6] & 0B11110000) >> 4;
	time->yrone = time_data[6] & 0B1111;

	/*ESP_LOGI(TAG, "Battery backup enabled?: %d",rtc_is_battery_enabled());
	ESP_LOGI(TAG, "Osc?: %d",rtc_get_osc());
	ESP_LOGI(TAG, "Ext osc?: %d",rtc_get_external_osc());*/

	return ESP_OK;
}

bool rtc_is_battery_enabled()
{
	return rtc_get_flag(0x03, 0B1000);
}

void rtc_set_battery_backup(bool enabled)
{
	rtc_set_bits(3, 0B00001000, enabled << 3);
}

void rtc_tm_2_rtc(const tm *from, rtc_time_t *to)
{
	// Seconds
	to->secten = (from->tm_sec / 10);
	to->secone = (from->tm_sec % 10);

	// Minutes
	to->minten = (from->tm_min / 10);
	to->minone = (from->tm_min % 10);

	// Hours (handle 24 hour conversion)
	if( to->mode_24 )
	{
		to->hrten = (from->tm_hour / 10);
		to->hrone = (from->tm_hour % 10);
		to->pm = false;
	}
	else
	{
		int hr = from->tm_hour;

		if( from->tm_hour >= 12 )
		{
			to->pm = true;
			hr -= 12;
		}
		else
		{
			to->pm = false;
		}

		to->hrten = (hr / 10);
		to->hrone = (hr % 10);
	}

	// Weekday
	to->wkday = (from->tm_wday);

	// Date
	to->dateten = (from->tm_mday / 10);
	to->dateone = (from->tm_mday % 10);

	// Month - Stored 0-index in tm but 1-index in rtcc
	to->mthten = ((from->tm_mon + 1) / 10);
	to->mthone = ((from->tm_mon + 1) % 10);

	// Year - Stored years since 1900 in tm but years since 2000 in rtcc
	to->yrten = ((from->tm_year - 100) / 10);
	to->yrone = ((from->tm_year - 100) % 10);
}

void rtc_rtc_2_tm(const rtc_time_t *from, tm *to)
{
	// Seconds
	to->tm_sec = (from->secten * 10) + from->secone;

	// Minutes
	to->tm_min = (from->minten * 10) + from->minone;

	// Hours - Handle 24 hour conversion
	if( from->mode_24 )
	{
		to->tm_hour = (from->hrten * 10) + from->hrone;
	}
	else
	{
		to->tm_hour = (from->hrten * 10) + from->hrone;
		to->tm_hour += from->pm ? 12 : 0;
	}

	// Weekday
	to->tm_wday = (from->wkday);

	// Date
	to->tm_mday = (from->dateten * 10) + from->dateone;

	// Month - Stored 0-index in tm but 1-index in rtcc
	to->tm_mon = ((from->mthten * 10) + from->mthone) - 1;

	// Year - Stored years since 1900 in tm but years since 2000 in rtcc
	to->tm_year = ((from->yrten * 10) + from->yrten) + 100;
}

void rtc_set_bits(uint8_t address, uint8_t mask, uint8_t values)
{

	uint8_t data;
	esp_err_t ret;
	ret = i2c_master_write_read_device(MCP_I2C_NUM, MCP_ADDR, &address, 1, &data, 1,APB_I2C_MAX_TIMEOUT);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"RTC set bits failed while reading. Error: %d", (int)ret);
	}
	//ESP_LOGI(TAG,"Reading %x in address %x, return %d",values,address,i2c_master_write_read_device(MCP_I2C_NUM, MCP_ADDR, &address, 1, &data, 1,APB_I2C_MAX_TIMEOUT));

	// Clear out the existing bits according to mask
	data &= ~mask;

	// Set bits according to values
	data |= (values & mask);

	uint8_t write_buf[2]={(uint8_t)address, data};
	ret = i2c_master_write_to_device(MCP_I2C_NUM, MCP_ADDR, write_buf, sizeof(write_buf), APB_I2C_MAX_TIMEOUT);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"RTC set bits failed while writing. Error: %d", (int)ret);
	}
	//ESP_LOGI(TAG,"Setting %x in address %x, return %d",values,address,i2c_master_write_to_device(MCP_I2C_NUM, MCP_ADDR, write_buf, sizeof(write_buf), APB_I2C_MAX_TIMEOUT));
}

uint8_t rtc_get_byte(uint8_t address)
{
	// Set read address to 0
	uint8_t data;
	esp_err_t ret;
	ret = i2c_master_write_read_device(MCP_I2C_NUM, MCP_ADDR, &address, 1, &data, 1,APB_I2C_MAX_TIMEOUT);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"RTC read bits failed. Error: %d", (int)ret);
	}
	return data;
}

bool rtc_get_flag(uint8_t address, uint8_t mask)
{
	uint8_t data = rtc_get_byte(address);
	return data & mask;
}

void rtc_set_osc(bool enabled)
{
	rtc_set_bits(0, 0B10000000, enabled << 7);
}

void rtc_set_external_osc(bool enabled)
{
	rtc_set_bits(0x07, 0B1000, enabled << 3);
}

bool rtc_get_external_osc()
{
	return rtc_get_flag(0x07, 0B1000);
}

bool rtc_get_osc()
{
	return rtc_get_flag(0, 0B10000000);
}
