#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "led_handler.h"

#define LED_DEVNAME  "/dev/myled"

void toggle_led_handler(const struct lvgl_msg_s *msg)
{
    int fd = open(LED_DEVNAME, O_RDWR);
    if (fd < 0)
    {
        printf("ERROR: Failed to open %s: %d\n", LED_DEVNAME, errno);
        return;
    }
    int ret = ioctl(fd, 0, msg->led_num);
    if (ret < 0)
        printf("ERROR: Failed to toggle LED #%d: %d\n",
               msg->led_num, errno);
    close(fd);
}
