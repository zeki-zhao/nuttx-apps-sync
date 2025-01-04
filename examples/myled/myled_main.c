#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nuttx/config.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/myled/myled.h>
#include <syslog.h>
#include	<unistd.h>//文件操作
#include	<string.h>

#include <pthread.h>
#include <sys/ioctl.h>

#include <nuttx/../unistd.h>
#include <nuttx/fs/fs.h>

// struct myled_lower_s LedTest;

typedef enum{
    eCHANNEL_LED_R,
    eCHANNEL_LED_G,
    eCHANNEL_LED_B,
    eCHANNEL_LED_CNT,
}myled_channelTypeDef;
static int led_fd = -1;
static int usart2_fd = -1;
static int mmc_fd = -1;


// static bool _myled_init(void){
//     led_fd = open("/dev/myled", O_WRONLY);
    
//     if(led_fd < 0){
//         printf("myled open failed\n");
//         return false;
//     }
//     return true;
// }

static bool _set_myled_channal(FAR char * cmd1, FAR char * cmd2)
{
    int ret;
    
    if(strncmp(cmd1,"0",1)!= 0  && strncmp(cmd1,"1",1)!= 0)
    {
        printf("error cmd\n");
        return false;
    }
    if(strncmp(cmd2,"1",1)!= 0 && strncmp(cmd2,"2",1)!= 0 && strncmp(cmd2,"3",1)!=0)
    {
        printf("error led\n");
        return false;
    }
    int Cmd1num = atoi(cmd1);
    int Cmd2num = atoi(cmd2);
    switch(Cmd1num)
    {
    case 0:
        {
            ioctl(led_fd,Cmd1num,Cmd2num);
            // write();
        }
    break;
    case 1:
        {
            ret = ioctl(led_fd,Cmd1num,Cmd2num);
            printf("led status: %d\n", ret);
        }
    break;
    default:
        syslog(LOG_ERR,"error led cmd");
    break;
    }

    return 1;
}

void myfun(void)
{
    while(1)
    {
        sleep(1); 
        write(usart2_fd,"hello,zeki\n",12);
        printf("child thread id: %d\n", pthread_self()); 
    }
}
int main(int argc, FAR char *argv[])
{
    int tempCnt;
    char *p;
    char output[500] ={0};
    // led_fd = open("/dev/myled", O_WRONLY);
    //  if(led_fd>0){
    //     printf("open myled success!\n");
    // }
    // usart2_fd = open("/dev/ttyS1",O_WRONLY);
    //     if(led_fd>0){
    //     printf("open usart2 success!\n");
    // }

    // pthread_t pthread = 0;
    // pthread_create(&pthread,NULL,(void *)myfun ,NULL);
    // printf("parent thread id: %d\n", pthread_self()); //打印当前id

    // sleep(2);  //避免主线程运行后，就死亡了，而子线程没机会
   
    // _set_myled_channal(argv[1], argv[2]);

    // while(1)
    // {
    //     sleep(5);
    //     printf("parent thread id: %d\n", pthread_self()); //打印当前id
    // }
    char tempinput[] = {0x21,0x22,0x23,0x24,0x25};

    struct inode *inode;
    int           ret;
    // int           x;
    // int           logsector;
    // uint16_t      seq;
    // uint16_t     *sectors;
    // uint16_t     *seqs;
    // char         *buffer;

    /* Find the inode of the block driver identified by 'source' */

    ret = open_blockdriver("/dev/mmcsd0", 0, &inode);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return ret;
    }else
    {
        printf("open mmcsd0 success!\n");
    }

    /* Get the low-level format from the device. */


    ret = inode->u.i_bops->write(inode, tempinput, 0, 1);
    // if (ret != OK)
    // {
    //     fprintf(stderr, "write error\n");
    // }

    ret = inode->u.i_bops->read(inode, output, 0, 1); //接受的缓冲区需要一个扇区大小？
    // if (ret != OK)
    // {
    //     printf(stderr, "read error\n");
    // }

    p = output;
    tempCnt = sizeof(tempinput);
    printf("tempinput:");
    while(tempCnt--)
    {
        printf("%d\t",*(p++));
    }
    printf("\n");


    tempinput[0]=233;
    ret = inode->u.i_bops->write(inode, tempinput, 0, 1);
    ret = inode->u.i_bops->read(inode, output, 0, 1);
    p = output;
    tempCnt = sizeof(tempinput);
    printf("tempinput:");
    while(tempCnt--)
    {
        printf("%d\t",*(p++));
    }
    printf("\n");



    close_blockdriver(inode);
    // close(led_fd);
    return 0;
}