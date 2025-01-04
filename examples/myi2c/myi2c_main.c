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


typedef struct 
{
  uint16_t start_x;   //按键的x起始坐标  
  uint16_t start_y;   //按键的y起始坐标
  uint16_t end_x;     //按键的x结束坐标 
  uint16_t end_y;     //按键的y结束坐标
  uint32_t para;      //颜色按钮中表示选择的颜色，笔迹形状按钮中表示选择的画刷
  uint8_t touch_flag; //按键按下的标志
    
  void (*draw_btn)(void * btn);     //按键描绘函数
  void (*btn_command)(void * btn);  //按键功能执行函数，例如切换颜色、画刷
 
}Touch_Button;


#define GTP_READ_COOR_ADDR    0x814E
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814A
// #define GTP_REG_CONFIG_DATA   touch_param[touchIC].config_reg_addr
#define GTP_REG_VERSION       0x8140
#define GTP_MAX_TOUCH         5
#define BUTTON_NUM            18
#define GTP_ADDR_LENGTH       2

uint8_t RegisterBuffer[]={0x81,0X40};
uint8_t RecvBuffer2[8]={0x81,0X40,0};
/*按钮结构体数组*/
Touch_Button button[18];


struct i2c_msg_s msgv;
struct i2c_transfer_s myMsg;
static int i2c2_fd = -1;


/**
* @brief  Touch_Button_Down 按键被按下时调用的函数，由触摸屏调用
* @param  x 触摸位置的x坐标
* @param  y 触摸位置的y坐标
* @retval 无
*/
void Touch_Button_Down(uint16_t x,uint16_t y)
{
  uint8_t i;
  for(i=0;i<BUTTON_NUM;i++)
  {
    /* 触摸到了按钮 */
    if(x<=button[i].end_x && y<=button[i].end_y && y>=button[i].start_y && x>=button[i].start_x )
    {
      if(button[i].touch_flag == 0)     /*原本的状态为没有按下，则更新状态*/
      {
      button[i].touch_flag = 1;         /* 记录按下标志 */
      
      button[i].draw_btn(&button[i]);  /*重绘按钮*/
      }        
      
    }
    else if(button[i].touch_flag == 1) /* 触摸移出了按键的范围且之前有按下按钮 */
    {
      button[i].touch_flag = 0;         /* 清除按下标志，判断为误操作*/
      
      button[i].draw_btn(&button[i]);   /*重绘按钮*/
    }

  }

}

/**
* @brief  Touch_Button_Up 按键被释放时调用的函数，由触摸屏调用
* @param  x 触摸最后释放时的x坐标
* @param  y 触摸最后释放时的y坐标
* @retval 无
*/
void Touch_Button_Up(uint16_t x,uint16_t y)
{
   uint8_t i; 
   for(i=0;i<BUTTON_NUM;i++)
   {
     /* 触笔在按钮区域释放 */
      if((x<button[i].end_x && x>button[i].start_x && y<button[i].end_y && y>button[i].start_y))
      {        
        button[i].touch_flag = 0;       /*释放触摸标志*/
        
        button[i].draw_btn(&button[i]); /*重绘按钮*/        
      
        button[i].btn_command(&button[i]);  /*执行按键的功能命令*/
        
        break;
      }
    }  

}

static void GTP_Touch_Down(int32_t id,int32_t x,int32_t y,int32_t w)
{
	/*取x、y初始值大于屏幕像素值*/
    printf("ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
}


/**
  * @brief   用于处理或报告触屏释放
  * @param 释放点的id号
  * @retval 无
  */
static void GTP_Touch_Up( int32_t id)
{
    printf("Touch id[%2d] release!", id);
}


//GTP read
int GTP_I2C_Read(uint8_t client_addr, uint8_t *buf, int32_t len)
{
    int ret;
    msgv.addr = client_addr; //0x5D
    msgv.flags = 0;
    msgv.buffer = buf;
    msgv.length = 2;
    msgv.frequency = I2C_SPEED_FAST;
    myMsg.msgv = &msgv;
    myMsg.msgc = 1;
    ret = ioctl(i2c2_fd, I2CIOC_TRANSFER, (unsigned long)&myMsg);

    msgv.buffer = &buf[2];
    msgv.flags = msgv.flags|I2C_M_READ;
    msgv.length = len-2;
    msgv.frequency = I2C_SPEED_FAST;
    myMsg.msgv = &msgv;
    myMsg.msgc = 1;
    ret = ioctl(i2c2_fd, I2CIOC_TRANSFER, (unsigned long)&myMsg);

    return ret;
}


//GTP write
int GTP_I2C_Write(uint8_t client_addr, uint8_t *buf, int32_t len)
{
    int ret;
    msgv.addr = client_addr; //0x5D
    msgv.flags = 0;
    msgv.buffer = buf;
    msgv.length = len;
    msgv.frequency = I2C_SPEED_FAST;
    myMsg.msgv = &msgv;
    myMsg.msgc = 1;
    ret = ioctl(i2c2_fd, I2CIOC_TRANSFER, (unsigned long)&myMsg);

    return ret;
}

/**
  * @brief   触屏处理函数，轮询或者在触摸中断调用
  * @param 无
  * @retval 无
  */
static void Goodix_TS_Work_Func(void)
{
    uint8_t  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
    uint8_t  point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1]={GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    uint8_t  touch_num = 0;
    uint8_t  finger = 0;
    static uint16_t pre_touch = 0;
    static uint8_t pre_id[GTP_MAX_TOUCH] = {0};

    uint8_t client_addr=0x5d;
    uint8_t* coor_data = NULL;
    int32_t input_x = 0;
    int32_t input_y = 0;
    int32_t input_w = 0;
    uint8_t id = 0;
 
    int32_t i  = 0;
    int32_t ret = -1;

    // GTP_DEBUG_FUNC();

    ret = GTP_I2C_Read(client_addr, point_data, 12);//10字节寄存器加2字节地址
    if (ret < 0)
    {
        printf("I2C transfer error. errno:%d\n ", ret);

        return;
    }
    
    finger = point_data[GTP_ADDR_LENGTH];//状态寄存器数据

    if (finger == 0x00)		//没有数据，退出
    {
        return;
    }

    if((finger & 0x80) == 0)//判断buffer status位
    {
        goto exit_work_func;//坐标未就绪，数据无效
    }

    touch_num = finger & 0x0f;//坐标点数
    if (touch_num > GTP_MAX_TOUCH)
    {
        goto exit_work_func;//大于最大支持点数，错误退出
    }

    if (touch_num > 1)//不止一个点
    {
        uint8_t buf[8 * GTP_MAX_TOUCH] = {(GTP_READ_COOR_ADDR + 10) >> 8, (GTP_READ_COOR_ADDR + 10) & 0xff};

        ret = GTP_I2C_Read(client_addr, buf, 2 + 8 * (touch_num - 1));
        memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));			//复制其余点数的数据到point_data
    }

    
    
    if (pre_touch>touch_num)				//pre_touch>touch_num,表示有的点释放了
    {
        for (i = 0; i < pre_touch; i++)						//一个点一个点处理
         {
            uint8_t j;
           for(j=0; j<touch_num; j++)
           {
               coor_data = &point_data[j * 8 + 3];
               id = coor_data[0] & 0x0F;									//track id
              if(pre_id[i] == id)
                break;

              if(j >= touch_num-1)											//遍历当前所有id都找不到pre_id[i]，表示已释放
              {
                 GTP_Touch_Up( pre_id[i]);
              }
           }
       }
    }


    if (touch_num)
    {
        for (i = 0; i < touch_num; i++)						//一个点一个点处理
        {
            coor_data = &point_data[i * 8 + 3];

            id = coor_data[0] & 0x0F;									//track id
            pre_id[i] = id;

            input_x  = coor_data[1] | (coor_data[2] << 8);	//x坐标
            input_y  = coor_data[3] | (coor_data[4] << 8);	//y坐标
            input_w  = coor_data[5] | (coor_data[6] << 8);	//size
        
            {
                GTP_Touch_Down( id, input_x, input_y, input_w);//数据处理
            }
        }
    }
    else if (pre_touch)		//touch_ num=0 且pre_touch！=0
    {
      for(i=0;i<pre_touch;i++)
      {
          GTP_Touch_Up(pre_id[i]);
      }
    }


    pre_touch = touch_num;


exit_work_func:
    {
        ret = GTP_I2C_Write(client_addr, end_cmd, 3);
        if (ret < 0)
        {
            // GTP_INFO("I2C write end_cmd error!");
        }
    }

}


int main(int argc, FAR char *argv[])
{
    /*******触控芯片初始化*******/


    /**************************/

    int ret;

    i2c2_fd = open("/dev/i2c2", O_WRONLY);
     if(i2c2_fd>0){
        printf("open myi2c2 success!\n");
    }

    
    int len = write(i2c2_fd,"hello",3);
    printf("len: %d\n",len);

    // RESET I2c2不知道有没有用 有用
    // ioctl(i2c2_fd, I2CIOC_RESET, 0 );    

    // msgv.addr = 0x5d; //0x5D
    // msgv.flags = 0;
    // msgv.buffer = RegisterBuffer;
    // msgv.length = 2;
    // msgv.frequency = I2C_SPEED_FAST;
    // myMsg.msgv = &msgv;
    // myMsg.msgc = 1;

    // // for (int trytime = 0; trytime < 20; trytime++)
    // // {
    // //   /* Read the MAC address */
    //  printf("I2C_SPEED_FAST: %d\n",myMsg.msgv->frequency);
    //  printf("myMsg.msgv->length: %d\n",myMsg.msgv->length);
    //  printf("myMsg.msgv->addr: %d\n",myMsg.msgv->addr);
    //  printf("myMsg.msgc: %d\n",myMsg.msgc);
    // ret = ioctl(i2c2_fd, I2CIOC_TRANSFER, (unsigned long)&myMsg);

    // msgv.buffer = RecvBuffer2;
    // msgv.flags = msgv.flags|I2C_M_READ;
    // msgv.length = 8;
    // msgv.frequency = I2C_SPEED_FAST;
    // myMsg.msgv = &msgv;
    // myMsg.msgc = 1;
    // ret = ioctl(i2c2_fd, I2CIOC_TRANSFER, (unsigned long)&myMsg);

    GTP_I2C_Read(0x5d,RecvBuffer2,8);

    for(int i = 2; i < (sizeof(RecvBuffer2) - 2); i++)
    {
        printf("%c",RecvBuffer2[i]);
    }

    while (1)
    {
        Goodix_TS_Work_Func();
    }
    
    
finish:

    close(i2c2_fd);
    return 0;
}