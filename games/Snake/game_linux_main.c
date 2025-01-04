#include <curses.h>
#include <stdlib.h>
#include <pthread.h>
#define  UP     1
#define  DOWN  -1
#define  LEFT   2
#define  RIGHT -2

/*蛇和食物的结构体*/
struct Snake
{
    int hang;
    int lie;
    struct Snake *next;
};

struct Snake *head = NULL;      //链表头
struct Snake *tail = NULL;        //链表尾
struct Snake food;              //食物
int key;                        //记录键盘输入值
int dir;                        //记录输入的方向键

/*输入横坐标和纵坐标值，判断是否存在蛇的链表节点*/
int hasSnakeNode(int i,int j)
{
    struct Snake *p;            //临时变量，用于记录蛇的链表表头
    p = head;

    while(p != NULL)            //当链表头不为空时，进入循环
    {
        if(p->hang == i && p->lie == j)
        {
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
    x = rand()%20;             //在[0-20]范围内随机获取横坐标值
    y = rand()%20;             //在[0-20]范围内随机获取纵坐标值

    if(y == 0)
    {
        y = rand()%20;          //当纵坐标取值为0时，重新取值
    }

    food.hang = x;             //将前面获取到的横坐标赋值为食物的横坐标
    food.lie = y;               //将前面获取到的纵坐标复制为食物的纵坐标
}

/*输入横纵坐标判断是否存在食物，用于地图刷新*/
int hasFoodnode(int i,int j)
{
    if((food.hang == i)&&(food.lie == j))
    {
        return 1;               //当输入横纵坐标为食物的横纵坐标时，返回1
    }
    return 0;                   //当输入横纵坐标不是食物的横纵坐标时，返回0
}

/*地图绘制*/
void gamePic()
{
    int hang,lie;              //行列临时变量
    move(0,0);                 //每次界面刷新都将界面光标移动到(0,0)的位置
    for(hang=0;hang<20;hang++) //历遍行
    {
        if(hang==0)
        {
            for(lie=0;lie<20;lie++)
            {
                printw("--");  //判断为第0行时，绘制‘--’边界符号
            }
            printw("\n");
        }
        if(hang>=0 || hang<=19)
        {
            for(lie=0;lie<=20;lie++)
            {
                if(lie==0 || lie==20)
                {
                    printw("|");//第0和19列绘制‘|’边界符号
                }
                else if(hasSnakeNode(hang,lie))
                {
                    printw("[]");//行列值满足蛇的节点坐标时，绘制‘[]’符号
                }
                else if(hasFoodnode(hang,lie))
                {
                    printw("##");//行列之满足食物节点坐标时，绘制‘##’符号
                }
                else
                {
                    printw("  ");//地图上空闲位置绘制‘  ’空格符号
                }
            }
            printw("\n");        //每绘制完一行，该处添加一处换行符
        }
        if(hang==19)
        {
            for(lie=0;lie<20;lie++)
            {
                printw("--");   //判断为第19行时，绘制‘--’边界符号
            }
            printw("\n");
            printw("By chenguanxiong\n");
            printw("%d",key);
        }
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
            new->hang = tail->hang - 1;  //向上，行减1，上移
            new->lie = tail->lie;        //列保持
            break;
        case DOWN:
            new->hang = tail->hang + 1;  //向下，行加1，下移
            new->lie = tail->lie;        //列保持不变
            break;
        case LEFT:
            new->hang = tail->hang;      //行保持不变
            new->lie = tail->lie - 1;    //向左，列减1，左移
            break;
        case RIGHT:
            new->hang = tail->hang;      //行保持不变
            new->lie = tail->lie + 1;    //向右，列加1，右移
            break;
        default:
            break;

    }

    new->next = NULL;           //新节点的下一个节点指向为NULL
    tail->next = new;           //尾部的下一个节点指向新节点
    tail = new;                 //新节点复制给尾部节点
}

/*初始化蛇*/
void initSnake()
{
    struct Snake *p;            //临时变量，指向蛇的链表头
    dir = RIGHT;                //运动方向初始化为向右
    while(head != NULL)         //当链表头不为空时进入，用于释放蛇当前的链表占用内存空间
    {
        p = head;               //p指向链表头
        head = head->next;      //链表头指向下一个节点
        free(p);                //释放链表头内存
    }

    initFoodnode();             //初始化食物
    head = (struct Snake *)malloc(sizeof(struct Snake)); //为链表头开辟新的内存空间
    head->hang = 2;             //链表头行初始值为2
    head->lie = 2;              //链表头列初始值为2
    head->next = NULL;          //链表头的下一个节点指向为NULL
    tail = head;                //链表尾指向链表头

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

    if(tail->hang<0 | tail->lie==0 | tail->hang==20 | tail->lie==20)
    {
        return 1;               //当蛇链表的尾部坐标等于边界值时，返回1
    }

    while(p->next != NULL)
    {
        if((p->hang==tail->hang)&&(p->lie==tail->lie))
        {
            return 1;           //当蛇链表其它的节点与尾部节点坐标相同，返回1
        }
        p = p->next;
    }

    return 0;                   //无越界，无自残，返回0
}

/*蛇移动*/
void moveSnake()
{
    addNode();                  //添加新节点
    if(hasFoodnode(tail->hang,tail->lie))
    {
        initFoodnode();         //当蛇链表尾节点坐标值和食物坐标值一样，刷新食物位置
    }
    else
    {
        deleteNode();           //否则，删除蛇链表中的头节点
    }

    if(ifSnakedie())
    {
        initSnake();            //如果满足越界或者自残条件，重新初始化蛇链表
    }
}

/*地图界面刷新线程函数*/
void* refreshjiemian()
{
    while(1)
    {
        moveSnake();           //蛇链表移动
        gamePic();             //地图刷新
        refresh();             //执行刷新
        usleep(150000);           //线程休眠函数，150ms
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

/*键盘方向输入监测线程函数*/
void* changeDir()
{
    while(1)
    {
        key = getch();          //获取键盘输入
        switch(key)
        {
            case KEY_UP:
                turn(UP);       //上
                break;
            case KEY_DOWN:
                turn(DOWN);     //下
                break;
            case KEY_LEFT:
                turn(LEFT);     //左
                break;
            case KEY_RIGHT:
                turn(RIGHT);    //右
                break;
            default:
                break;
        }
    }
}

int main()
{
    pthread_t t1;               //创建线程变量t1
    pthread_t t2;               //创建线程变量t2

    initscr();                  //Ncurses初始化
    keypad(stdscr,1);           //在std中接受键盘的功能键
    noecho();                   //控制键盘输入进来的字符

    initSnake();                //初始化蛇列表
    gamePic();                  //地图初始化

    pthread_create(&t1,NULL,refreshjiemian,NULL);  //创建界面刷新线程
    pthread_create(&t2,NULL,changeDir,NULL);       //创建键盘方向输入监测线程

    while(1);

    getch();
    endwin();

    return 0;
}