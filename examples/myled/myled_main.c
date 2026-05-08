#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <nuttx/nx/nxterm.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxtk.h>
#include <nuttx/nx/nxterm.h>
#include <sys/poll.h>
#include <threads.h>
#include <unistd.h>
#include "../nxterm/nxterm_internal.h"

#  define CONFIG_EXAMPLES_LED_DEVNAME "/dev/myled"
#  define CONFIG_EXAMPLES_NXTERM_DEVNAME "/dev/nxterm0"
#  define CONFIG_EXAMPLES_CONSOLE_DEVNAME "/dev/console"

//todo:创建个线程，自己任务跑完
pthread_t thread;
int nxterm_input_test_fd_g;
int nxterm_input_test_consolefd;
int nxterm_input_test_ret;
uint8_t nxterm_input_test_buflen[20];
char nxterm_input_test_buffer;
int nxterm_input_test_nread;
struct pollfd nxterm_input_test_fd_gs;

/* led灯闪烁 */
static void* sub_test(void *arg)
{
    int fd = *(int *)arg;
    printf("in sub_test, fd=%d\n", fd);
    

    int8_t u8Ret = -1;
    uint16_t u16LedDeviceFd = open(CONFIG_EXAMPLES_LED_DEVNAME, O_RDWR);
    if (u16LedDeviceFd < 0)
    {
        printf("ERROR: Failed to open %s: %d\n", CONFIG_EXAMPLES_LED_DEVNAME, errno);
        // return;
    }

    while(1){
        u8Ret = ioctl(u16LedDeviceFd, 0, 1); //切换1号灯状态
        if (u8Ret < 0) {
            printf("ERROR: Failed to set led on: %d\n", errno);
        }
        usleep(500000);
        u8Ret = ioctl(u16LedDeviceFd, 0, 2); //切换2号灯状态
        if (u8Ret < 0) {
            printf("ERROR: Failed to set led on: %d\n", errno);
        }
        usleep(500000);
        u8Ret = ioctl(u16LedDeviceFd, 0, 3); //切换3号灯状态
        if (u8Ret < 0) {
            printf("ERROR: Failed to set led on: %d\n", errno);
            // return;
        }
        usleep(500000);
    }
    return NULL;
}

void main(int argc, FAR char *argv[])
{
    pthread_t pthread = 0;
    pthread_create(&pthread,NULL,(void *)sub_test ,NULL);
    printf("parent thread id: %d\n", pthread_self()); //打印当前id

    while(1){
        sleep(1);  //避免主线程运行后，就死亡了，而子线程没机会
    }
    return;
}
