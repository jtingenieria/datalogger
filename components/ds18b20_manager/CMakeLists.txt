idf_component_register(SRCS "ds18b20_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES espressif__ds18b20 log usb_sd_fs)

