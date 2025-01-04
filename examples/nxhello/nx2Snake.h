#ifndef __APPS_EXAMPLES_NXHELLO_NX2SNAKE_H
#define __APPS_EXAMPLES_NXHELLO_NX2SNAKE_H

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/boardctl.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nxfonts.h>


void nxhello_hello(NXWINDOW hwnd);

#endif /* __APPS_EXAMPLES_NXHELLO_NXHELLO_H */