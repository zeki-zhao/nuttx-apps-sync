#ifndef GAME_SNAKE_H
#define GAME_SNAKE_H

#include "nx_internal.h"

int snake(NXHANDLE *hnx,struct nxhw_handle *hwnd,struct nxhw_handle *hwnd_score);
void Snake_draw_squares(NXEGWINDOW *hwnd,int16_t x,int16_t y,int16_t size, int16_t color);

#endif