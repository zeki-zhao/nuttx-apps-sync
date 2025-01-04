// #include <curses.h>
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
#include "../examples/nxhello/nx2Snake.h"

#define PIXEL_SIZE (3) 


#define UP_BOUNDARY (40)
#define DOWN_BOUNDARY (480-10)
#define LEFT_BOUNDARY (40)
#define RIGHT_BOUNDARY (800-200)

#define BUTTON_SIZE (50)
#define UP_BUTTON_LOCATION_X (675)
#define UP_BUTTON_LOCATION_Y (310)
#define DOWN_BUTTON_LOCATION_X (675)
#define DOWN_BUTTON_LOCATION_Y (410) 
#define LEFT_BUTTON_LOCATION_X (625) 
#define LEFT_BUTTON_LOCATION_Y (360)
#define RIGHT_BUTTON_LOCATION_X (725)
#define RIGHT_BUTTON_LOCATION_Y (360)


#define  UP     1
#define  DOWN  -1
#define  LEFT   2
#define  RIGHT -2

/*蛇和食物的结构体*/
struct Snake
{
    int x;  //x位置
    int y; //y位置 
    struct Snake *next;
};

enum{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    ALL_KEY
};

static struct Snake *head = NULL;      //链表头，蛇尾
static struct Snake *tail = NULL;        //链表尾，蛇头
struct Snake food;              //食物
int key;                        //记录键盘输入值
int dir;                        //记录输入的方向键


///////////////////////////////////Snake/////////////////////////////////////////

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
void initFoodnode()
{
    int x;                     //定义横坐标临时变量
    int y;                     //定义纵坐标临时变量
    x = LEFT_BOUNDARY+rand()%RIGHT_BOUNDARY;            //在范围内随机获取横坐标值
    y = UP_BOUNDARY+rand()%DOWN_BOUNDARY;             //在范围内随机获取纵坐标值

    // if(y == 0)
    // {
    //     y = rand()%20;          //当纵坐标取值为0时，重新取值
    // }

    food.x = x;       
    food.y = y;   
    
    printf("food x:%d, y:%d \n",food.x,food.y);
}

/*输入横纵坐标判断是否存在食物，用于地图刷新*/
int hasFoodnode(int i,int j)
{
    if((food.y == i)&&(food.x == j))
    {
        return 1;               //当输入横纵坐标为食物的横纵坐标时，返回1
    }
    return 0;                   //当输入横纵坐标不是食物的横纵坐标时，返回0
}

/* 地图绘制 */
void gamePic(NXEGWINDOW *hwnd)
{
    int y,x;              //行列临时变量
    
    /*绘制活动空间  */
    Snake_draw_rectangle(hwnd,LEFT_BOUNDARY,UP_BOUNDARY,RIGHT_BOUNDARY,DOWN_BOUNDARY,0x07e2);

    /* 上键 */
    Snake_draw_squares(hwnd,UP_BUTTON_LOCATION_X,UP_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_TBCOLOR);
    Snake_draw_squares(hwnd,DOWN_BUTTON_LOCATION_X,DOWN_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_TBCOLOR);
    Snake_draw_squares(hwnd,LEFT_BUTTON_LOCATION_X,LEFT_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_TBCOLOR);
    Snake_draw_squares(hwnd,RIGHT_BUTTON_LOCATION_X,RIGHT_BUTTON_LOCATION_Y,BUTTON_SIZE,CONFIG_EXAMPLES_NX_TBCOLOR);

    for(y=UP_BOUNDARY; y<=DOWN_BOUNDARY; y++) //历遍上下边界
    {
        if(y>=UP_BOUNDARY || y<=DOWN_BOUNDARY)
        {
            for(x=LEFT_BOUNDARY;x<=RIGHT_BOUNDARY;x++) //历遍左右边界
            {
                if(x==LEFT_BOUNDARY || x==RIGHT_BOUNDARY) //画边界
                {

                    Snake_draw_squares(hwnd,x,y,2,CONFIG_EXAMPLES_NX_TBCOLOR);
                    // printw("|");//第0和19列绘制‘|’边界符号
                }
                else if(hasSnakeNode(y,x))
                {
                    Snake_draw_squares(hwnd,x-1,y-1,3,CONFIG_EXAMPLES_NX_COLOR2);
                    printf("in %s :%d\n",__FILE__,__LINE__);
                    // printw("[]");//行列值满足蛇的节点坐标时，绘制‘[]’符号
                }
                else if(hasFoodnode(y,x))
                {
                    Snake_draw_squares(hwnd,x-1,y-1,3,CONFIG_EXAMPLES_NX_COLOR2);
                    printf("in %s :%d\n",__FILE__,__LINE__);
                //     // printw("##");//行列之满足食物节点坐标时，绘制‘##’符号
                }
                // else
                // {
                //     Snake_draw_squares(hwnd,x-1,y-1,3,0x07e2);
                // }
            }
        }
        if(y==UP_BOUNDARY || y==DOWN_BOUNDARY)
        {
            for(x=LEFT_BOUNDARY; x<=RIGHT_BOUNDARY; x++)
            {
                Snake_draw_squares(hwnd,x,y,2,CONFIG_EXAMPLES_NX_TBCOLOR); //判断为第19行时，绘制‘--’边界符号
            }
        }
    }
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

//TODO：无需做地图，只需限定地图范围
void Snake_draw_squares(NXEGWINDOW *hwnd,int16_t x,int16_t y,int16_t size, uint16_t color)
{
    // printf("in %s :%d\n",__FILE__,__LINE__);
    int ret;
    struct nxgl_rect_s TempRect;
    TempRect.pt1.x = x;
    TempRect.pt1.y = y;
    TempRect.pt2.x = x+size;
    TempRect.pt2.y = y+size;
    ret = nx_fill(hwnd, &TempRect, color);
    if (ret < 0)
    {
        printf("nx_main: nx_fill failed: %d\n", errno);
    }
}

void Snake_draw_rectangle(NXEGWINDOW *hwnd,int16_t x1,int16_t y1,int16_t x2, int16_t y2, uint16_t color)
{
    // printf("in %s :%d\n",__FILE__,__LINE__);
    int ret;
    struct nxgl_rect_s TempRect;
    TempRect.pt1.x = x1;
    TempRect.pt1.y = y1;
    TempRect.pt2.x = x2;
    TempRect.pt2.y = y2;
    ret = nx_fill(hwnd, &TempRect, color);
    if (ret < 0)
    {
        printf("nx_main: nx_fill failed: %d\n", errno);
    }
}

/*添加蛇身节点*/
void addNode()
{
    struct Snake *new;         //新节点变量
    new = (struct Snake *)malloc(sizeof(struct Snake));//为新节点开辟内存空间

    switch(dir)                //方向键判断
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

/*初始化蛇*/
void initSnake(NXEGWINDOW *hwnd)
{
    struct Snake *p;            //临时变量，指向蛇的链表头
    dir = RIGHT;                //运动方向初始化为向右

    // while(head != NULL)         //当链表头不为空时进入，用于释放蛇当前的链表占用内存空间
    // {
    //     p = head;               //p指向链表头
    //     head = head->next;      //链表头指向下一个节点
    //     Snake_draw_squares(hwnd,(p->x)-1,(p->y)-1,3,0x07e2); //清空原有蛇的颜色   
    //     free(p);                //释放链表头内存
    // }

    initFoodnode();             //初始化食物
    head = (struct Snake *)malloc(sizeof(struct Snake)); //为链表头开辟新的内存空间
    head->y = UP_BOUNDARY+PIXEL_SIZE*3;             //链表头行初始值为2
    head->x = LEFT_BOUNDARY+PIXEL_SIZE*3;              //链表头列初始值为2
    head->next = NULL;          //链表头的下一个节点指向为NULL
    tail = head;                //链表尾指向链表头
    printf("in %s :%d\n",__FILE__,__LINE__);

    addNode();                  //为链表添加新节点
    addNode();
    addNode();
    addNode();
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

    if(tail->y<=UP_BOUNDARY | tail->x<=LEFT_BOUNDARY | tail->y>=DOWN_BOUNDARY | tail->x>=RIGHT_BOUNDARY)
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

/*蛇移动*/
void moveSnake(NXEGWINDOW *hwnd)
{
    addNode();                  //添加新节点,tail是蛇的头部
    Snake_draw_squares(hwnd,(tail->x)-1,(tail->y)-1,3,CONFIG_EXAMPLES_NX_COLOR2); /* 头部新加色块 */
    if(hasFoodnode(tail->y,tail->x)) /* 吃到食物了，不删蛇尾 */
    {
        initFoodnode();         //当蛇链表尾节点坐标值和食物坐标值一样，刷新食物位置
    }
    else
    {
        Snake_draw_squares(hwnd,(head->x)-1,(head->y)-1,3,0x07e2); /* 尾部恢复背景色 */
        deleteNode();           //否则，删除蛇链表中的头节点
    }

    if(ifSnakedie())//TODO:死亡检测
    {

        initSnake(hwnd);            //如果满足越界或者自残条件，重新初始化蛇链表
    }
}

/*地图界面刷新线程函数*/
void* refreshjiemian(void *arg )
{
    static int temp = 0;
    NXEGWINDOW *hwnd = (NXEGWINDOW *)arg;
    // printf("in %s :%d\n",__FILE__,__LINE__);
    while(1)
    {
        moveSnake(hwnd);           //蛇链表移动
        // gamePic(hwnd);             //地图刷新

        // printf("in %s :%d\n",__FILE__,__LINE__);

        // if(temp++>10)
        // {
        //     dir = DOWN;
        // }
        usleep(200000);           //线程休眠函数，150ms
    } 
}

/*方向函数*/
void turn(int direction)
{
    if(abs(dir) != abs(direction))
    {
        dir = direction;        //方向取绝对值比较，当左右运动时只有上下输入才生效
    }
}

int Snake_Get_Dir(void)
{
    if((Snake_Touch.point->x > UP_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < UP_BUTTON_LOCATION_X + BUTTON_SIZE) && 
        (Snake_Touch.point->y > UP_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < UP_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        printf("in %s :%d\n",__FILE__,__LINE__);
        printf("X: %d Y:%d\n",Snake_Touch.point->x,Snake_Touch.point->y);
        usleep(100000);           //线程休眠函数，150ms
        return KEY_UP; 
    }

    if((Snake_Touch.point->x > DOWN_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < DOWN_BUTTON_LOCATION_X + BUTTON_SIZE)&& 
        (Snake_Touch.point->y > DOWN_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < DOWN_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        printf("in %s :%d\n",__FILE__,__LINE__);
        usleep(100000);           //线程休眠函数，150ms
        return KEY_DOWN; 
    }

    if((Snake_Touch.point->x > LEFT_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < LEFT_BUTTON_LOCATION_X + BUTTON_SIZE)&& 
        (Snake_Touch.point->y > LEFT_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < LEFT_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        printf("in %s :%d\n",__FILE__,__LINE__);
        usleep(100000);           //线程休眠函数，150ms
        return KEY_LEFT; 
    }

    if((Snake_Touch.point->x > RIGHT_BUTTON_LOCATION_X) && 
        (Snake_Touch.point->x < RIGHT_BUTTON_LOCATION_X + BUTTON_SIZE)&& 
        (Snake_Touch.point->y > RIGHT_BUTTON_LOCATION_Y) && 
        (Snake_Touch.point->y < RIGHT_BUTTON_LOCATION_Y + BUTTON_SIZE))
    {
        printf("in %s :%d\n",__FILE__,__LINE__);
        usleep(100000);           //线程休眠函数，150ms
        return KEY_RIGHT; 
    }    

    return -1;
}

/*键盘方向输入监测线程函数*/
void* changeDir()
{
    while(1)
    {
        key = Snake_Get_Dir();          //获取触控输入
        switch(key)
        {
            case KEY_UP:
            printf("in %s :%d\n",__FILE__,__LINE__);
                turn(UP);       //上
                break;
            case KEY_DOWN:
            printf("in %s :%d\n",__FILE__,__LINE__);
                turn(DOWN);     //下
                break;
            case KEY_LEFT:
            printf("in %s :%d\n",__FILE__,__LINE__);
                turn(LEFT);     //左
                break;
            case KEY_RIGHT:
            printf("in %s :%d\n",__FILE__,__LINE__);
                turn(RIGHT);    //右
                break;
            default:
                break;
        }
    }
}
 
int snake(NXEGWINDOW *hwnd)
{
    pthread_t t1;               //创建线程变量t1
    pthread_t t2;               //创建线程变量t2

    // // // printf("in %s :%d\n",__FILE__,__LINE__);   

    initSnake(hwnd);                //初始化蛇列表
    gamePic(hwnd);                  //地图初始化 //todo:这个有问题？ 好像是的 需要使用这个函数中的边界

    // Snake_draw_squares(hwnd,100,200,50,0x07e0);

    
    // // // printf("in %s :%d\n",__FILE__,__LINE__);

    pthread_create(&t1,NULL,refreshjiemian,hwnd);  //创建界面刷新线程
    pthread_create(&t2,NULL,changeDir,NULL);       //创建键盘方向输入监测线程

    // getch();
    // endwin();

    return 0;
}
