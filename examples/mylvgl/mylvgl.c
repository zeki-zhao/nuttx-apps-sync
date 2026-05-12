#include <nuttx/config.h>
#include <nuttx/pthread.h>
#include <unistd.h>
#include <sys/boardctl.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdlib.h>
#include <time.h>

#include "ui/ui.h"
#include "lvgl_event.h"
#include "led_handler.h"
#include "sd_handler.h"
#include "modbus_data.h"

#undef NEED_BOARDINIT

#if defined(CONFIG_BOARDCTL) && !defined(CONFIG_NSH_ARCHINIT)
#  define NEED_BOARDINIT 1
#endif

static volatile bool g_running;

static void sigint_handler(int sig)
{
    g_running = false;
}

static void *modbus_data_thread(void *arg)
{
    while (g_running) {
        usleep(2000000);
        modbus_data_simulate();
    }
    return NULL;
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

    modbus_data_init();
    srand(time(NULL));

    signal(SIGINT, sigint_handler);

    /* initialize event message queue */
    mqd_t mqd = lvgl_event_init();
    if (mqd == (mqd_t)-1)
    {
        goto error;
    }

    lvgl_evt_register(LVGL_MSG_TOGGLE_LED, toggle_led_handler);
    lvgl_evt_register(LVGL_MSG_SAVE_TEXT, save_text_handler);

    pthread_t LvglEvent;
    pthread_create(&LvglEvent, NULL, LvglEventProcess, (void *)(intptr_t)mqd);

    pthread_t ModbusThread;
    pthread_create(&ModbusThread, NULL, modbus_data_thread, NULL);

    while (g_running){
        uint32_t idle;
        idle = lv_timer_handler();
        idle = idle ? idle : 1;
        usleep(idle * 1000);
    }

    pthread_cancel(ModbusThread);
    pthread_join(ModbusThread, NULL);
    
    pthread_cancel(LvglEvent);
    pthread_join(LvglEvent, NULL);
    lvgl_event_fini();

error:
    lv_nuttx_deinit(&result);
    lv_deinit();
    return 0;
}
