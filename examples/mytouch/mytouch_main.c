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

struct touch_sample_s testsample;
static int mytouch_fd = -1;
struct pollfd myfds;

// static int led_fd = -1;
// static int ledSwitch = 1;

// static void test(struct pollfd *fds)
// {
//     printf("hello,in %s: %d\n",__func__,__LINE__);
//     if(ledSwitch == 1)
//     {
//         ledSwitch = 0;
//     }else{
//         ledSwitch = 1;
//     }
//     ioctl(led_fd,ledSwitch,2);
// }

/****************************************************************************
 * Name: overlay_accl
 *
 * Description:
 *   Determines overlay acceleration support
 *
 * Parameters:
 *   fb        - Open framebuffer filehandle
 *   overlayno - Overlay number
 *   accl      - Acceleration to detect
 *
 * Return:
 *   OK    - Success
 *   ERROR - Failed
 ****************************************************************************/

static int overlay_accl(int fb, uint8_t overlayno, uint32_t accl)
{
    int ret;
    struct fb_overlayinfo_s oinfo;

    oinfo.overlay = overlayno;

    ret = ioctl(fb, FBIOGET_OVERLAYINFO, (unsigned long)((uintptr_t)&oinfo));
    if (ret != OK)
        {
            fprintf(stderr, "Unable to get overlay information\n");
            return -1;
        }

    printf("%s: %08" PRIx32 " %08" PRIx32 "\n", __func__, oinfo.accl, accl);
    return (oinfo.accl & accl) ? OK : -1;
}

/****************************************************************************
 * Name: overlay_color
 *
 * Description:
 *   Set overlay color
 *
 * Parameters:
 *   fb    - Open framebuffer filehandle
 *   oinfo - Overlay information
 *
 ****************************************************************************/


static void usage(const char * progname)
{
    fprintf(stderr,
                    "usage: %s <option> -d <fbdev>\n"
                    "\n"
                    "    -vinfo\n"
                    "    -pinfo\n"
                    "    -oinfo overlayno\n"
                    "    -fill overlayno <color> <xpos> <ypos> <xres> <yres>\n"
                    "      color: pixel format color\n"
                    "      xpos: x-offset\n"
                    "      ypos: y-offset\n"
                    "      xres: x-resolution or area width\n"
                    "      yres: y-resolution or area height\n"
#ifdef CONFIG_FB_CMAP
                    "    -cmap <color1> <color2> <color3> <color4> <color5>\n"
                    "      colors: 0xAARRGGBB\n"
                    "      one color must be set at least\n"
#endif
                    "    -color overlayno <value>\n"
                    "      value: pixel format color\n"
                    "    -chroma overlayno <value>\n"
                    "      value: pixel format color\n"
                    "    -transp overlayno <value> <mode>\n"
                    "      value: 0-255\n"
                    "      mode : %d = const alpha, %d = pixel alpha\n"
                    "    -blank : <value>\n"
                    "      0 : On\n"
                    "      1 : Off\n"
                    "    -area overlayno <xpos> <ypos> <xres> <yres>\n"
#ifdef CONFIG_FB_OVERLAY_BLIT
                    "    -blit doverlayno <destxpos> <destypos> <destxres> "
                    "<destyres>\n"
                    "          soverlayno <srcxpos> <srcypos> <srcxres> <srcyres>\n"
                    "    -blend doverlayno <dxpos> <dypos> <dxres> <dyres>\n"
                    "           foverlayno <fxpos> <fypos> <fxres> <fyres>\n"
                    "           boverlayno <bxpos> <bypos> <bxres> <byres>\n"
#endif
                    "\n"
                    "    -d <fbdev> optional, default: \"/dev/fb0\"\n",
                    progname, FB_CONST_ALPHA, FB_PIXEL_ALPHA);
}

static int fbopen(const char * device)
{
    int fb = open(device, O_RDWR);

    if (fb < 0)
        {
            fprintf(stderr, "Unable to open framebuffer device: %s\n", device);
            return EXIT_FAILURE;
        }

    return fb;
}

// int *temp = NULL;
// int *temp2 = NULL;
int main(int argc, FAR char *argv[])
{
    int ret;
    char *fbdevice;
    int  fb = -1;
    struct fb_overlayinfo_s oinfo;
    mytouch_fd = open("/dev/mytouch", O_RDWR);
    printf("mytouch_fd:%d\n",mytouch_fd);
    if(mytouch_fd > 0){
            printf("open mytouch success!\n");
    }
    fbdevice = "/dev/fb0";
    oinfo.overlay   = 1;
    oinfo.color     = 0xfff;

    fb = fbopen(fbdevice);
    if (fb >= 0)
    {
            overlay_color(fb, &oinfo);
    }

    myfds.fd = mytouch_fd; 
    myfds.events = POLLIN;
    // myfds.cb = test;
    // temp2 = (int*)malloc(sizeof(int));
    // temp = temp2;

    printf("In %s:%d\n",__func__,__LINE__);

    while(1)
    {
            ret = poll(&myfds,1,-1);
            if( ret > 0)
            {
                    // printf("In %s: %d\n",__func__,__LINE__);
                    read(mytouch_fd,&testsample,sizeof(struct touch_sample_s));
                    printf("npoints:%d\t x:%d y:%d\n",testsample.npoints,testsample.point->x,testsample.point->y);
            }
    }
    return 0;
    // int ret;
    // mytouch_fd = open("/dev/mytouch", O_RDWR);
    // printf("mytouch_fd:%d\n",mytouch_fd);
    // if(mytouch_fd > 0){
    //     printf("open mytouch success!\n");
    // }

    // char *fbdevice;
    // int  fb = -1;

    // if (argc < 2)
    // {
    //     usage(argv[0]);
    //     return EXIT_FAILURE;
    // }

    // if (argc >= 2 && !strcmp(argv[argc - 2], "-d"))
    // {
    //     fbdevice = argv[argc - 1];
    // }
    // else
    // {
    //     fbdevice = "/dev/fb0";
    // }

    // if (!strcmp(argv[1], "-color") && argc >= 4)
    // {
    //   struct fb_overlayinfo_s oinfo;

    //   oinfo.overlay   = atoi(argv[2]);
    //   oinfo.color     = strtoul(argv[3], NULL, 16);

    //   fb = fbopen(fbdevice);
    //   if (fb >= 0)
    //     {
    //       overlay_color(fb, &oinfo);
    //     }
    // }



    // myfds.fd = mytouch_fd; 
    // myfds.events = POLLIN;
    // // myfds.cb = test;
    
    // printf("In %s:%d\n",__func__,__LINE__);


    // while(1)
    // {
    //     ret = poll(&myfds,1,-1);
    //     if( ret > 0)
    //     {
    //         // printf("In %s: %d\n",__func__,__LINE__);
    //         read(mytouch_fd,&testsample,sizeof(struct touch_sample_s));
    //         printf("npoints:%d\t x:%d y:%d\n",testsample.npoints,testsample.point->x,testsample.point->y);
    //     }
    // }
    // return 0;
}