#include <nuttx/config.h>
#include <nuttx/pthread.h>
#include <unistd.h>
#include <sys/boardctl.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ui/ui.h"
#include "ui/screens/ui_Screen2.h"
#include "lvgl_event.h"
#include "led_handler.h"
#include "sd_handler.h"
#include "modbus_slave.h"
#include "nsh_terminal.h"
#include "netutils/cJSON.h"
#include <arch/board/board_paths.h>

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
        modbus_serial_poll();
    }
}

static void *modbus_timer_thread(void *arg)
{
    while (g_running) {
        modbus_timer_tick();
        usleep(1000);  /* 1 ms */
    }
}

static void restore_modbus_slave_config(void)
{
    char path[128];
    snprintf(path, sizeof(path), SD_CONFIG_DIR "/modbus_slave_show_config.json");

    FILE *fp = fopen(path, "r");
    if (!fp)
        return;

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    if (content) {
        fread(content, 1, fsize, fp);
        content[fsize] = '\0';

        cJSON *root = cJSON_Parse(content);
        if (root) {
            cJSON *addr = cJSON_GetObjectItem(root, "start_addr");
            cJSON *rows = cJSON_GetObjectItem(root, "num_rows");
            cJSON *type = cJSON_GetObjectItem(root, "reg_type");

            if (addr && rows && type) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%X", addr->valueint);
                lv_textarea_set_text(ui_TextArea1, buf);
                snprintf(buf, sizeof(buf), "%d", rows->valueint);
                lv_textarea_set_text(ui_TextArea2, buf);
                lv_dropdown_set_selected(ui_Dropdown1, type->valueint);
            }

            cJSON_Delete(root);
        }
        free(content);
    }

    fclose(fp);
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
    ui_Terminal_screen_init();
    ui_Terminal_homebutton_create(ui_HomeScreen);

    restore_modbus_slave_config();

    nsh_terminal_init();

    modbus_data_init();
    srand(time(NULL));
    
    signal(SIGINT, sigint_handler);

    /* initialize event message queue */
    mqd_t mqd = lvgl_event_init();
    if (mqd == (mqd_t)-1)
    {
        goto error;
    }

    lvgl_evt_register(LVGL_MSG_SET_LED, set_led_handler);
    lvgl_evt_register(LVGL_MSG_SAVE_LED_STATUS, save_led_status_handler);
    lvgl_evt_register(LVGL_MSG_SAVE_TEXT, save_text_handler);
    lvgl_evt_register(LVGL_MSG_SAVE_MODBUS_SLAVE_CONFIG, save_modbus_slave_show_config_hander);
    

    pthread_t LvglEvent; /* lvgl消息队列处理线程 */
    pthread_create(&LvglEvent, NULL, LvglEventProcess, (void *)(intptr_t)mqd);

    pthread_t ModbusThread; /* modbus数据模拟线程 */
    pthread_create(&ModbusThread, NULL, modbus_data_thread, NULL);

    pthread_t ModbusTimer; /* 1ms协议定时器线程 */
    pthread_create(&ModbusTimer, NULL, modbus_timer_thread, NULL);

    while (g_running){
        uint32_t idle;
        idle = lv_timer_handler();
        idle = idle ? idle : 1;
        usleep(idle * 1000);
    }

    pthread_cancel(ModbusTimer);
    pthread_join(ModbusTimer, NULL);

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
