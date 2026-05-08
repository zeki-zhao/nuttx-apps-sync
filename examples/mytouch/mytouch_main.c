#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>//文件操作
#include <pthread.h>
#include <sys/ioctl.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/input/gt9xx.h>
#include <nuttx/../sys/poll.h>

#include <nuttx/input/touchscreen.h>
#include <sys/mman.h>

#include <nuttx/config.h>

#define MAX_TOUCH_POINTS 6

struct
{
  int npoints;
  struct touch_point_s point[MAX_TOUCH_POINTS];
} testsample_storage;

static struct touch_sample_s *testsample = (struct touch_sample_s *)&testsample_storage;
static int mytouch_fd = -1;
struct pollfd myfds;

int main(int argc, FAR char *argv[])
{
    int ret;
    mytouch_fd = open("/dev/mytouch", O_RDWR);
    printf("mytouch_fd:%d\n",mytouch_fd);
    if(mytouch_fd > 0){
            printf("open mytouch success!\n");
    }

    myfds.fd = mytouch_fd; 
    myfds.events = POLLIN;

    printf("In %s:%d\n",__func__,__LINE__);

    while(1)
    {
        ret = poll(&myfds,1,-1);
        if( ret > 0)
        {
            read(mytouch_fd, testsample, sizeof(testsample_storage));
            for (int i = 0; i < testsample->npoints && i < MAX_TOUCH_POINTS; i++)
              {
                printf("  point[%d]: x=%d y=%d\n", i,
                       testsample->point[i].x, testsample->point[i].y);
              }
        } 
    }
    return 0;

}