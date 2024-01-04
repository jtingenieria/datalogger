set(srcs "main.c" "logger.c")
set(requires vfs graphic_driver lvgl 
                       lvgl_esp32_drivers usb_console usb_sd_fs)

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS .
    REQUIRES "${requires}"
)
