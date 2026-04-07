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

static void sub_test(void *arg) //TODO:可行，需要将监听进程放到后台运行
{
    printf("in sub_test");
    sleep(2);
}


// void main(int argc, FAR char *argv[])
// {
//     int status = 0;
//     while(1)
//     {
//         switch (status)
//         {
//         case 0:
//             /* code */
//             nxterm_input_test_fd_g = open(CONFIG_EXAMPLES_NXTERM_DEVNAME, O_RDWR); //需要在开启nxterm之后再打开这个listen //可以作为nxterm的子线程
//             if (nxterm_input_test_fd_g < 0){
//                 printf("ERROR: Failed to open %s: %d\n", CONFIG_EXAMPLES_NXTERM_DEVNAME, errno);
//                 status = 0;
//             }else{
//                 status = 1;
//             }
//             break;
//         // case 1:
//         //     nxterm_input_test_ret = pthread_create(&thread, NULL, (*pthread_startroutine_t)sub_test, &nxterm_input_test_fd_g);
//         //     if (nxterm_input_test_ret == 0)
//         //     {
//         //         printf("pthread_create success\n");
//         //         goto end;
//         //     }
//         //     break;
//         default:
//             break;
//         }
//     }
// end:
//     pthread_join(thread, NULL);
//     printf("exit test\n");
//     exit(0);
// }

uint16_t u16LedDeviceFd = 0;
void main(int argc, FAR char *argv[])
{
    int8_t u8Ret = -1;
    u16LedDeviceFd = open(CONFIG_EXAMPLES_LED_DEVNAME, O_RDWR);
    if (u16LedDeviceFd < 0)
    {
        printf("ERROR: Failed to open %s: %d\n", CONFIG_EXAMPLES_LED_DEVNAME, errno);
        return;
    }

    while(1){
        u8Ret = ioctl(u16LedDeviceFd, 0, 1);
        if (u8Ret < 0) {
            printf("ERROR: Failed to set led on: %d\n", errno);
            return;
        }
        sleep(1);
        u8Ret = ioctl(u16LedDeviceFd, 0, 0);
        if (u8Ret < 0) {
            printf("ERROR: Failed to set led on: %d\n", errno);
            return;
        }
        sleep(1);
    }
    return;
}