#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/myled/myled.h>
#include <syslog.h>
#include <unistd.h>//文件操作
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <nuttx/i2c/i2c_master.h>


// #include "../nuttx/boards/arm/stm32/mystm32f429IGT6/include/board.h"

// #include <../nuttx/arch/arm/src/stm32/stm32_gpio.h>
// /home/zeki/workspace/NuttX/nuttxspace/nuttx/arch/arm/src/a1x/hardware/../a1x_lowputc.h

// /home/zeki/workspace/NuttX/nuttxspace/apps/examples/myi2c/myi2c_main.c

// /home/zeki/workspace/NuttX/nuttxspace/nuttx/arch/arm/src/stm32/stm32_gpio.h





FAR struct i2c_transfer_s myMsg;

// typedef enum{
//     eCHANNEL_LED_R,
//     eCHANNEL_LED_G,
//     eCHANNEL_LED_B,
//     eCHANNEL_LED_CNT,
// }myled_channelTypeDef;


static int i2c2_fd = -1;


// static bool _myled_init(void){
//     led_fd = open("/dev/myled", O_WRONLY);
    
//     if(led_fd < 0){
//         printf("myled open failed\n");
//         return false;
//     }
//     return true;
// }

// static bool _set_myled_channal(FAR char * cmd1, FAR char * cmd2)
// {
//     int ret;
    
//     if(strncmp(cmd1,"0",1)!= 0  && strncmp(cmd1,"1",1)!= 0)
//     {
//         printf("error cmd\n");
//         return false;
//     }
//     if(strncmp(cmd2,"1",1)!= 0 && strncmp(cmd2,"2",1)!= 0 && strncmp(cmd2,"3",1)!=0)
//     {
//         printf("error led\n");
//         return false;
//     }
//     int Cmd1num = atoi(cmd1);
//     int Cmd2num = atoi(cmd2);
//     switch(Cmd1num)
//     {
//     case 0:
//         {
//             ioctl(led_fd,Cmd1num,Cmd2num);
//             // write();
//         }
//     break;
//     case 1:
//         {
//             ret = ioctl(led_fd,Cmd1num,Cmd2num);
//             printf("led status: %d\n", ret);
//         }
//     break;
//     default:
//         syslog(LOG_ERR,"error led cmd");
//     break;
//     }

//     return 1;
// }


int main(int argc, FAR char *argv[])
{
    /*******触控芯片初始化*******/


    /**************************/

    uint8_t mybuffer[]={0x80,0X47 };


    i2c2_fd = open("/dev/i2c2", O_WRONLY);
     if(i2c2_fd>0){
        printf("open myi2c2 success!\n");
    }

    
    int len = write(i2c2_fd,"hello",3);
    printf("len: %d\n",len);

    //RESET I2c2不知道有没有用 有用
    ioctl(i2c2_fd, I2CIOC_RESET, 0 );    

    //发送指令的过程有误？
    // myMsg.msgv->frequency = I2C_SPEED_FAST;
    // myMsg.msgv->addr      = I2C_WRITEADDR8(0x5D);
    // // myMsg.msgv->flags     = I2C_M_NOSTOP;
    // myMsg.msgv->buffer    = mybuffer;
    // myMsg.msgv->length    = 2;

    // myMsg.msgc = 2;
    // //
    // ioctl(i2c2_fd, I2CIOC_TRANSFER, &myMsg );

    
    

    close(i2c2_fd);
    return 0;
}