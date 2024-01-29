idf_component_register(SRCS "graphic_driver.c" "lv_app_helper_functions.c" "usb_icon.c"
					"lv_roller_file_explorer.c" "recording_icon.c" "stopped_recording_icon.c"
					"intro_logo.c"
                    INCLUDE_DIRS "include"
                    REQUIRES log lvgl driver lvgl_esp32_drivers)

