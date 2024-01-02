set(srcs "main.c")
set(requires fatfs console vfs graphic_driver lvgl 
                       lvgl_esp32_drivers)

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS .
    REQUIRES "${requires}"
)
