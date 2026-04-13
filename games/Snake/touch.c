#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
// #include <nuttx/analog/ioctl.h>
#include <syslog.h>
#include <unistd.h>//文件操作
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/input/gt9xx.h>
#include <nuttx/../sys/poll.h>

#include <nuttx/input/touchscreen.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

#include <nuttx/video/fb.h>

#include <nuttx/config.h>

extern pthread_t Touch_t1;    //创建线程变量t1
extern pthread_t Snake_t1;    //创建线程变量t1
struct touch_sample_s Snake_Touch;
extern int mytouch_fd;
extern struct pollfd touch_fds;

#define CLOSE_BUTTON_X 780
#define CLOSE_BUTTON_Y 0
#define CLOSE_BUTTON_SIZE 20

void* touch_detect(void *arg)
{
    int ret;
    while(1)
    {
        ret = poll(&touch_fds,1,20);
        if(ret > 0)
        {
            ret = read(mytouch_fd,&Snake_Touch,sizeof(struct touch_sample_s));
            if(ret > 0)
            {
                printf("touch_detect: x=%d, y=%d\n", Snake_Touch.point->x, Snake_Touch.point->y);
                if((Snake_Touch.point->x > CLOSE_BUTTON_X) && 
                (Snake_Touch.point->x < CLOSE_BUTTON_X + CLOSE_BUTTON_SIZE) && 
                (Snake_Touch.point->y > CLOSE_BUTTON_Y) && 
                (Snake_Touch.point->y < CLOSE_BUTTON_Y + CLOSE_BUTTON_SIZE))
                {
                    printf("touch_detect exit\n");
                    pthread_kill(Snake_t1, SIGUSR1); //给Snake_t1线程发送关闭信号
                    pthread_exit(NULL);
                }
            }
        }
    }
}


int touch(void)
{
    int ret;
    mytouch_fd = open("/dev/mytouch", O_RDWR);
    printf("mytouch_fd:%d\n",mytouch_fd);
    if(mytouch_fd > 0){
        printf("open mytouch success!\n");
    }
    touch_fds.fd = mytouch_fd; 
    touch_fds.events = POLLIN;

    pthread_attr_t attr;
    size_t stack_size = 4096; // 设置堆栈大小为 1MB

    // 初始化线程属性对象
    if (pthread_attr_init(&attr) != 0) {
        perror("pthread_attr_init failed");
        return -1;
    }
    // 设置堆栈大小
    if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
        perror("pthread_attr_setstacksize failed");
        return -1;
    }    
    ret = pthread_create(&Touch_t1,&attr,touch_detect,&touch_fds);  //创建界面刷新线程  /* 传入二级指针：指针数组的地址实际为二级指针 */
    // 销毁线程属性对象
    pthread_attr_destroy(&attr);
    if(ret < 0 )
    { 
        return -1;
    }
    return 0;
}