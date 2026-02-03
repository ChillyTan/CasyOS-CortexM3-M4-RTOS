/*********************************************************************************************************
* 模块名称: os_task.c
* 摘    要: 任务管理模块
* 当前版本: 1.0.0
* 作    者: Chill
* 完成日期: 2026年01月22日 
* 内    容:
*   				(1) 维护全局任务链表(用于遍历/查找任务)
*   				(2) 维护就绪队列组(按优先级划分，每级一个FIFO队列, prio数值越小优先级越高)
*   				(3) 维护优先级位图(用于O(1)定位最高优先级就绪任务)
*   				(4) 维护延时队列(增量tick链表，用于OS_Delay/超时管理)
*   				(5) 维护对象挂起队列(信号量/互斥量/消息队列等等待队列)
* 设计说明:
*   				- 优先级模型:
*       				OS_CFG_PRIO_MAX 个优先级(0~OS_CFG_PRIO_MAX-1)
*       				本工程中: prio数值越小优先级越高(典型RTOS设计)
*       				但位图中为了配合 __builtin_clz()，做了 bit反转映射:
*           		bit_index = 31 - prio
*
*   				- 就绪队列模型:
*       				s_OSRdyLists[prio] 是一个双向链表队列
*       				同优先级任务采用FCFS策略(尾插入，头取出)
*
*   				- 延时队列模型(增量链表):
*       				tick链表中的每个结点保存的是“相对延时”
*       				head->tick 每次SysTick减1，减到0则唤醒该任务
*       				插入/删除需要维护相对tick的差值，保证遍历效率
*
*   				- 挂起队列模型:
*       				OS_PEND_LIST 用于对象等待队列(按优先级排序)
*       				Insert时按优先级插入，GetHighest直接返回头结点
* 注    意:           
*   				(1) 任务切换相关变量(如g_pCurrentTask)必须在临界区内修改
*   				(2) 就绪队列/延时队列/挂起队列的操作必须保证原子性
*   				(3) __builtin_clz(0) 是未定义行为，因此位图为0时必须拦截
**********************************************************************************************************
* 取代版本: 
* 作    者:
* 完成日期: 
* 修改内容:
* 修改文件: 
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "CasyOS.h"
#include <stdint.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量定义
*********************************************************************************************************/
OS_TASK_HANDLE g_OSTaskListHead;	//全局任务单链表(仅用于管理/查找 不参与调度)
static OS_RDY_LIST s_OSRdyLists[OS_CFG_PRIO_MAX];	//就绪列表组(每个优先级一个双向链表)
static u32 s_OSPrioBitMap;	//优先级位图 0-空 1-非空 共32位即32个优先级 bit[0]优先级最低 bit[31]优先级最高
//TODO: 可以扩展为数组(支持更多优先级)
static OS_TICK_LIST s_OSTickList;	//增量列表
extern OS_TASK_HANDLE* g_pCurrentTask;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: StrCaseCmp
* 函数功能: 判断两个字符串是否相等(忽略大小写)
* 输入参数: str1, str2 两个字符串
* 输出参数: void
* 返 回 值: 0-相等 else-不相等
* 创建日期: 2026年01月22日
* 注    意: 忽略大小写 内部辅助函数 OS_TaskFind() 按任务名查找时使用
*********************************************************************************************************/
static int StrCaseCmp(char *s1, char *s2)
{
	char c1, c2;

	while(*s1 && *s2)
	{
		c1 = (*s1 >= 'A' && *s1 <= 'Z') ? (*s1 + 32) : *s1;
		c2 = (*s2 >= 'A' && *s2 <= 'Z') ? (*s2 + 32) : *s2;

		if (c1 != c2) return c1 - c2;

		s1++;
		s2++;
	}

	return *s1 - *s2;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: OSInitTaskSched
* 函数功能: 初始化任务调度模块
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 需要用户在初始化CasyOS时调用
*********************************************************************************************************/
void OSInitTaskSched(void)
{
	OS_ENTER_CRITICAL();

	OS_TaskListInit();	//初始化全局任务列表
	OS_RdyListInit();		//初始化就绪列表
	OS_TickListInit();	//初始化延时列表
	OS_BitMapInit();		//初始化优先级位图

	OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
* 函数名称: OSTaskFind
* 函数功能: 按照不同类型信息查找任务
* 输入参数: opt: 查找选项(可选任务名/任务函数首地址) 查询关键字
*   			 - OS_TASK_ID_TCB : 直接返回TCB指针
*   			 - OS_TASK_ID_NAME: 按任务名查找(需使能OS_CFG_TASK_NAME_EN)
*   			 - OS_TASK_ID_FUNC: 按任务入口函数地址查找(需使能OS_CFG_TASK_FUNC_EN)
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 允许用户用这个函数查找任务
*********************************************************************************************************/
OS_TASK_HANDLE* OSTaskFind(OS_TASK_ID_TYPE opt, void* key)
{
	OS_TASK_HANDLE *p_tcb = &g_OSTaskListHead;
	
	switch (opt)
	{
		case OS_TASK_ID_TCB:
			return (OS_TASK_HANDLE *)key;

#if OS_CFG_TASK_NAME_EN
		case OS_TASK_ID_NAME:	//用任务名称查找
			//在全局任务单链表中遍历查找
			while(p_tcb != NULL)
			{
				if(StrCaseCmp(p_tcb->taskName, (char*)key) == 0)
				{
					return p_tcb;
				}
				p_tcb = p_tcb->nextPtr;
			}
			break;
#endif	//OS_CFG_TASK_NAME_EN

#if OS_CFG_TASK_FUNC_EN
		case OS_TASK_ID_FUNC:	//用任务函数首地址查找
			while(p_tcb != NULL)
			{
				if(p_tcb->func == key)
				{
					return p_tcb;
				}
				p_tcb = p_tcb->nextPtr;
			}
			break;
#endif	//OS_CFG_TASK_FUNC_EN

			default:
				break;
	}

	printf("Warning: Cannot Find Task!\r\n");
	return NULL;
}

/*********************************************************************************************************
* 函数名称: OS_UpdateCurrentTask
* 函数功能: 从就绪列表中选出优先级最高的任务 更新到g_pCurrentTask
* 输入参数: void
* 输出参数: void
* 返 回 值: 任务句柄指针
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*						本函数仅“选择任务”，实际切换通常由PendSV完成
*********************************************************************************************************/
void OS_UpdateCurrentTask(void)
{
	u32 prio;

	OS_ENTER_CRITICAL();

	//位图找到优先级最高的任务
	prio = OS_BitMapGetHighest();
	//返回到PendSV部分以切换任务
	g_pCurrentTask = s_OSRdyLists[prio].headPtr;

	OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
* 函数名称: OS_TaskListInit
* 函数功能: 初始化全局任务列表
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_TaskListInit(void)
{
	//哨兵结点(head) 不存储真实任务
	g_OSTaskListHead.nextPtr = NULL;
}

/*********************************************************************************************************
* 函数名称: OS_TaskListInit
* 函数功能: 添加任务到全局任务列表末尾
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*						该链表不参与调度，仅用于OS_TaskFind/调试统计
*********************************************************************************************************/
void OS_TaskListAdd(OS_TASK_HANDLE *p_tcb)
{
	OS_TASK_HANDLE* pCurrTask;	//遍历单链表索引
	
	//输入参数检查
	if(p_tcb == NULL)
	{
		return;
	}
	
	pCurrTask = &g_OSTaskListHead;
	while(pCurrTask->nextPtr != NULL)
	{
		pCurrTask = pCurrTask->nextPtr;
	}
	pCurrTask->nextPtr = p_tcb;
	p_tcb->nextPtr = NULL;
}

/*********************************************************************************************************
* 函数名称: OS_TaskListRemove
* 函数功能: 从全局任务列表中删除任务
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*						该链表不参与调度，仅用于OS_TaskFind/调试统计
*********************************************************************************************************/
void OS_TaskListRemove(OS_TASK_HANDLE *p_tcb)
{
	OS_TASK_HANDLE* pPrevTask;	//需要删除任务的前驱结点
	OS_TASK_HANDLE* pCurrTask;	//需要删除的任务结点
	
	//输入参数检查
	if(p_tcb == NULL)
	{
		return;
	}
	
	pPrevTask = &g_OSTaskListHead;
	while(pPrevTask->nextPtr != NULL)
	{
		if(pPrevTask->nextPtr == p_tcb)	//找到前驱结点
		{
			//从单链表中删除
			pCurrTask = pPrevTask->nextPtr;
			pPrevTask->nextPtr = pCurrTask->nextPtr;
			pCurrTask->nextPtr = NULL;
			return;
		}
		pCurrTask = pCurrTask->nextPtr;
	}
	
	//遍历到最后还没找到该任务
	printf("Warning: Cannot Find Task!\r\n");
}

/*********************************************************************************************************
* 函数名称: OS_RdyListInit
* 函数功能: 初始化所有就绪列表
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_RdyListInit(void)
{
	u8 i;

	for (i = 0; i < OS_CFG_PRIO_MAX; i++)
	{
		s_OSRdyLists[i].headPtr = NULL;
		s_OSRdyLists[i].tailPtr = NULL;
	}
}

/*********************************************************************************************************
* 函数名称: OS_RdyTaskAdd
* 函数功能: 在就绪列表中加入一个任务到末尾
* 输入参数: 任务句柄
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 只能加入到某个列表的末尾 同优先级就绪任务遵循FCFS原则
*********************************************************************************************************/
void OS_RdyTaskAdd(OS_TASK_HANDLE *p_tcb)
{
	OS_RDY_LIST *pRdyList;
	u32 prio;

	//输入参数检查
	if(p_tcb == NULL)
	{
		return;
	}

	//从就绪列表组中找到对应该任务优先级的就绪列表
	prio = p_tcb->priority;
	pRdyList = &s_OSRdyLists[prio];

	//插入到链表末尾
	p_tcb->rdyNextPtr = NULL;
	if(pRdyList->headPtr == NULL)	//该就绪列表空
	{
		p_tcb->rdyPrevPtr = NULL;
		pRdyList->headPtr = p_tcb;
	}
	else	//该就绪列表非空
	{
		p_tcb->rdyPrevPtr = pRdyList->tailPtr;
		pRdyList->tailPtr->rdyNextPtr = p_tcb;
	}
	pRdyList->tailPtr = p_tcb;	//无论哪个情况 尾指针都指向新加进来的任务

	//位图更新 置位该列表的优先级位图
	OS_BitMapSet(prio);
}

/*********************************************************************************************************
* 函数名称: OS_RdyTaskRemove
* 函数功能: 将任务从就绪列表中移除
* 输入参数: 任务句柄
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 可能从列表的任意位置删除
						非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_RdyTaskRemove(OS_TASK_HANDLE *p_tcb)
{
	OS_RDY_LIST *pRdyList;
	u32 prio;

	//输入参数检查
	if(p_tcb == NULL)
	{
		return;
	}

	//从就绪列表组中找到对应该任务优先级的就绪列表
	prio = p_tcb->priority;
	pRdyList = &s_OSRdyLists[prio];

	//从列表开头移除
	if(p_tcb->rdyPrevPtr == NULL)	//在头结点
	{
		pRdyList->headPtr = p_tcb->rdyNextPtr;	//就绪列表头指针指向下一个结点
		if(p_tcb->rdyNextPtr != NULL)	//如果后面还有结点
		{
			p_tcb->rdyNextPtr->rdyPrevPtr = NULL;	//下一个结点变成头结点
		}
	}
	else if(p_tcb->rdyNextPtr == NULL)//在尾结点
	{
		pRdyList->tailPtr = p_tcb->rdyPrevPtr;	//就绪列表尾指针指向上一个结点
		p_tcb->rdyPrevPtr->rdyNextPtr = NULL;	//上一个结点变成尾结点
	}
	else	//在中间结点
	{
		p_tcb->rdyPrevPtr->rdyNextPtr = p_tcb->rdyNextPtr;
		p_tcb->rdyNextPtr->rdyPrevPtr = p_tcb->rdyPrevPtr;
	}
	
	p_tcb->rdyNextPtr = NULL;
	p_tcb->rdyPrevPtr = NULL;

	//位图更新 如果该就绪列表已经没有任务
	if(pRdyList->headPtr == NULL)
	{
		OS_BitMapClr(prio);
	}
}

/*********************************************************************************************************
* 函数名称: OS_TickListInit
* 函数功能: 在就绪列表中加入一个任务到末尾
* 输入参数: 任务句柄
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_TickListInit(void)
{
	s_OSTickList.headPtr = NULL;
	s_OSTickList.tailPtr = NULL;
}

/*********************************************************************************************************
* 函数名称: OS_TickTaskInsert
* 函数功能: 按照延时时长插入延时列表
* 输入参数: 任务句柄 延时时长
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_TickTaskInsert(OS_TASK_HANDLE *p_tcb, u64 time)
{
	u64 ticksRemain;
	OS_TASK_HANDLE *p_tcb_cur;	//需要插入的位置在这个任务的前面
	OS_TASK_HANDLE *p_tcb_prev;	//p_tcb_cur的前驱结点
	
	//输入参数检查
	if(p_tcb == NULL)
	{
		return;
	}
	if(p_tcb->tick == 0)	//如果延时结束 应该加入就绪列表
	{
		OS_RdyTaskAdd(p_tcb);
		return;
	}
	
	ticksRemain = p_tcb->tick;
	
	//列表为空
	if(s_OSTickList.headPtr == NULL)	
	{
		//直接插到头节点
		p_tcb->tick = time;	//延时时间可以直接保存到TCB
		p_tcb->tickNextPtr = NULL;
		p_tcb->tickPrevPtr = NULL;
		s_OSTickList.headPtr = p_tcb;
		s_OSTickList.tailPtr = p_tcb;
		return;
	}
	
	//列表不为空
	//遍历链表查找插入位置 并更新所有任务的相对延时时间
	p_tcb_cur = s_OSTickList.headPtr;
	p_tcb_prev = NULL;
	ticksRemain = time;
	while(p_tcb_cur != NULL)
	{
		if(ticksRemain <= p_tcb_cur->tick)
		{
			if(p_tcb_cur->tickPrevPtr == NULL)	//如果插入在头结点
			{
				p_tcb->tick = ticksRemain;
				p_tcb->tickPrevPtr = NULL;
				p_tcb->tickNextPtr = p_tcb_cur;
				p_tcb_cur->tick -= ticksRemain;
				p_tcb_cur->tickPrevPtr = p_tcb;
				s_OSTickList.headPtr = p_tcb;
				s_OSTickList.tailPtr = p_tcb_cur;
			}
			else	//如果插入不在头节点
			{
				p_tcb_prev = p_tcb_cur->tickPrevPtr;
				p_tcb->tick = ticksRemain;
				p_tcb->tickPrevPtr = p_tcb_prev;
				p_tcb->tickNextPtr = p_tcb_cur;
				p_tcb_cur->tick -= ticksRemain;
				p_tcb_cur->tickPrevPtr = p_tcb;
				p_tcb_prev->tickNextPtr = p_tcb;
			}
			return;
		}
		else	//还未找到要插入的位置 相对延时每次都要减少
		{
			ticksRemain -= p_tcb_cur->tick;
			p_tcb_prev = p_tcb_cur;
			p_tcb_cur = p_tcb_cur->tickNextPtr;
		}
	}
	
	//遍历到末尾 插入位置在列表末尾
	p_tcb->tick = ticksRemain;
	p_tcb->tickPrevPtr = p_tcb_prev;
	p_tcb->tickNextPtr = NULL;
	p_tcb_prev->tickNextPtr = p_tcb;
	s_OSTickList.tailPtr = p_tcb;
}

/*********************************************************************************************************
* 函数名称: OS_TickTaskRemove
* 函数功能: 从延时列表中删除某个任务
* 输入参数: 任务句柄
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_TickTaskRemove(OS_TASK_HANDLE *p_tcb)
{
	OS_TASK_HANDLE *p_tcb_prev;
	OS_TASK_HANDLE *p_tcb_next;
	
	if(p_tcb == NULL)
	{
		printf("Warning: Try to remove a NULL Task!\r\n");
		return;
	}
	
	p_tcb_prev = p_tcb->tickPrevPtr;
	p_tcb_next = p_tcb->tickNextPtr;
	
	//如果删除的是头结点
	if(p_tcb_prev == NULL)
	{
		if(p_tcb_next == NULL)	//而且是唯一一个节点
		{
			p_tcb->tickPrevPtr = NULL;
			p_tcb->tickNextPtr = NULL;
			s_OSTickList.headPtr = NULL;
			s_OSTickList.tailPtr = NULL;
		}
		else	//如果头结点后面还有别的结点
		{
			p_tcb_next->tickPrevPtr = NULL;
			p_tcb_next->tick += p_tcb->tick;
			p_tcb->tickPrevPtr = NULL;
			p_tcb->tickNextPtr = NULL;
			s_OSTickList.headPtr = p_tcb_next;
		}
	}
	else	//删除的不是头结点
	{
		p_tcb_prev->tickNextPtr = p_tcb_next;
		if(p_tcb_next != NULL)	//如果删除的不是尾结点
		{
			p_tcb_next->tickPrevPtr = p_tcb_prev;
			p_tcb_next->tick += p_tcb->tick;
		}
		else	//删除的是尾结点
		{
			s_OSTickList.tailPtr = p_tcb_prev;
		}
		p_tcb->tickPrevPtr = NULL;
		p_tcb->tickNextPtr = NULL;
		p_tcb->tick = 0;
	}
}

/*********************************************************************************************************
* 函数名称: OS_TickUpdate
* 函数功能: 时钟中断 任务调度
* 输入参数: 任务句柄
* 输出参数: pNeedSchedFlag 是否需要切换任务标志
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_TickUpdate(void)
{
	OS_TASK_HANDLE *headTask = NULL;
	
	headTask = s_OSTickList.headPtr;
	if(headTask == NULL)	//没有任务正在延时
	{
		return;
	}
	
	//延时列表最前面的任务tick递减
	headTask->tick--;
	
	while(headTask != NULL && headTask->tick == 0)
	{
		OS_TickTaskRemove(headTask);
		OS_RdyTaskAdd(headTask);
		headTask = s_OSTickList.headPtr;	//重新获取新的头节点
	}
}

/*********************************************************************************************************
* 函数名称: OS_BitMapInit
* 函数功能: 初始化优先级位图
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_BitMapInit(void)
{
	s_OSPrioBitMap = (u32)0;
}

/*********************************************************************************************************
* 函数名称: OS_BitMapSet
* 函数功能: 优先级位图置位
* 输入参数: 任务优先级
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_BitMapSet(u32 prio)
{
	if(prio >= OS_CFG_PRIO_MAX)
	{
		printf("ERROR: [OS_BitMapSet] Illegal Argument!\r\n");
		return;
	}
	prio = 31 - prio;
	s_OSPrioBitMap |= (1 << prio);
}

/*********************************************************************************************************
* 函数名称: OS_BitMapClr
* 函数功能: 优先级位图位清除
* 输入参数: 任务优先级
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_BitMapClr(u32 prio)
{
	if(prio >= OS_CFG_PRIO_MAX)
	{
		printf("ERROR: [OS_BitMapSet] Illegal Argument!\r\n");
		return;
	}
	prio = 31 - prio;
	s_OSPrioBitMap &= ~(1 << prio);
}

/*********************************************************************************************************
* 函数名称: OS_BitMapGetHighest
* 函数功能: 获取最高优先级的就绪任务
* 输入参数: 任务优先级
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
u32 OS_BitMapGetHighest(void)
{
	if(s_OSPrioBitMap == (u32)0)
	{
		printf("ERROR: [OS_BitMapGetHighest] No Ready Task!\r\n");
		while(1){}	//没有就绪任务, 报错卡住
	}
	
	return __builtin_clz(s_OSPrioBitMap);
}

/*********************************************************************************************************
* 函数名称: OS_PendListInit
* 函数功能: 初始化某个对象的挂起队列
* 输入参数: 挂起队列
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_PendListInit(OS_PEND_LIST *p_pend_list)
{
	p_pend_list->headPtr = NULL;
	p_pend_list->tailPtr = NULL;
}

/*********************************************************************************************************
* 函数名称: OS_PendListInsert
* 函数功能: 按优先级(从大到小)插入挂起列表
* 输入参数: 查找选项(可选任务名/任务函数首地址) 查询关键字
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_PendListInsert(OS_PEND_LIST *p_pend_list, OS_TASK_HANDLE *p_tcb)
{
	OS_TASK_HANDLE *p_tcb_cur;	//需要插入在这个任务的后面
	OS_TASK_HANDLE *p_tcb_prev;	//p_tcb_cur的前驱结点
	u32 prio;
	
	//输入参数检查
	if(p_pend_list == NULL || p_tcb == NULL)
	{
		printf("ERROR: [OS_PendListInsert] Illegal Argument!\r\n");
		return;
	}
	
	//列表为空
	if(p_pend_list->headPtr == NULL)	
	{
		//直接加入列表
		p_tcb->pendPrevPtr = NULL;
		p_tcb->pendNextPtr = NULL;
		p_pend_list->headPtr = p_tcb;
		p_pend_list->tailPtr = p_tcb;
		return;
	}
	
	//列表不为空
	prio = p_tcb->priority;
	//遍历链表查找插入位置
	p_tcb_cur = p_pend_list->headPtr;
	p_tcb_prev = NULL;
	while(p_tcb_cur != NULL && prio > p_tcb_cur->priority)
	{
		p_tcb_prev = p_tcb_cur;
		p_tcb_cur = p_tcb_cur->pendNextPtr;
	}	//遍历结束后 插入位置在cur和prev中间
	
	if(p_tcb_prev == NULL)	//插到头结点
	{
		p_tcb_cur->pendPrevPtr = p_tcb;
		p_tcb->pendPrevPtr = NULL;
		p_tcb->pendNextPtr = p_tcb_cur;
		p_pend_list->headPtr = p_tcb;
	}
	else if(p_tcb_cur == NULL)	//插到尾结点
	{
		p_tcb_prev->pendNextPtr = p_tcb;
		p_tcb->pendPrevPtr = p_tcb_prev;
		p_tcb->pendNextPtr = NULL;
		p_pend_list->tailPtr = p_tcb;
	}
	else	//若插入位置是中间结点
	{
		p_tcb_prev->pendNextPtr = p_tcb;
		p_tcb_cur->pendPrevPtr = p_tcb;
		p_tcb->pendPrevPtr = p_tcb_prev;
		p_tcb->pendNextPtr = p_tcb_cur;
	}
}

/*********************************************************************************************************
* 函数名称: OS_PendListRemove
* 函数功能: 从挂起列表中移除某个任务
* 输入参数: 查找选项(可选任务名/任务函数首地址) 查询关键字
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用
*********************************************************************************************************/
void OS_PendListRemove(OS_PEND_LIST *p_pend_list, OS_TASK_HANDLE *p_tcb)
{
	OS_TASK_HANDLE *p_tcb_prev;
	OS_TASK_HANDLE *p_tcb_next;

	//输入参数检查
	if(p_pend_list == NULL || p_tcb == NULL)
	{
		printf("ERROR: [OS_PendListRemove] Illegal Argument!\r\n");
		return;
	}

	//删除列表唯一的结点
	if(p_tcb->pendPrevPtr == NULL && p_tcb->pendNextPtr == NULL)
	{
		p_pend_list->headPtr = NULL;
		p_pend_list->tailPtr = NULL;
		return;
	}

	p_tcb_prev = p_tcb->pendPrevPtr;
	p_tcb_next = p_tcb->pendNextPtr;
	//如果不是列表中唯一的结点
	if(p_tcb_prev == NULL)	//在头结点
	{
		p_tcb_next->pendPrevPtr = NULL;
		p_tcb->pendNextPtr = NULL;
		p_pend_list->headPtr = p_tcb_next;
	}
	else if(p_tcb_next == NULL)	//在尾结点
	{
		p_tcb_prev->pendNextPtr = NULL;
		p_tcb->pendPrevPtr = NULL;
		p_pend_list->tailPtr = p_tcb_prev;
	}
	else	//在中间结点
	{
		p_tcb_prev->pendNextPtr = p_tcb_next;
		p_tcb_next->pendPrevPtr = p_tcb_prev;
		p_tcb->pendPrevPtr = NULL;
		p_tcb->pendNextPtr = NULL;
	}	
}

/*********************************************************************************************************
* 函数名称: OS_PendListPopAlltoRdyList
* 函数功能: 把挂起列表里的所有任务唤醒到就绪任务
* 输入参数: 某个对象的挂起列表
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月26日
* 注    意: 非用户调用 内核其他部分使用
*						删除对象时 始终唤醒所有等待者 & 广播事件通知 唤醒所有等待者
*********************************************************************************************************/
void OS_PendListPopAlltoRdyList(OS_PEND_LIST *p_pend_list)
{
	OS_TASK_HANDLE *p_tcb;
	OS_TASK_HANDLE *p_tcb_next;
	
	if(p_pend_list == NULL || (p_pend_list->headPtr == NULL && p_pend_list->tailPtr == NULL))
	{
		//列表不合法或列表为空
		return;
	}

	p_tcb = p_pend_list->headPtr;
	while (p_tcb != NULL)
	{
		p_tcb_next = p_tcb->pendNextPtr;
		OS_PendListRemove(p_pend_list, p_tcb);
		OS_RdyTaskAdd(p_tcb);
		p_tcb = p_tcb_next;
	}
}

/*********************************************************************************************************
* 函数名称: OS_PendListGetHighest
* 函数功能: 获取列表中优先级最高的任务(但不从列表中删除)
* 输入参数: 某个对象的挂起列表
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月22日
* 注    意: 非用户调用 内核其他部分使用 
*						因为Insert时已排序，所以直接返回head即可
*********************************************************************************************************/
OS_TASK_HANDLE* OS_PendListGetHighest(OS_PEND_LIST *p_pend_list)
{
	return (p_pend_list != NULL) ? p_pend_list->headPtr : NULL;
}
