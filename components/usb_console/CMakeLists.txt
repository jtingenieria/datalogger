idf_component_register(SRCS "usb_console.c"
                    INCLUDE_DIRS "include"
                    REQUIRES fatfs console vfs console espressif__esp_tinyusb)

