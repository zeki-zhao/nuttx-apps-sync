/**
*   @file   snake.c
***********************************************************************************************************************/
#include <fixedmath.h>
#ifdef __cplusplus
extern "C"{
#endif


/***********************************************************************************************************************
*                                                     INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
***********************************************************************************************************************/
#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <debug.h>


#include <sys/types.h>
#include <sys/boardctl.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxtk.h>
#include <nuttx/nx/nxfonts.h>
#include <nuttx/nx/nxglib.h>

#include "nx_internal.h"

#include "touch.h"
#include "snake_logic.h"


/* touch.c  */
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
// #include <nuttx/analog/ioctl.h>
#include <syslog.h>

#include <sys/ioctl.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/input/gt9xx.h>
#include <nuttx/../sys/poll.h>

#include <nuttx/input/touchscreen.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <unistd.h>

#include "snake.h"

#ifdef CONFIG_GAMES_SNAKES
/*add user code*/


/***********************************************************************************************************************
*                                            SOURCE FILE VERSION INFORMATION
***********************************************************************************************************************/
#define SNAKE_SW_MAJOR_VERSION_C             0
#define SNAKE_SW_MINOR_VERSION_C             0
#define SNAKE_SW_PATCH_VERSION_C             0

/***********************************************************************************************************************
*                                                  FILE VERSION CHECKS
***********************************************************************************************************************/
#if ((SNAKE_SW_MAJOR_VERSION_C != SNAKE_SW_MAJOR_VERSION) || \
     (SNAKE_SW_MINOR_VERSION_C != SNAKE_SW_MINOR_VERSION) || \
     (SNAKE_SW_PATCH_VERSION_C != SNAKE_SW_PATCH_VERSION))
#error "Software Version Numbers of snake.c and snake.h are different"
#endif


/***********************************************************************************************************************
*                                                   DEFINES AND MACROS
***********************************************************************************************************************/
/* Configuration ************************************************************/
/* If not specified, assume that the hardware supports one video plane */

#ifndef CONFIG_EXAMPLES_NX_VPLANE
#  define CONFIG_EXAMPLES_NX_VPLANE 0
#endif

/* If not specified, assume that the hardware supports one LCD device */

#ifndef CONFIG_EXAMPLES_NX_DEVNO
#  define CONFIG_EXAMPLES_NX_DEVNO 0
#endif

/***********************************************************************************************************************
*                                                        ENUMS
***********************************************************************************************************************/


/***********************************************************************************************************************
*                                              STRUCTURES AND OTHER TYPEDEFS
***********************************************************************************************************************/


/***********************************************************************************************************************
*                                              STATIC VARIABLE DECLARATIONS
***********************************************************************************************************************/
static int g_exitcode = NXEXIT_SUCCESS;

/***********************************************************************************************************************
*                                              GLOBAL VARIABLE DECLARATIONS
***********************************************************************************************************************/
struct pollfd touch_fds;
pthread_t Snake_t1;               //创建线程变量t1
pthread_t Snake_t2;               //创建线程变量t2
pthread_t Touch_t1;               //创建线程变量t1

NXHANDLE g_snake_fonthandle = NULL; //字体句柄
NXHANDLE g_snake_hnx = NULL; //窗口句柄
nxgl_coord_t g_snake_xres;
nxgl_coord_t g_snake_yres;

/* The screen resolution */

nxgl_coord_t g_snake_xres;
nxgl_coord_t g_snake_yres;

bool b_snake_haveresolution = false;
bool g_snake_connected = false;
sem_t g_snake_semevent = {0};

/* Colors used to fill window 1 & 2 */

nxgl_mxpixel_t g_snake_color1[CONFIG_NX_NPLANES];
nxgl_mxpixel_t g_snake_color2[CONFIG_NX_NPLANES];
#ifndef CONFIG_EXAMPLES_NX_RAWWINDOWS
nxgl_mxpixel_t g_snake_tbcolor[CONFIG_NX_NPLANES];
#endif

int mytouch_fd;/* 触控设备文件号 */

/***********************************************************************************************************************
*                                               STATIC FUNCTION PROTOTYPES
***********************************************************************************************************************/


/***********************************************************************************************************************
*                                              STATIC FUNCTION DEFINITIONS
***********************************************************************************************************************/
static void nxeg_draw_exit_button(NXEGWINDOW hwnd)
{
    int ret;
    struct nxgl_rect_s TempRect;
    TempRect.pt1.x = 780;
    TempRect.pt1.y = 0;
    TempRect.pt2.x = 796;
    TempRect.pt2.y = 19;
    nxgl_mxpixel_t nxgl_color[CONFIG_NX_NPLANES] = {0x0100};
    ret = nx_fill(hwnd, &TempRect, nxgl_color);
    if (ret < 0)
    {
        printf("nx_main: nx_fill failed: %d\n", errno);
    }
}

#ifdef CONFIG_NX_XYINPUT
static void nxeg_drivemouse(void)
{
    nxgl_coord_t x;
    nxgl_coord_t y;
    nxgl_coord_t xstep = g_snake_xres / 8;
    nxgl_coord_t ystep = g_snake_yres / 8;

    for (x = 0; x < g_snake_xres; x += xstep)
    {
        for (y = 0; y < g_snake_yres; y += ystep)
        {
            printf("nxeg_drivemouse: Mouse left button at (%d,%d)\n", x, y);
            nx_mousein(g_snake_hnx, x, y, NX_MOUSE_LEFTBUTTON);
        }
    }
}
#endif

/**
 * @brief nxeg_initstate
 * 
 * @param st 
 * @param wnum 
 * @param color 
 */
static void nxeg_initstate(FAR struct nxeg_state_s *st, int wnum,
                           nxgl_mxpixel_t color)
{
#ifdef CONFIG_NX_KBD
    FAR const struct nx_font_s *fontset;
#endif

    /* Initialize the window number (used for debug output only) and color
    * (used for redrawing the window)
    */

    st->wnum     = wnum;
    st->color[0] = color;

    /* Get information about the font set being used and save this in the
    * state structure
    */

#ifdef CONFIG_NX_KBD
    fontset      = nxf_getfontset(g_snake_fonthandle);
    st->nchars   = 0;
    st->nglyphs  = 0;
    st->height   = fontset->mxheight;
    st->width    = fontset->mxwidth;
    st->spwidth  = fontset->spwidth;
#endif
}


#ifndef CONFIG_EXAMPLES_NX_RAWWINDOWS
/**
 * @brief nxeg_freestate
 * 
 * @param st 
 */
static void nxeg_freestate(FAR struct nxeg_state_s *st)
{
#ifdef CONFIG_NX_KBD
    int i;

    if (st){
        for (i = 0; i < st->nglyphs; i++){
            if (st->glyph[i].bitmap){
                free(st->glyph[i].bitmap);
            }
            st->glyph[i].bitmap = NULL;
        }
        st->nchars = 0;
    }
#endif
}
#endif


#ifdef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline NXEGWINDOW nxeg_openwindow(FAR const struct nx_callback_s *cb,
                                       FAR struct nxeg_state_s *state)
{
    NXEGWINDOW hwnd;

    hwnd = nx_openwindow(g_snake_hnx, 0, cb, (FAR void *)state);
    if (!hwnd)
    {
        printf("nxeg_openwindow: nx_openwindow failed: %d\n", errno);
        g_exitcode = NXEXIT_NXOPENWINDOW;
    }
  return hwnd;
}
#else
static inline NXEGWINDOW nxeg_openwindow(FAR const struct nx_callback_s *cb,
                                         FAR struct nxeg_state_s *state)
{
  NXEGWINDOW hwnd;

    hwnd = nxtk_openwindow(g_snake_hnx, 0, cb, (FAR void *)state);
    if (!hwnd)
    {
        printf("nxeg_openwindow: nxtk_openwindow failed: %d\n", errno);
        g_exitcode = NXEXIT_NXOPENWINDOW;
    }
  return hwnd;
}

// static inline NXEGWINDOW nxeg_openwindow_noinput(FAR struct nxeg_state_s *state)
// {
//   NXEGWINDOW hwnd;
//   printf("in func %s:%d\n",__func__,__LINE__);
//   hwnd = nxtk_openwindow(g_snake_hnx, 0, &g_snake_nxcb, (FAR void *)state);
//   if (!hwnd)
//     {
//       printf("nxeg_openwindow: nxtk_openwindow failed: %d\n", errno);
//       g_exitcode = NXEXIT_NXOPENWINDOW;
//     }
//   return hwnd;
// }
#endif

#ifdef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline int nxeg_closewindow(NXEGWINDOW hwnd, FAR struct nxeg_state_s *state)
{
    int ret = nx_closewindow(hwnd);
    if (ret < 0)
    {
        printf("nxeg_closewindow: nx_closewindow failed: %d\n", errno);
        g_exitcode = NXEXIT_NXCLOSEWINDOW;
    }
  return ret;
}
#else
static inline int nxeg_closewindow(NXEGWINDOW hwnd, FAR struct nxeg_state_s *state)
{
    int ret = nxtk_closewindow(hwnd);
    if (ret < 0)
    {
        printf("nxeg_closewindow: nxtk_closewindow failed: %d\n", errno);
        g_exitcode = NXEXIT_NXCLOSEWINDOW;
    }
    nxeg_freestate(state);
    return ret;
}
#endif

#ifdef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline int nxeg_setsize(NXEGWINDOW hwnd, FAR struct nxgl_size_s *size)
{
    int ret = nx_setsize(hwnd, size);
    if (ret < 0)
    {
        printf("nxeg_setsize: nx_setsize failed: %d\n", errno);
        g_exitcode = NXEXIT_NXSETSIZE;
    }
  return ret;
}
#else
static inline int nxeg_setsize(NXEGWINDOW hwnd, FAR struct nxgl_size_s *size)
{
    int ret = nxtk_setsize(hwnd, size);
    if (ret < 0)
    {
        printf("nxeg_setsize: nxtk_setsize failed: %d\n", errno);
        g_exitcode = NXEXIT_NXSETSIZE;
    }
  return ret;
}
#endif

#ifdef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline int nxeg_setposition(NXEGWINDOW hwnd, FAR struct nxgl_point_s *pos)
{
    int ret = nx_setposition(hwnd, pos);
    if (ret < 0)
    {
        printf("nxeg_setposition: nx_setposition failed: %d\n", errno);
        g_exitcode = NXEXIT_NXSETPOSITION;
    }
  return ret;
}
#else
static inline int nxeg_setposition(NXEGWINDOW hwnd, FAR struct nxgl_point_s *pos)
{
    int ret = nxtk_setposition(hwnd, pos);
    if (ret < 0)
    {
        printf("nxeg_setposition: nxtk_setposition failed: %d\n", errno);
        g_exitcode = NXEXIT_NXSETPOSITION;
    }
    return ret;
}
#endif

#ifndef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline int nxeq_opentoolbar(NXEGWINDOW hwnd, nxgl_coord_t height,
                                   FAR const struct nx_callback_s *cb,
                                   FAR struct nxeg_state_s *state)
{
    int ret;
    ret = nxtk_opentoolbar(hwnd, height, cb, (FAR void *)state);
    if (ret < 0)
    {
        printf("nxeq_opentoolbar: nxtk_opentoolbar failed: %d\n", errno);
        g_exitcode = NXEXIT_NXOPENTOOLBAR;
    }
    return ret;
}
#endif

#ifdef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline int nxeg_lower(NXEGWINDOW hwnd)
{
    int ret = nx_lower(hwnd);
    if (ret < 0)
    {
        printf("nxeg_lower: nx_lower failed: %d\n", errno);
        g_exitcode = NXEXIT_NXLOWER;
    }
  return ret;
}
#else
static inline int nxeg_lower(NXEGWINDOW hwnd)
{
    int ret = nxtk_lower(hwnd);
    if (ret < 0)
    {
        printf("nxeg_lower: nxtk_lower failed: %d\n", errno);
        g_exitcode = NXEXIT_NXLOWER;
    }
    return ret;
}
#endif

#ifdef CONFIG_EXAMPLES_NX_RAWWINDOWS
static inline int nxeg_raise(NXEGWINDOW hwnd)
{
  int ret = nx_raise(hwnd);
  if (ret < 0)
    {
      printf("nxeg_raise: nx_raise failed: %d\n", errno);
      g_exitcode = NXEXIT_NXRAISE;
    }
  return ret;
}
#else
static inline int nxeg_raise(NXEGWINDOW hwnd)
{
    int ret = nxtk_raise(hwnd);
    if (ret < 0)
    {
        printf("nxeg_raise: nxtk_raise failed: %d\n", errno);
        g_exitcode = NXEXIT_NXRAISE;
    }
    return ret;
}
#endif

static int nxeg_initialize(NXHANDLE* pNxhandle)
{
    struct sched_param param;
    pthread_t thread;
    int ret;
    int i;

  /* Initialize window colors */

    for (i = 0; i < CONFIG_NX_NPLANES; i++){
        g_snake_color1[i]  = CONFIG_EXAMPLES_NX_COLOR1;
        g_snake_color2[i]  = CONFIG_EXAMPLES_NX_COLOR2;
#ifndef CONFIG_EXAMPLES_NX_RAWWINDOWS
        g_snake_tbcolor[i] = CONFIG_EXAMPLES_NX_TBCOLOR;
#endif
    }

    /* Set the client task priority */
    param.sched_priority = CONFIG_EXAMPLES_NX_CLIENTPRIO;
    ret = sched_setparam(0, &param);
    if (ret < 0){
        printf("nxeg_initialize: sched_setparam failed: %d\n" , ret);
        g_exitcode = NXEXIT_SCHEDSETPARAM;
        return ERROR;
    }

    /* Start the NX server kernel thread */
    ret = boardctl(BOARDIOC_NX_START, 0);
    if (ret < 0){
        printf("nxeg_initialize: Failed to start the NX server: %d\n", errno);
        g_exitcode = NXEXIT_TASKCREATE;
        return ERROR;
    }

    /* Connect to the server */
    *pNxhandle = nx_connect();
    if(*pNxhandle){
        pthread_attr_t attr;

#ifdef CONFIG_VNCSERVER
        /* Setup the VNC server to support keyboard/mouse inputs */

        struct boardioc_vncstart_s vnc =
        {
            0, *pNxhandle
        };

        ret = boardctl(BOARDIOC_VNC_START, (uintptr_t)&vnc);
        if (ret < 0)
            {
            printf("boardctl(BOARDIOC_VNC_START) failed: %d\n", ret);
            nx_disconnect(*pNxhandle);
            g_exitcode = NXEXIT_FBINITIALIZE;
            return ERROR;
            }
#endif

       /* Start a separate thread to listen for server events.  This is probably
        * the least efficient way to do this, but it makes this example flow more
        * smoothly.
        */

        pthread_attr_init(&attr);
        param.sched_priority = CONFIG_GAMES_SNAKES_LISTENERPRIO;
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setstacksize(&attr, CONFIG_GAMES_SNAKES_STACKSIZE);

        ret = pthread_create(&thread, &attr, nx_snake_listenerthread, pNxhandle);
        if (ret != 0){
            printf("nxeg_initialize: pthread_create failed: %d\n", ret);
            g_exitcode = NXEXIT_PTHREADCREATE;
            return ERROR;
        }

       /* Don't return until we are connected to the server */

        while (!g_snake_connected){
            /* Wait for the listener thread to wake us up when we really are connected.*/
            sem_wait(&g_snake_semevent);
        }
    }else{
        printf("nxeg_initialize: nx_connect failed: %d\n", errno);
        g_exitcode = NXEXIT_NXCONNECT;
        return ERROR;
    }

  return OK;
}

/***********************************************************************************************************************
*                                              GLOBAL FUNCTION DEFINITIONS
***********************************************************************************************************************/

/**
 * @brief nxeg_initialize
 * 
 * @return int 
 */
int snake_initialize(void)
{
  syslog(LOG_INFO, "snake initialize\n");
  return 0;
}

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, FAR char *argv[])
{
    printf("hello,world\n");
    // nxgl_mxpixel_t color;
    nxgl_mxpixel_t color[CONFIG_NX_NPLANES] = {CONFIG_EXAMPLES_NX_BGCOLOR};
    int16_t ret;
    struct nxhw_handle hwnd1 ={
      .nxeg_initstate = nxeg_initstate,
      .nxeg_openwindow = nxeg_openwindow,
      .nxeg_closewindow = nxeg_closewindow,
      .nxeg_setsize = nxeg_setsize,
      .nxeg_setposition = nxeg_setposition
    };
    struct nxhw_handle hwnd2 ={
      .nxeg_initstate = nxeg_initstate,
      .nxeg_openwindow = nxeg_openwindow,
      .nxeg_closewindow = nxeg_closewindow,
      .nxeg_setsize = nxeg_setsize,
      .nxeg_setposition = nxeg_setposition
    };

    // Initialize
    // 启动NX server线程，启动listener线程，连接NX server
    ret = nxeg_initialize(&g_snake_hnx);
    printf("nx_main: NX handle=%p\n", g_snake_hnx);
    if (!g_snake_hnx || ret < 0)
    {
        printf("nx_main: Failed to get NX handle: %d\n", errno);
        g_exitcode = NXEXIT_NXOPEN;
        goto errout;
    }

    // Get the default font handle
    g_snake_fonthandle = nxf_getfonthandle(CONFIG_EXAMPLES_NX_FONTID);
    if (!g_snake_fonthandle)
    {
        printf("nx_main: Failed to get font handle: %d\n", errno);
        g_exitcode = NXEXIT_FONTOPEN;
        goto errout;
    }

    // Set the background to the configured background color
    ret = nx_setbgcolor(g_snake_hnx, color);
    if (ret < 0)
    {
        printf("nx_main: nx_setbgcolor failed: %d\n", errno);
        g_exitcode = NXEXIT_NXSETBGCOLOR;
        goto errout_with_nx;
    }

    /* *************Create window#1************ */
    printf("nx_main: Create window #1\n");
    nxeg_initstate(&hwnd1.g_wstate, 1, CONFIG_EXAMPLES_NX_COLOR1);
    hwnd1.hwnd = nxeg_openwindow(&g_snake_nxcb, &hwnd1.g_wstate);
    printf("nx_main: hwnd1=%p\n", hwnd1.hwnd);
    if (!hwnd1.hwnd)
    {
        goto errout_with_nx;
    }

    // Wait until we have the screen resolution //等待窗口创建处理完成
    while (!b_snake_haveresolution)
    {
        sem_wait(&g_snake_semevent);
    }

    // Set the size of the window 1
    hwnd1.size.w = g_snake_xres;
    hwnd1.size.h = g_snake_yres-5;

    printf("nx_main: Set window #1 size to (%d,%d)\n", hwnd1.size.w, hwnd1.size.h);
    ret = nxeg_setsize(hwnd1.hwnd, &hwnd1.size);
    if (ret < 0)
    {
        goto errout_with_hwnd1;
    }
   
    // Set the position of window #1
    hwnd1.pt.x = 0; //左上角
    hwnd1.pt.y = 0; //左上角

    printf("nx_main: Set window #1 position to (%d,%d)\n", hwnd1.pt.x, hwnd1.pt.y);
    ret = nxeg_setposition(hwnd1.hwnd, &hwnd1.pt);
    if (ret < 0)
    {
        goto errout_with_hwnd1;
    }

    /* Open the toolbar */
    #ifndef CONFIG_EXAMPLES_NX_RAWWINDOWS
    printf("nx_main: Add toolbar to window #1\n");
    ret = nxeq_opentoolbar(hwnd1.hwnd, CONFIG_EXAMPLES_NX_TOOLBAR_HEIGHT, &g_snake_tbcb, &hwnd1.g_wstate); //TODO:顶部增加退出按钮
    if (ret < 0)
    {
        goto errout_with_hwnd1;
    }

    //延时等待窗口创建完成
    sleep(1);
    nxeg_draw_exit_button(hwnd1.hwnd);
    #endif
    /* ************************************* */


    /* **********Create window #2*********** */
    nxeg_initstate(&hwnd2.g_wstate, 2, 0xffff);
    hwnd2.hwnd = nxeg_openwindow(&g_snake_nxcb, &hwnd2.g_wstate); //分数窗口
    printf("nx_main: hwnd2=%p\n", hwnd2.hwnd);
    if (!hwnd2.hwnd)
    {
        goto errout_with_nx;
    }
    // Wait until we have the screen resolution 
    while (!b_snake_haveresolution)
    {
        sem_wait(&g_snake_semevent);
    }

    hwnd2.size.w = 100;
    hwnd2.size.h = 50;

    ret = nxeg_setsize(hwnd2.hwnd, &hwnd2.size);
    if (ret < 0)
    {
        goto errout_with_hwnd2;
    }
    hwnd2.pt.x = hwnd1.size.w - hwnd2.size.w -50; //左上角
    hwnd2.pt.y = 40; //左上角

    ret = nxeg_setposition(hwnd2.hwnd, &hwnd2.pt);
    if (ret < 0)
    {
        goto errout_with_hwnd2;
    }

    //延时等待窗口创建完成
    sleep(1);

    /* ************************************** */
    
    /* 创建贪吃蛇逻辑任务 */
    ret = snake_logic(g_snake_hnx,&hwnd1,&hwnd2);  //检测返回，有问题直接到窗口销毁退出
    if(ret < 0)
    {
      goto errout_with_hwnd2;
    }

    /* 创建触控检测线程 */
    ret = touch();
    if(ret <0)
    {
      goto out_close;
    }

    pthread_join(Snake_t1,NULL);//等待线程结束

    while(1)
    {
        sleep(1);
    }
    

    //Close the window2
out_close:
    printf("close pthread\n");
    // pthread_cancel(Snake_t1);
    pthread_cancel(Snake_t2);
    // pthread_cancel(Touch_t1);
    // sleep(5);

errout_with_hwnd2:
    printf("nx_main: Close window #2\n");
    nxeg_closewindow(hwnd2.hwnd, &hwnd2.g_wstate);

    // Close the window1
errout_with_hwnd1:
    struct nxgl_rect_s TempRect;
    TempRect.pt1.x = 0;
    TempRect.pt1.y = 0;
    TempRect.pt2.x = 800;
    TempRect.pt2.y = 480;
    nxgl_mxpixel_t nxgl_color[CONFIG_NX_NPLANES];
    nxgl_color[0] = 0x07e2;
    ret = nx_fill(hwnd1.hwnd, &TempRect, nxgl_color);
    if (ret < 0)
    {
        printf("nx_main: nx_fill failed: %d\n", errno);
    }
    printf("nx_main: Close window #1\n");
    nxeg_closewindow(hwnd1.hwnd, &hwnd1.g_wstate);

errout_with_nx:
    printf("nx_main: Disconnect from the server\n");
    nx_disconnect(g_snake_hnx);

errout:
    exit(0);
    return g_exitcode;
}



#endif /* CONFIG_SNAKE */

#ifdef __cplusplus
}
#endif
