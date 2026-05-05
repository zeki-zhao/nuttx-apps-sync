#include <nuttx/config.h>
#include <unistd.h>
#include <sys/boardctl.h>
#include <signal.h>

#include "ui/ui.h"

#undef NEED_BOARDINIT

#if defined(CONFIG_BOARDCTL) && !defined(CONFIG_NSH_ARCHINIT)
#  define NEED_BOARDINIT 1
#endif

static volatile bool g_running;

static void sigint_handler(int sig)
{
    g_running = false;
}

int main(int argc, FAR char *argv[])
{
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;

    g_running = true;

    lv_init();

    lv_nuttx_dsc_init(&info);

#ifdef CONFIG_LV_USE_NUTTX_LCD
    info.fb_path = "/dev/lcd0";
#endif

#ifdef CONFIG_INPUT_TOUCHSCREEN
  info.input_path = CONFIG_EXAMPLES_MY_LVGL_INPUT_DEVPATH;
#endif

    lv_nuttx_init(&info, &result);

    if (result.disp == NULL){
        return 1;
    }

    ui_init();

    signal(SIGINT, sigint_handler);

    while (g_running){
        uint32_t idle;
        idle = lv_timer_handler();
        idle = idle ? idle : 1;
        usleep(idle * 1000);
    }

    lv_nuttx_deinit(&result);
    lv_deinit();

    return 0;
}
