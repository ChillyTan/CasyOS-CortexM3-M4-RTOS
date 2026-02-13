/*********************************************************************************************************
* 模块名称：Main.c
* 摘    要：主文件，包含软硬件初始化函数和main函数
* 当前版本：1.0.0
* 作    者：Chill
* 完成日期：2026年01月31日
* 内    容：
* 注    意：
*          1、注意勾选Options for Target 'Target1'->Code Generation->Use MicroLIB，否则printf无法使用
*          2、在Options for Target 'Target1'->C/C++->Optimization将编译器优化等级调整到"Level0(-O0)"
*             能规避因为编译器优化带来的Bug
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "Main.h"
#include "gd32f30x_conf.h"
#include "RCU.h"
#include "NVIC.h"
#include "Timer.h"
#include "UART0.h"
#include "LED.h"
#include "KeyOne.h"
#include "CasyOS.h"
#include "Delay.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//各任务栈空间与TCB
#if LED_TEST != 0
static u32 s_arrLED1Stack[128];
static OS_TASK_HANDLE s_structLED1Handle;
static u32 s_arrLED2Stack[128];
static OS_TASK_HANDLE s_structLED2Handle;
#endif
#if FPU_TEST != 0
static u32 s_arrFPUStack[128];
static OS_TASK_HANDLE s_structFPUHandle;
#endif
static u32 s_arrTestTaskStack1[512];
static OS_TASK_HANDLE s_structTestHandle1;
#if MEM_TEST != 0
static u32 s_arrTestTaskStack2[128];
static OS_TASK_HANDLE s_structTestHandle2;
#endif
#if SEM_TEST != 0
static u32 s_arrTestTaskStack3[128];
OS_TASK_HANDLE g_structTestHandle3;
#endif
#if Q_TEST != 0
static u32 s_arrTestTaskStack4[128];
OS_TASK_HANDLE g_structTestHandle4;
#endif
#if MUTEX_TEST != 0
static u32 s_arrTestTaskStack5[128];
static OS_TASK_HANDLE s_structTestHandle5;
static u32 s_arrTestTaskStack6[128];
static OS_TASK_HANDLE s_structTestHandle6;
#endif
#if FLAG_TEST != 0
static u32 s_arrTestTaskStack7[128];
static OS_TASK_HANDLE s_structTestHandle7;
static u32 s_arrTestTaskStack8[128];
OS_TASK_HANDLE g_structTestHandle8;
static u32 s_arrTestTaskStack9[128];
OS_TASK_HANDLE g_structTestHandle9;
static u32 s_arrTestTaskStack10[128];
OS_TASK_HANDLE g_structTestHandle10;
#endif

//CasyOS对象
#if MUTEX_TEST != 0
static OS_MUTEX s_MutexTest;
#endif
#if FLAG_TEST != 0
OS_FLAG g_FlagTest;
#endif

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void InitHardware(void);   //初始化硬件相关的模块
static void InitSoftware(void);   //初始化软件相关的模块

#if LED_TEST != 0
static void LED1Task(void);       //LED1任务
static void LED2Task(void);       //LED2任务
#endif
#if FPU_TEST != 0
static void FPUTask(void);        //浮点单元测试任务
#endif
static void TestTask1(void);			//独立按键处理任务
#if MEM_TEST != 0
static void TestTask2(void);			//内存管理组件测试任务
#endif
#if SEM_TEST != 0
static void TestTask3(void);			//内建信号量组件测试任务
#endif
#if Q_TEST != 0
static void TestTask4(void);			//内建消息队列组件测试任务
#endif
#if MUTEX_TEST != 0
static void TestTask5(void);			//互斥量组件测试任务
static void TestTask6(void);			//互斥量组件测试任务
#endif
#if FLAG_TEST != 0
static void TestTask7(void);			//事件标志组测试任务
static void TestTask8(void);			//事件标志组测试任务
static void TestTask9(void);			//事件标志组测试任务
static void TestTask10(void);			//事件标志组测试任务
#endif

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
	u32 						semSize;	//任务内建信号量大小
	u32							queSize;	//任务消息队列栈大小
}StructTaskInfo;	//用于统一注册任务

StructTaskInfo taskInfo[] =
{
#if LED_TEST != 0
	{&s_structLED1Handle, LED1Task, "LED1Task", 1, s_arrLED1Stack, sizeof(s_arrLED1Stack)/4, 0, 0},
	{&s_structLED2Handle, LED2Task, "LED2Task", 2, s_arrLED2Stack, sizeof(s_arrLED2Stack)/4, 0, 0},
#endif
#if FPU_TEST != 0
	{&s_structFPUHandle,  FPUTask,  "FPUTask",  3, s_arrFPUStack,  sizeof(s_arrFPUStack)/4,  0, 0},
#endif
	{&s_structTestHandle1, TestTask1, "TestTask1", 3, s_arrTestTaskStack1, sizeof(s_arrTestTaskStack1)/4, 5, 128},
#if MEM_TEST != 0
    {&s_structTestHandle2, TestTask2, "TestTask2", 4, s_arrTestTaskStack2, sizeof(s_arrTestTaskStack2)/4, 0, 0},
#endif
#if SEM_TEST != 0
	{&g_structTestHandle3, TestTask3, "TestTask3", 5, s_arrTestTaskStack3, sizeof(s_arrTestTaskStack3)/4, 5, 512},
#endif
#if Q_TEST != 0
	{&g_structTestHandle4, TestTask4, "TestTask4", 5, s_arrTestTaskStack4, sizeof(s_arrTestTaskStack4)/4, 5, 512},
#endif
#if MUTEX_TEST != 0
	{&s_structTestHandle5, TestTask5, "TestTask5", 5, s_arrTestTaskStack5, sizeof(s_arrTestTaskStack5)/4, 5, 512},
  {&s_structTestHandle6, TestTask6, "TestTask6", 5, s_arrTestTaskStack6, sizeof(s_arrTestTaskStack6)/4, 5, 128},
#endif
#if FLAG_TEST != 0
	{&s_structTestHandle7, TestTask7, "TestTask7", 7, s_arrTestTaskStack7, sizeof(s_arrTestTaskStack7)/4, 6, 128},
	{&g_structTestHandle8, TestTask8, "TestTask8", 7, s_arrTestTaskStack8, sizeof(s_arrTestTaskStack8)/4, 6, 128},
	{&g_structTestHandle9, TestTask9, "TestTask9", 7, s_arrTestTaskStack9, sizeof(s_arrTestTaskStack9)/4, 6, 128},
	{&g_structTestHandle10, TestTask10, "TestTask10", 7, s_arrTestTaskStack10, sizeof(s_arrTestTaskStack10)/4, 6, 128}
#endif
};

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitHardware
* 函数功能：所有的硬件相关的模块初始化函数都放在此函数中
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月23日
* 注    意：
*********************************************************************************************************/
static  void  InitHardware(void)
{ 
  InitRCU();           //初始化RCU模块
  InitNVIC();          //初始化NVIC模块
  InitUART0(115200);   //初始化UART模块
  InitTimer();         //初始化Timer模块
}

/*********************************************************************************************************
* 函数名称：InitSoftware
* 函数功能：所有的软件相关的模块初始化函数都放在此函数中
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月23日
* 注    意：
*********************************************************************************************************/
static  void  InitSoftware(void)
{
  InitLED();    //初始化LED模块
  InitKeyOne(); //初始化独立按键模块
}

#if LED_TEST != 0
/*********************************************************************************************************
* 函数名称：LED1Task
* 函数功能：LED1闪烁 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月23日
* 注    意：
*********************************************************************************************************/
static void LED1Task(void)
{
  while(1)
  {
    //PA8状态取反，实现LED1闪烁
    gpio_bit_write(GPIOA, GPIO_PIN_8, (FlagStatus)(1 - gpio_output_bit_get(GPIOA, GPIO_PIN_8)));
    OSDelay(300);
  }
}

/*********************************************************************************************************
* 函数名称：LED2Task
* 函数功能：LED2闪烁 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月23日
* 注    意：
*********************************************************************************************************/
static void LED2Task(void)
{
  while(1)
  {
    //PE6状态取反，实现LED2闪烁
    gpio_bit_write(GPIOE, GPIO_PIN_6, (FlagStatus)(1 - gpio_output_bit_get(GPIOE, GPIO_PIN_6)));
    OSDelay(700);
  }
}
#endif	//LED_TEST

#if FPU_TEST != 0
/*********************************************************************************************************
* 函数名称：FPUTask
* 函数功能：浮点单元测试
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月23日
* 注    意：
*********************************************************************************************************/
static void FPUTask(void)
{
  int a = 0;
  double b = 0;
  
  while(1)
  {
    a = a + 1;
    b = b + 0.1;
   	printf("FPUTask: a = %d, b = %.2f\r\n", a, b);
    OSDelay(1000);
  }
}
#endif	//FPU_TEST

/*********************************************************************************************************
* 函数名称：TestTask1
* 函数功能：按下按键测试CasyOS不同功能
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：
*********************************************************************************************************/
static void TestTask1(void)
{
#if MEM_TEST != 0
	void* p = NULL;
#endif	//MEM_TEST
#if Q_TEST != 0
	const char* msgList[] =
	{
			"MSG_1",
			"MSG_2",
			"MSG_3",
			"MSG_4",
			"MSG_5",
	};
#endif	//Q_TEST

	while(1)
	{
		if(ScanKeyOne(KEY_NAME_KEY1, NULL, NULL))
		{
#if MEM_TEST != 0
			p = OSMalloc(512);
			if(p != NULL)
			{
				printf("Malloc 512 Bytes OK, Addr = 0x%08x\r\n", (u32)p);
			}
			else
			{
				printf("[ERROR]: Malloc 512 Bytes Failed!\r\n");
			}
#endif	//MEM_TEST
#if SEM_TEST != 0
			printf("[Task1] Post 1 Semaphore to Task3!\r\n");
			OSSemPost(&g_structTestHandle3.sem);
#endif
#if Q_TEST != 0
		for(int i = 0; i < 5; i++)
		{
			OSQPost(&g_structTestHandle4, (u32)msgList[i]);
			printf("[Task1] Post: %s\r\n", msgList[i]);
		}
#endif	//Q_TEST
#if FLAG_TEST != 0
			printf("[Task1] Set A\r\n");
			OSFlagPost(&g_FlagTest, (1 << 0), OS_FLAG_SET);
#endif	//FLAG_TEST
		}

		if(ScanKeyOne(KEY_NAME_KEY2, NULL, NULL))
		{
#if MEM_TEST != 0
			OSFree(p);
			printf("Free 512 Bytes OK\r\n");
#endif	//MEM_TEST
#if FLAG_TEST != 0
			printf("[Task1] Set B\r\n");
			OSFlagPost(&g_FlagTest, (1 << 1), OS_FLAG_SET);
#endif	//FLAG_TEST
		}

		if(ScanKeyOne(KEY_NAME_KEY3, NULL, NULL))
		{
#if FLAG_TEST != 0
			printf("[Task1] Set A&B\r\n");
			OSFlagPost(&g_FlagTest, (1 << 0) | (1 << 1), OS_FLAG_SET);
#endif	//FLAG_TEST
		}

		OSDelay(10);
	}
}

#if MEM_TEST != 0
/*********************************************************************************************************
* 函数名称：TestTask2
* 函数功能：测试内存管理
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：每2s打印一次内存利用率
*********************************************************************************************************/
static	void	TestTask2(void)
{
	u32 used;
	while(1)
	{
		used = OSMemoryPerused();
		printf("Memory Used = %d.%d%%\r\n", used / 10, used % 10);
		OSDelay(2000);
	}
}
#endif	//MEM_TEST

#if SEM_TEST != 0
/*********************************************************************************************************
* 函数名称：TestTask3
* 函数功能：测试信号量组件
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：每 3s Pend 等待并消耗一个信号量 按下 KEY1 Post 一个信号量
*********************************************************************************************************/
static void TestTask3(void)
{
	while(1)
	{
		printf("[Task3] Pend For Sem...\r\n");
		OSSemPend(&g_structTestHandle3.sem);
		printf("[Task3] Get Sem...\r\n");
		printf("[Task3] Sem Count = %d\r\n", g_structTestHandle3.sem.count);
		OSDelay(3000);	//每3s消耗一次信号量 在这个期间内 其他任务可以多次Post信号量
	}
}
#endif	//SEM_TEST

#if Q_TEST != 0
/*********************************************************************************************************
* 函数名称：TestTask4
* 函数功能：测试内建消息队列
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：可以测试 FIFO 按下一次 KEY1 Post 五条消息
					 串口发送一次任意数据到 CasyOS 则触发一次串口接收中断 从中断 Post 一条消息到该任务
*********************************************************************************************************/
static void TestTask4(void)
{
	u32 data;
	u8 *str;

	while(1)
	{
		OSQPend(&data);
		str = (u8*)data;
		printf("[Task4] Get Message: %s\r\n", str);
		OSDelay(500);	//每500ms获取一次消息队列里的消息
	}
}
#endif	//Q_TEST

#if MUTEX_TEST != 0
/*********************************************************************************************************
* 函数名称：TestTask5 TestTask6
* 函数功能：测试互斥量
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：任务5与任务6抢占互斥量 Task5长时间占锁 Task6频繁抢锁(抢占失败) 可以测试递归锁
					 若串口信息完整且无混合穿插 则说明互斥量组件正常工作
*********************************************************************************************************/
static void TestTask5(void)
{
	u8 *str1 = "[Task5] Print Info 1\r\n";
	u8 *str2 = "[Task5] Print Info 2\r\n";
	u8 i;

	while(1)
	{
		OSMutexPend(&s_MutexTest);
		OSMutexPend(&s_MutexTest);	//Task5递归获取2次锁

    //循环打印字符串1 使用软件延时模拟打印大量信息消耗时间很长
    i = 0;
    while(0 != str1[i])
    {
      printf("%c", str1[i]);
      OSDelay(100);
      i++;
    }

		OSMutexPost(&s_MutexTest);	//第一次解锁(不完全释放)

		//循环打印字符串2 使用软件延时模拟打印大量信息消耗时间很长
    i = 0;
    while(0 != str2[i])
    {
      printf("%c", str2[i]);
      OSDelay(100);
      i++;
    }

		OSMutexPost(&s_MutexTest);	//第二次解锁(完全释放)
		OSDelay(500);
	}
}

static void TestTask6(void)
{
	while(1)
	{
		OSMutexPend(&s_MutexTest);

		printf("[Task6] Got Mutex\r\n");

		OSMutexPost(&s_MutexTest);
		OSDelay(100);
	}
}
#endif	//MUTEX_TEST

#if FLAG_TEST != 0
/*********************************************************************************************************
* 函数名称：TestTask7 TestTask8
* 函数功能：测试事件标志组
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：按KEY1释放标志位A 按KEY2释放标志位B 按KEY3释放标志位A&B 串口接收中断发送标志位A&B&C
					 Task7唤醒条件 A|B; Task8唤醒条件 A&B; Task9唤醒条件 A&B&C;
					 任务每5s获取一次标志位 始终消耗事件标志位
*********************************************************************************************************/
static void TestTask7(void)
{
	while(1)
	{
		OSFlagPend(&g_FlagTest, (1 << 0) | (1 << 1), OS_FLAG_WAIT_SET_ANY);
		printf("[Task7] Wakeup! (A|B matched, flags consumed)\r\n");
		OSDelay(5000);
	}
}

static void TestTask8(void)
{
	while(1)
	{
		OSFlagPend(&g_FlagTest, (1 << 0) | (1 << 1), OS_FLAG_WAIT_SET_ALL);
		printf("[Task8] Wakeup! (A&B matched, flags consumed)\r\n");
		OSDelay(5000);
	}
}

static void TestTask9(void)
{
	while(1)
	{
		OSFlagPend(&g_FlagTest, (1 << 0) | (1 << 1) | (1 << 2), OS_FLAG_WAIT_SET_ALL);
		printf("[Task9] Wakeup! (A&B&C matched, flags consumed)\r\n");
		OSDelay(5000);
	}
}

/*********************************************************************************************************
* 函数名称：TestTask10
* 函数功能：测试事件标志组
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：每1s打印一次事件标志组值(十六进制)
*********************************************************************************************************/
static void TestTask10(void)
{
	while(1)
	{
		printf("g_FlagTest = 0x%08x\r\n", g_FlagTest.flags);
		OSDelay(1000);
	}
}
#endif	//FLAG_TEST

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：main
* 函数功能：主函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：int
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
int main(void)
{
  InitHardware();   //初始化硬件相关函数
  InitSoftware();   //初始化软件相关函数
	InitCasyOS();			//初始化CasyOS
  printf("Init System has been finished\r\n");
	
  u8 i;
	u32 err;
	
#if MUTEX_TEST != 0
	OSMutexCreate(&s_MutexTest, (u8*)"TestMutex");	//创建互斥量
#endif
#if	FLAG_TEST != 0
	OSFlagCreate(&g_FlagTest, (u8*)"TestFlag", 0);	//创建事件标志组
#endif
	
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
}
