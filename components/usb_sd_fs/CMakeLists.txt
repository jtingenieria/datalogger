idf_component_register(SRCS "usb_sd_fs.c"
                    INCLUDE_DIRS "include"
                    REQUIRES fatfs vfs espressif__esp_tinyusb)

