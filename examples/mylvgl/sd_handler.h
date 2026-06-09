#ifndef __SD_HANDLER_H
#define __SD_HANDLER_H

#include "lvgl_event.h"

/**
 * @brief Save-text event handler
 * register with lvgl_evt_register()
 * @param msg
 */
void save_text_handler(const struct lvgl_msg_s *msg);
void save_modbus_slave_show_config_hander(const struct lvgl_msg_s *msg);
void move_sd_firmware_to_flash(const struct lvgl_msg_s *msg);

/**
 * @brief Load saved text from SD card
 * @return malloc'd string, caller must free. NULL if file not found or error.
 */
char *load_text_from_sd(void);

#endif /* __SD_HANDLER_H */
