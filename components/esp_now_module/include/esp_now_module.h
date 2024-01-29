#ifndef ESP_NOW_MODULE
#define ESP_NOW_MODULE

void esp_now_module_init();

void esp_now_module_set_success_cb(void (*success_cb)());

void esp_now_module_set_fail_cb(void (*fail_cb)());

void esp_now_module_send(void * data, uint32_t size);

void esp_now_module_init_after_wifi();

#endif
