#ifndef USB_SD_FS
#define USB_SD_FS

#include "stdbool.h"
#include "esp_err.h"

typedef enum
{
    USB_SD_FS_OK,
    USB_SD_FS_MOUNT_FAIL,
    USB_SD_FS_OPEN_FAILED,
    USB_SD_FS_OTHER_ERR,
	USB_SD_FS_MALLOC_FAILED,
} usb_sd_fs_err_t;


void usb_sd_fs_init_card(void);

void usb_sd_fs_init_usb(void);

void usb_sd_fs_deinit_usb(void);

bool usb_sd_fs_is_storage_in_use_by_host(void);

void usb_sd_fs_set_file(char *file);

usb_sd_fs_err_t usb_sd_fs_write(char *data);

esp_err_t usb_sd_fs_ota_start(void);

esp_err_t usb_sd_fs_ota_wait_for_result(void);

char * usb_sd_fs_get_full_filename(void);

bool usb_sd_fs_is_filename_set(void);

usb_sd_fs_err_t usb_sd_fs_read_file(char * file_name, char **data_out, long * read_size);

#endif
