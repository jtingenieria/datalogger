set(srcs "main.c" "logger.c" "wifi_cx.c" "wifi_pwd.c")
set(requires vfs
			graphic_driver
			lvgl 
		    lvgl_esp32_drivers
	        usb_console
	        usb_sd_fs
	        ds18b20_manager
	        time_driver
	        esp_event
	        esp_wifi
	        lwip
	        nvs_flash
	        serial_logger)

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS .
    REQUIRES "${requires}"
)
