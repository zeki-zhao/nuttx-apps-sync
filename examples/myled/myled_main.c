#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <nuttx/myled/myled.h>

#define LED_DEVNAME  "/dev/myled"

static void toggle_led(int num)
{
    char devpath[32];
    snprintf(devpath, sizeof(devpath), "%s%d", LED_DEVNAME, num);

    int fd = open(devpath, O_RDWR);
    if (fd < 0)
    {
        printf("ERROR: Failed to open %s: %d\n", devpath, errno);
        return;
    }

    int ret = ioctl(fd, SLEDIOC_SET, 0);
    if (ret < 0)
        printf("ERROR: Failed to toggle LED %d: %d\n", num, errno);

    close(fd);
}

static void *led_blink_thread(void *arg)
{
    while (1)
    {
        toggle_led(1);
        usleep(500000);
        toggle_led(2);
        usleep(500000);
        toggle_led(3);
        usleep(500000);
    }
    return NULL;
}

int main(int argc, FAR char *argv[])
{
    pthread_t thread;

    printf("Starting LED blink test\n");

    pthread_create(&thread, NULL, led_blink_thread, NULL);
    pthread_join(thread, NULL);

    return 0;
}
