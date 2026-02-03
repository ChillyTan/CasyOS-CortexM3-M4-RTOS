/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "Main.h"
#include "stm32f10x_conf.h"
#include "DataType.h"
#include "CasyOS.h"

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static u32 s_arrTask1Stack[128];
static OS_TASK_HANDLE s_structTask1Handle;
static u32 s_arrTask2Stack[128];
static OS_TASK_HANDLE s_structTask2Handle;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void Task1(void);
static void Task2(void);

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
typedef struct StructTaskInfo
{
	OS_TASK_HANDLE* handle;		//任务句柄
	void*						func;			//任务函数
	char*						name;			//任务名称
	u32							prio;			//任务优先级
	u32*						stkBase;	//任务栈首地址
	u32							stkSize;	//任务栈大小
	u32 						semSize;	//任务内建信号量大小(不需要可以填0)
	u32							queSize;	//任务消息队列栈大小(不需要可以填0)
}StructTaskInfo;	//用于统一注册任务

StructTaskInfo taskInfo[] =
{
	{&s_structTask1Handle, Task1, "Task1", 1, s_arrTask1Stack, sizeof(s_arrTask1Stack)/4, 0, 0},
	{&s_structTask2Handle, Task2, "Task2", 2, s_arrTask2Stack, sizeof(s_arrTask2Stack)/4, 0, 0},
};

/*********************************************************************************************************
* 函数名称：Task1
* 函数功能：任务1：500ms打印一次
*********************************************************************************************************/
static void Task1(void)
{
	while(1)
	{
		printf("Task1 running\r\n");
		OSDelay(500);
	}
}

/*********************************************************************************************************
* 函数名称：Task2
* 函数功能：任务2：1000ms打印一次
*********************************************************************************************************/
static void Task2(void)
{
	while(1)
	{
		printf("Task2 running\r\n");
		OSDelay(1000);
	}
}

/*********************************************************************************************************
* 函数名称：main
* 函数功能：主函数
*********************************************************************************************************/
int main(void)
{
	InitUART1(115200);
  printf("Init System has been finished\r\n");
	
	//初始化CasyOS
	InitCasyOS();
	
  u8 i;
	u32 err;
	
	//创建任务
  for(i = 0; i < sizeof(taskInfo) / sizeof(StructTaskInfo); i++)
  {
		err = OSRegister(taskInfo[i].handle,
							 taskInfo[i].func,
							 taskInfo[i].name,
							 taskInfo[i].prio,
							 taskInfo[i].stkBase,
							 taskInfo[i].stkSize,
							 taskInfo[i].semSize,
							 taskInfo[i].queSize);

    //校验
    if(err != 0)
    {
      printf("Fail to create %s (%d)\r\n", taskInfo[i].name, err);
      while(1){}
    }
  }
	
	//启动CasyOS
	OSStart();
	while(1){}	//不应该执行到这里
}
