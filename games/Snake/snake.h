#ifndef GAME_SNAKE_H
#define GAME_SNAKE_H

#include "nx_internal.h"

int snake(NXEGWINDOW *hwnd);
void Snake_draw_squares(NXEGWINDOW *hwnd,int16_t x,int16_t y,int16_t size, int16_t color);

#endif