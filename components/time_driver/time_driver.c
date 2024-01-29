/**
 * @file time_driver.c
 * @author Juan Tinazzo (juantinazzoingenieria@gmail.com)
 * @brief
 * @version 0.1
 * @copyright Copyright (c) 2023
 *
 */

#include "time_driver.h"

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "mcp_7940.h"


static const char *TAG = "time_driver";

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

time_t now = 0;
struct tm timeinfo = { 0 };
int retry = 0;
const int retry_count = 10;

time_t now;

void time_sync_notification_cb(struct timeval *tv)
{
    char strftime_buf[64];
    ESP_LOGD(TAG, "Notification of a time synchronization event");
    settimeofday(tv, NULL);
    time_t now = (time_t) tv->tv_sec;
    time(&now);
    rtc_time_t rtc_time;
    struct tm timeinfo = { 0 };
    localtime_r(&now, &timeinfo);
    rtc_tm_2_rtc(&timeinfo, &rtc_time);
    rtc_set_time(&rtc_time);
    rtc_read_time(&rtc_time);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Sync: %s", strftime_buf);
}

bool time_driver_obtain_time()
{
    time_t now;
    struct tm timeinfo = { 0 };
    rtc_time_t rtc_time;
    char strftime_buf[64];
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900))
    {
    	rtc_read_time(&rtc_time);
    	rtc_rtc_2_tm(&rtc_time,&timeinfo);
    	now = mktime(&timeinfo);
    	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    	ESP_LOGI(TAG, "RTC time: %s", strftime_buf);
    	struct timeval tval={};
    	tval.tv_sec=now;
    	settimeofday(&tval,NULL);
    	time(&now);
    	return false;
    }
    return true;
}

void time_driver_initialize()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

	rtc_begin_i2c();
	rtc_set_external_osc(false);
	rtc_set_osc(true);
	rtc_set_battery_backup(true);
	time_driver_initialize_sntp();
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    setenv("TZ", CONFIG_SNTP_TIME_ZONE, 1);
    tzset();
    time_driver_obtain_time();
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGD(TAG, "Init with SNTP: %s", strftime_buf);
}

void time_driver_initialize_from_sleep()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    rtc_begin_i2c();
    rtc_set_external_osc(false);
    rtc_set_osc(true);
    rtc_set_battery_backup(true);
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    setenv("TZ", CONFIG_SNTP_TIME_ZONE, 1);
    tzset();
    time_driver_obtain_time();
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
}


void time_driver_initialize_sntp(void)
{
    ESP_LOGD(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);

/*
 * If 'NTP over DHCP' is enabled, we set dynamic pool address
 * as a 'secondary' server. It will act as a fallback server in case that address
 * provided via NTP over DHCP is not accessible
 */
#if LWIP_DHCP_GET_NTP_SRV && SNTP_MAX_SERVERS > 1
    esp_sntp_setservername(1, "pool.ntp.org");

#if LWIP_IPV6 && SNTP_MAX_SERVERS > 2          // statically assigned IPv6 address is also possible
    ip_addr_t ip6;
    if (ipaddr_aton("2a01:3f7::1", &ip6)) {    // ipv6 ntp source "ntp.netnod.se"
        esp_sntp_setserver(2, &ip6);
    }
#endif  /* LWIP_IPV6 */

#else   /* LWIP_DHCP_GET_NTP_SRV && (SNTP_MAX_SERVERS > 1) */
    // otherwise, use DNS address from a pool
    esp_sntp_setservername(0, CONFIG_SNTP_TIME_SERVER);

    esp_sntp_setservername(1, "pool.ntp.org");     // set the secondary NTP server (will be used only if SNTP_MAX_SERVERS > 1)
#endif

    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    esp_sntp_init();

//    ESP_LOGI(TAG, "List of configured NTP servers:");

//    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i)
//    {
//        if (esp_sntp_getservername(i))
//        {
//            ESP_LOGI(TAG, "server %d: %s", i, esp_sntp_getservername(i));
//        }
//        else
//        {
//            // we have either IPv4 or IPv6 address, let's print it
//            char buff[INET6_ADDRSTRLEN];
//            ip_addr_t const *ip = esp_sntp_getservername(i);
//            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)   ESP_LOGI(TAG, "server %d: %s", i, buff);
//        }
//    }
}
