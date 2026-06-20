#ifndef UI_TERMINAL_H
#define UI_TERMINAL_H

#include "lvgl/lvgl.h"

extern lv_obj_t *ui_Terminal;
extern lv_obj_t *ui_TerminalOutput;
extern lv_obj_t *ui_TerminalInput;
extern lv_obj_t *ui_TerminalKeyboard;

void ui_Terminal_screen_init(void);
void ui_Terminal_homebutton_create(lv_obj_t *parent);

#endif
