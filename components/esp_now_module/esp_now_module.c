#include <stdio.h>
#include "esp_now_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_now.h"
//#include "espnow.h"
#include "esp_log.h"
#include "string.h"

static const char * TAG = "esp_now_module";

#define WIFI_CHANNEL 6

static void (*_success_cb)(void);
static void (*_fail_cb)(void);

uint8_t broadcastAddress[] = {0x8C, 0x4B, 0x14, 0xE7, 0xB0 , 0xFC};

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
   ESP_LOGI(TAG, "Delivery %s",status == ESP_NOW_SEND_SUCCESS ? "success" : "fail");
   if(status == ESP_NOW_SEND_SUCCESS)
   {
       if (_success_cb != NULL) (*_success_cb)();
   }
   else
   {
       if(_fail_cb != NULL) (*_fail_cb)();
   }
}

static void esp_now_wifi_init()
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}


void esp_now_module_init()
{
    //espnow_storage_init();
    esp_now_wifi_init();
    int chan = WIFI_CHANNEL;
    ESP_ERROR_CHECK(esp_wifi_set_channel(chan,WIFI_SECOND_CHAN_NONE));
    esp_now_init();
    esp_now_register_send_cb(OnDataSent);
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = chan;
    peerInfo.encrypt = false;
    esp_err_t result = esp_now_add_peer(&peerInfo);
    ESP_LOGI(TAG, "Result from add peer is %d", result);
}

void esp_now_module_init_after_wifi()
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_now_module_init();

}

void esp_now_module_set_success_cb(void (*success_cb)())
{
    _success_cb = success_cb;
    return;
}

void esp_now_module_set_fail_cb(void (*fail_cb)())
{
    _fail_cb = fail_cb;
    return;
}

void esp_now_module_send(void * data, uint32_t size)
{
    esp_err_t result;
    result = esp_now_send(broadcastAddress, (uint8_t *) data, size);
    //ESP_LOGI(TAG, "Result from ESP_NOW send is %d", result);
}
