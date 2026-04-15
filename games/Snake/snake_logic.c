/**
*   @file   snake_logic.c
***********************************************************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif


/***********************************************************************************************************************
*                                                     INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
***********************************************************************************************************************/
#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <debug.h>

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <nuttx/video/fb.h>
#include <fcntl.h> // 文件操作符

/*显示库*/
#include <nuttx/nx/nxglib.h>
#include "nx_internal.h"
#include "touch.h"

#include "touch.h"

#include "snake_logic.h"

/*add user code*/


/***********************************************************************************************************************
*                                            SOURCE FILE VERSION INFORMATION
***********************************************************************************************************************/
#define SNAKE_LOGIC_SW_MAJOR_VERSION_C             0
#define SNAKE_LOGIC_SW_MINOR_VERSION_C             0
#define SNAKE_LOGIC_SW_PATCH_VERSION_C             0

/***********************************************************************************************************************
*                                                  FILE VERSION CHECKS
***********************************************************************************************************************/
#if ((SNAKE_LOGIC_SW_MAJOR_VERSION_C != SNAKE_LOGIC_SW_MAJOR_VERSION) || \
     (SNAKE_LOGIC_SW_MINOR_VERSION_C != SNAKE_LOGIC_SW_MINOR_VERSION) || \
     (SNAKE_LOGIC_SW_PATCH_VERSION_C != SNAKE_LOGIC_SW_PATCH_VERSION))
#error "Software Version Numbers of snake_logic.c and snake_logic.h are different"
#endif


/***********************************************************************************************************************
*                                                   DEFINES AND MACROS
***********************************************************************************************************************/
#define PIXEL_SIZE (3) 

#define BOUNDARY_SIZE 3

#define UP_BOUNDARY (40)
#define DOWN_BOUNDARY (480-10)
#define LEFT_BOUNDARY (20)
#define RIGHT_BOUNDARY (600)

#define BUTTON_SIZE (50)
#define UP_BUTTON_LOCATION_X (800 - 120)
#define UP_BUTTON_LOCATION_Y (310)
#define DOWN_BUTTON_LOCATION_X (800 - 120)
#define DOWN_BUTTON_LOCATION_Y (410) 
#define LEFT_BUTTON_LOCATION_X (800 - 170) 
#define LEFT_BUTTON_LOCATION_Y (360)
#define RIGHT_BUTTON_LOCATION_X (800 - 70)
#define RIGHT_BUTTON_LOCATION_Y (360)


#define  UP     1
#define  DOWN   -1
#define  LEFT   2
#define  RIGHT  -2

/***********************************************************************************************************************
*                                                        ENUMS
***********************************************************************************************************************/
enum{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    ALL_KEY
};


/***********************************************************************************************************************
*                                              STRUCTURES AND OTHER TYPEDEFS
***********************************************************************************************************************/
/*蛇和食物的结构体*/
struct Snake
{
    int x;  //x位置
    int y; //y位置 
    struct Snake *next;
};

/***********************************************************************************************************************
*                                              STATIC VARIABLE DECLARATIONS
***********************************************************************************************************************/


/***********************************************************************************************************************
*                                              GLOBAL VARIABLE DECLARATIONS
***********************************************************************************************************************/
struct nxhw_handle *g_hwnd_main = NULL;
struct nxhw_handle *g_hwnd_score = NULL;
static struct Snake *head = NULL;      //链表头，蛇尾
static struct Snake *tail = NULL;        //链表尾，蛇头
static int Score = 0;           //分数
static nxgl_mxpixel_t nxgl_color[CONFIG_NX_NPLANES];

struct Snake food;              //食物
static int16_t s16Dir = RIGHT; //记录输入的方向键


/***********************************************************************************************************************
*                                               STATIC FUNCTION PROTOTYPES
***********************************************************************************************************************/


/***********************************************************************************************************************
*                                              STATIC FUNCTION DEFINITIONS
***********************************************************************************************************************/
static int snake_show_score_windows(struct nxhw_handle *hwnd_score)
{
    int ret;
    uint8_t g_kbdmsg[20];
    printf("&g_kbdmsg=%p\n",g_kbdmsg);
    snprintf((char*)g_kbdmsg,sizeof(g_kbdmsg),"Score: %03d",Score);;
    ret = nx_kbdin(g_snake_hnx, strlen((FAR const char *)g_kbdmsg), g_kbdmsg);
    if (ret < 0)
    {
        printf("nx_main: nx_kbdin failed: %d\n", errno);
        return -1;
    }
    return 0;
}

/*方向函数*/
static void turn(int16_t s16Direction)
{
    if(abs(s16Dir) != abs(s16Direction))
    {
        s16Dir = s16Direction;        //方向取绝对值比较，当左右运动时只有上下输入才生效
    }
}


static int16_t Snake_Get_Dir(void)
{
    if((Snake_Touch.point->x > UP_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < UP_BUTTON_LOCATION_X + BUTTON_SIZE) && 
        (Snake_Touch.point->y > UP_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < UP_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {        
        return KEY_UP; 
    }else if((Snake_Touch.point->x > DOWN_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < DOWN_BUTTON_LOCATION_X + BUTTON_SIZE)&& 
        (Snake_Touch.point->y > DOWN_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < DOWN_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        return KEY_DOWN; 
    }else if((Snake_Touch.point->x > LEFT_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < LEFT_BUTTON_LOCATION_X + BUTTON_SIZE)&& 
        (Snake_Touch.point->y > LEFT_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < LEFT_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        return KEY_LEFT; 
    }else if((Snake_Touch.point->x > RIGHT_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < RIGHT_BUTTON_LOCATION_X + BUTTON_SIZE)&& 
        (Snake_Touch.point->y > RIGHT_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < RIGHT_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        return KEY_RIGHT; 
    }else{
        return -1;
    }
}

/*键盘方向输入监测线程函数*/
static void* changeDir(void *arg)
{
    while(1)
    {
        sem_wait(&stDirChangeSemevent);
        switch(Snake_Get_Dir())
        {
            case KEY_UP:
                printf("up\n");
                turn(UP);       //上
                break;
            case KEY_DOWN:
                printf("down\n");
                turn(DOWN);     //下
                break;
            case KEY_LEFT:
                printf("left\n");
                turn(LEFT);     //左
                break;
            case KEY_RIGHT:
                printf("right\n");
                turn(RIGHT);    //右
                break;
            default:
                break;
        }
    }
}

static void Snake_draw_rectangle(NXEGWINDOW *hwnd,int16_t x1,int16_t y1,int16_t x2, int16_t y2, uint16_t color)
{
    // printf("in %s :%d\n",__FILE__,__LINE__);
    int ret;
    struct nxgl_rect_s TempRect;
    TempRect.pt1.x = x1;
    TempRect.pt1.y = y1;
    TempRect.pt2.x = x2;
    TempRect.pt2.y = y2;
    nxgl_color[0] = CONFIG_EXAMPLES_NX_FONTCOLOR;
    ret = nx_fill(hwnd, &TempRect, nxgl_color);
    if (ret < 0)
    {
        printf("nx_main: nx_fill failed: %d\n", errno);
    }
}

/*添加蛇身节点*/
static void addNode(int16_t s16Direction)
{
    struct Snake *new;         //新节点变量
    new = (struct Snake *)malloc(sizeof(struct Snake));//为新节点开辟内存空间

    switch(s16Direction)                //方向键判断
    {
        case UP:
            new->y = tail->y - PIXEL_SIZE;  //向上，行减1，上移
            new->x = tail->x;        //列保持
            break;
        case DOWN:
            new->y = tail->y + PIXEL_SIZE;  //向下，行加1，下移
            new->x = tail->x;        //列保持不变
            break;
        case LEFT:
            new->y = tail->y;      //行保持不变
            new->x = tail->x - PIXEL_SIZE;    //向左，列减1，左移
            break;
        case RIGHT:
            new->y = tail->y;      //行保持不变
            new->x = tail->x + PIXEL_SIZE;    //向右，列加1，右移
            break;
        default:
            break;
    }

    new->next = NULL;           //新节点的下一个节点指向为NULL
    tail->next = new;           //尾部的下一个节点指向新节点
    tail = new;                 //新节点复制给尾部节点
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


/***********************************************************************************************************************
*                                              GLOBAL FUNCTION DEFINITIONS
***********************************************************************************************************************/

/*输入横坐标和纵坐标值，判断是否存在蛇的链表节点*/
int hasSnakeNode(int i,int j)
{
    struct Snake *p;            //临时变量，用于记录蛇的链表表头
    p = head;

    while(p != NULL)            //当链表头不为空时，进入循环
    {
        if(p->y == i && p->x == j)
        {
            printf("in %s :%d\n",__FILE__,__LINE__);
            printf("x:%d, y:%d\n",j,i);
            return 1;           //输入的行纵坐标存在于蛇的链表中时返回1
        }
        p = p->next;            //链表头指向下一个节点
    }
    return 0;                   //输入的横纵坐标不存在蛇的链表中时返回0
}

/*食物初始化*/
void initFoodnode(NXEGWINDOW *hwnd)
{
    int x;                     //定义横坐标临时变量
    int y;                     //定义纵坐标临时变量
    x = LEFT_BOUNDARY+10+rand()%(RIGHT_BOUNDARY-LEFT_BOUNDARY-10);            //在范围内随机获取横坐标值
    y = UP_BOUNDARY+10+rand()%(DOWN_BOUNDARY-UP_BOUNDARY-10);             //在范围内随机获取纵坐标值

    Snake_draw_squares(hwnd,food.x-1,food.y-1,2,0); /* 消除原有食物色块 */

    food.x = x;       
    food.y = y;   
    
    Snake_draw_squares(hwnd,food.x-1,food.y-1,2,0xffff); /* 绘制食物色块 */

    printf("food x:%d, y:%d \n",food.x,food.y);
}

/*输入横纵坐标判断是否存在食物，用于地图刷新*/
int hasFoodnode(int i,int j)
{
    //食物[x-1,x+1][y-1,y+1] 与蛇头[x-1,x+1][y-1][y+1]重合即可判定
    if(((food.x<= i && (food.x+2)>= i)&&(food.y<= j && (food.y+2)>= j))
    || (((food.x-2)<= i && food.x>= i)&&((food.y-2)<= j && food.y>= j)))
    {
        return 1;               //吃到食物返回1
    }
    return 0;                   //未吃到食物返回0
}

/* 地图绘制 */
void gamePic(NXEGWINDOW *hwnd)
{
    int y,x;              //行列临时变量

    /*绘制活动空间  */
    Snake_draw_rectangle(hwnd,LEFT_BOUNDARY,UP_BOUNDARY,RIGHT_BOUNDARY,DOWN_BOUNDARY,CONFIG_EXAMPLES_NX_FONTCOLOR);

    /* 上键 */
    Snake_draw_squares(hwnd,UP_BUTTON_LOCATION_X,UP_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_FONTCOLOR);
    Snake_draw_squares(hwnd,DOWN_BUTTON_LOCATION_X,DOWN_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_FONTCOLOR);
    Snake_draw_squares(hwnd,LEFT_BUTTON_LOCATION_X,LEFT_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_FONTCOLOR);
    Snake_draw_squares(hwnd,RIGHT_BUTTON_LOCATION_X,RIGHT_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_FONTCOLOR);

    for(y=UP_BOUNDARY; y<=DOWN_BOUNDARY; y++) //历遍上下边界
    {
        if(y>=UP_BOUNDARY || y<=DOWN_BOUNDARY)
        {
            for(x=LEFT_BOUNDARY;x<=RIGHT_BOUNDARY;x++) /* 历遍左右边界 */
            {
                if(x==LEFT_BOUNDARY || x==RIGHT_BOUNDARY) /* 画左右边界 */
                {
                    Snake_draw_squares(hwnd,x-1,y-1,2,CONFIG_EXAMPLES_NX_TBCOLOR);
                }
                else if(hasSnakeNode(y,x)) /* 画蛇身 */
                {
                    Snake_draw_squares(hwnd,x-1,y-1,2,0xffff);
                }
            }
        }
        if(y==UP_BOUNDARY || y==DOWN_BOUNDARY)
        {
            for(x=LEFT_BOUNDARY; x<=RIGHT_BOUNDARY; x++)
            {
                Snake_draw_squares(hwnd,x-1,y-1,2,CONFIG_EXAMPLES_NX_TBCOLOR); /* 绘制上下边界 */
            }
        }
    }
    initFoodnode(hwnd);  /* 初始化食物 */
}



void Snake_draw_squares(NXEGWINDOW *hwnd,int16_t x,int16_t y,int16_t size, uint16_t color)
{
    int ret;
    struct nxgl_rect_s TempRect;
    TempRect.pt1.x = x;
    TempRect.pt1.y = y;
    TempRect.pt2.x = x+size;
    TempRect.pt2.y = y+size;
    nxgl_color[0] = color;
    ret = nx_fill(hwnd, &TempRect, nxgl_color);
    if (ret < 0)
    {
        printf("nx_main: nx_fill failed: %d\n", errno);
    }
}




/*初始化蛇*/
void initSnake(NXEGWINDOW *hwnd)
{
    struct Snake *p;            //临时变量，指向蛇的链表头

    while(head != NULL)         //当链表头不为空时进入，用于释放蛇当前的链表占用内存空间
    {
        p = head;               //p指向链表头
        head = head->next;      //链表头指向下一个节点
        Snake_draw_squares(hwnd,(p->x)-1,(p->y)-1,2,0x0000); //清空原有蛇的颜色   
        free(p);                //释放链表头内存
    }

    
    head = (struct Snake *)malloc(sizeof(struct Snake)); //为链表头开辟新的内存空间
    head->y = UP_BOUNDARY+PIXEL_SIZE*3;             //链表头行初始值为2
    head->x = LEFT_BOUNDARY+PIXEL_SIZE*3;              //链表头列初始值为2
    head->next = NULL;          //链表头的下一个节点指向为NULL
    tail = head;                //链表尾指向链表头

    addNode(RIGHT);                  //为链表添加新节点
    addNode(RIGHT);
    addNode(RIGHT);
    addNode(RIGHT);
}

/*节点删除*/
void deleteNode()
{
    struct Snake *p;            //创建临时节点
    p = head;                    //节点指向链表头
    head = head->next;          //链表头指向下一个节点
    free(p);                    //释放p的空间(原链表头)
}

/*判断蛇是否越界或自残*/
int ifSnakedie()
{
    struct Snake *p;            //创建临时节点
    p = head;                   //指向链表头

    if((tail->y-BOUNDARY_SIZE)<=UP_BOUNDARY || (tail->x-BOUNDARY_SIZE)<=LEFT_BOUNDARY || (tail->y+BOUNDARY_SIZE)>=DOWN_BOUNDARY || (tail->x+BOUNDARY_SIZE)>=RIGHT_BOUNDARY)
    {
        return 1;               //当蛇链表的尾部坐标等于边界值时，返回1
    }

    while(p->next != NULL)
    {
        if((p->y==tail->y)&&(p->x==tail->x))
        {
            return 1;           //当蛇链表其它的节点与尾部节点坐标相同，返回1
        }
        p = p->next;
    }

    return 0;                   //无越界，无自残，返回0
}


/*刷新分数*/
void showScore(struct nxhw_handle *hwnd_score)
{   
    uint8_t g_kbdmsg[20]= {0};

    hwnd_score->g_wstate.nchars = 0;
    hwnd_score->g_wstate.nglyphs = 0;
    
    sprintf((char *)g_kbdmsg,"Score: %03d",Score);
    nx_kbdin(g_snake_hnx, strlen((FAR const char *)g_kbdmsg), g_kbdmsg);
}


/*蛇移动*/
void moveSnake(NXEGWINDOW *hwnd, struct nxhw_handle *hwnd_score)
{
    addNode(s16Dir); /* 添加新节点,tail是蛇的头部 */
    if(ifSnakedie()){   /* 死亡检测：死亡 */
        printf("die!!!!!\n");
        printf("Last Score:%d\n", Score);
        s16Dir = RIGHT; /* 死亡后默认向右移动 */
        Score = 0;
        showScore(hwnd_score); /* 刷新分数 */
        initSnake(hwnd);    /* 如果满足越界或者自残条件，重新初始化蛇链表 */
        initFoodnode(hwnd);   /* 当蛇链表尾节点坐标值和食物坐标值一样，刷新食物位置 */
    }else{ /* 未死亡 */
        if(hasFoodnode(tail->x,tail->y)){ /* 吃到食物了，不删蛇尾 */
            Score++;
            showScore(hwnd_score); /* 刷新分数 */
            initFoodnode(hwnd);   /* 当蛇链表尾节点坐标值和食物坐标值一样，刷新食物位置 */
        }else{
            Snake_draw_squares(hwnd,head->x-1,head->y-1,2,0x0000); /* 尾部恢复背景色 */
            deleteNode();   /* 删除蛇链表中的头节点 */
        }
        Snake_draw_squares(hwnd,tail->x-1,tail->y-1,2,0xffff); /* 头部新加色块 */
    }
}

void signal_handler(int signum) 
{
    printf("Thread received signal: %d\n", signum);
    struct Snake *p;            //临时变量，指向蛇的链表头
    while(head != NULL)         //当链表头不为空时进入，用于释放蛇当前的链表占用内存空间
    {
        p = head;               //p指向链表头
        head = head->next;      //链表头指向下一个节点
        free(p);                //释放链表头内存
    }

    pthread_exit(NULL);
}


/*地图界面刷新线程函数*/
void* refreshjiemian(void *arg)
{
    struct nxhw_handle **nxhw_handle_set; //二级指针
    nxhw_handle_set = (struct nxhw_handle **)arg; //指针数组
    printf("nxhw_handle_set addr:%p\n",(void *)nxhw_handle_set);
    
    // signal(SIGUSR1, signal_handler);

    while(1)
    {
        moveSnake(g_hwnd_main->hwnd, g_hwnd_score);           //蛇链表移动
        usleep(50000);           //线程休眠函数，100ms
        // pthread_testcancel();//主动设置取消点
    } 
}


int snake_logic(NXHANDLE *hnx,struct nxhw_handle *hwnd_main,struct nxhw_handle *hwnd_score)
{
    int ret;
    printf("&head=%p\n",&head);
    static struct nxhw_handle * nxhw_handle_set[2]; //指针数组

    nxhw_handle_set[0] = hwnd_main; //数组0保存地址
    nxhw_handle_set[1] = hwnd_score;

    ret = snake_show_score_windows(hwnd_score);
    if(ret<0)
    {
        return -1;
    }

    ret = sem_init(&stDirChangeSemevent, 0, 0); //初始化信号量
    if (ret < 0)
    {
        printf("Failed to initialize semaphore: %d\n", errno);
        return -1;
    }
    initSnake(hwnd_main->hwnd);                //初始化蛇列表
    gamePic(hwnd_main->hwnd);                  //地图初始化
    

    g_hwnd_main = hwnd_main;
    g_hwnd_score = hwnd_score;
    pthread_create(&Snake_t1,NULL,refreshjiemian,nxhw_handle_set);  //创建界面刷新线程
    pthread_create(&Snake_t2,NULL,changeDir,NULL);       //创建键盘方向输入监测线程

    return 0;
}

/****************************************************************************
 * Name: snake_logic_initialize
 *
 * Description:
 *   Initialize snake_logic module.
 ****************************************************************************/

int snake_logic_initialize(void)
{
  syslog(LOG_INFO, "snake_logic initialize\n");
  return 0;
}


#ifdef __cplusplus
}
#endif
