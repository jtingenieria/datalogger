#ifndef USB_SD_FS
#define USB_SD_FS

#include "stdbool.h"

void usb_sd_fs_init_card(void);

void usb_sd_fs_init_usb(void);

bool usb_sd_fs_is_storage_in_use_by_host(void);

#endif
