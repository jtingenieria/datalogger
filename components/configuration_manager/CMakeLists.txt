idf_component_register(SRCS "configuration_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES json log ds18b20_manager usb_sd_fs)

