#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <stddef.h>
#include <syslog.h>

#include "led_handler.h"
#include <arch/board/board_paths.h>

#define LED_DEVNAME  "/dev/myled"

#define FILE_NAME  "led_status.bin"


void led_state_init(void)
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        char devpath[32];
        snprintf(devpath, sizeof(devpath), "%s%d", LED_DEVNAME, i + 1);

        int fd = open(devpath, O_RDWR);
        if (fd < 0)
            continue;

        char buf[4] = {0};
        read(fd, buf, sizeof(buf));
        close(fd);

        if (buf[0] == '1')
            g_led_file.led_state[0] |=  (1 << i);
        else
            g_led_file.led_state[0] &= ~(1 << i);
    }

    g_led_file.led_count = LED_COUNT;
}

int lvgl_event_send_set_led(uint8_t led_num, bool on)
{
    mqd_t mqd = lvgl_event_get_mqd(LVGL_EVENT_MQ_NAME);
    if (mqd == (mqd_t)-1)
        return -ENODEV;

    struct lvgl_msg_s msg =
    {
        .type    = LVGL_MSG_SET_LED,
        .led_num = led_num,
        .led_on  = on,
    };
    mq_send(mqd, (const char *)&msg, sizeof(msg), 0);

    msg.type = LVGL_MSG_SAVE_LED_STATUS;
    mq_send(mqd, (const char *)&msg, sizeof(msg), 0);

    return 0;
}

void set_led_handler(const struct lvgl_msg_s *msg)
{
    int idx = msg->led_num - 1;
    if (idx < 0 || idx >= LED_COUNT)
        return;

    char devpath[32];
    snprintf(devpath, sizeof(devpath), "%s%d", LED_DEVNAME, msg->led_num);

    int fd = open(devpath, O_RDWR);
    if (fd < 0)
    {
        printf("ERROR: Failed to open %s: %d\n", devpath, errno);
        return;
    }

    char c = msg->led_on ? '1' : '0';
    write(fd, &c, 1);
    close(fd);

    if (msg->led_on)
        g_led_file.led_state[0] |=  (1 << idx);
    else
        g_led_file.led_state[0] &= ~(1 << idx);
}

void save_led_status_handler(const struct lvgl_msg_s *msg)
{
    char path[128];
    snprintf(path, sizeof(path), SPI_FLASH_DEVICE_STATUS_DIR "/" FILE_NAME);
    mkdir(SPI_FLASH_DEVICE_STATUS_DIR, 0777);
    FILE *fp = fopen(path, "w");
    if (fp) {
        fwrite(&g_led_file, 1, sizeof(g_led_file), fp);
        fclose(fp);
        syslog(LOG_NOTICE,"Saved: %s\n", path);
    } else {
        syslog(LOG_NOTICE,"ERROR: Failed to create %s: %d\n", path, errno);
    }

}
