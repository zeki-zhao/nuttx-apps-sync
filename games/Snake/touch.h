#ifndef __APPS_GAME_SNAKE_TOUCH_H
#define __APPS_GAME_SNAKE_TOUCH_H

#include <nuttx/input/touchscreen.h>
#include <pthread.h>

extern struct touch_sample_s Snake_Touch;
extern pthread_t Touch_t1;    //创建线程变量t1
extern pthread_t Snake_t1;    //创建线程变量t1

extern int mytouch_fd;
extern struct pollfd touch_fds;
extern sem_t stDirChangeSemevent;

int touch(void);
void* touch_detect(void *arg);


#endif