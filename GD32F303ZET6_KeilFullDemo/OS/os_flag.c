/*********************************************************************************************************
* 模块名称：os_flag.c
* 摘    要：事件标志组组件
* 当前版本：1.0.0
* 作    者：Chill
* 完成日期：2026年01月29日
* 内    容：
*           (1) 提供事件标志组（Event Flags）机制，用于任务间同步
*           (2) 支持等待标志位：SET_ALL / SET_ANY / CLR_ALL / CLR_ANY
*           (3) 支持对满足条件的最高优先级任务进行唤醒
* 注    意：
*           (1) 本实现中任务等待成功后会“消耗标志位”（仅对等待 SET_xxx 生效）
*           (2) 标志组的挂起队列为 OS_PEND_LIST，节点为 OS_TASK_HANDLE
*           (3) 优先级比较规则：priority 数值越小优先级越高
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
#include "CasyOS.h"

#if OS_CFG_FLAG_EN != 0
/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
extern OS_TASK_HANDLE *g_pCurrentTask;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u8 IsFlagsMatch(u32 flags, OS_TASK_HANDLE *p_tcb);
static OS_TASK_HANDLE *PendListGetHighestMatch(OS_PEND_LIST *p_pend_list, u32 flags);
/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：IsFlagsMatch
* 函数功能：检查当前标志位更改以后是否与任务等待的标志组合匹配
* 输入参数：flags: 当前事件标志组的标志值 p_tcb: 任务控制块指针(包含等待掩码/等待选项)
* 输出参数：void
* 返 回 值：u8 0-匹配失败 1-匹配成功
* 创建日期：2026年01月29日
* 注    意：内部辅助函数，仅用于事件标志组匹配判断
*********************************************************************************************************/
static u8 IsFlagsMatch(u32 flags, OS_TASK_HANDLE *p_tcb)
{
  u32 mask = p_tcb->flagsMaskPendOn;
  switch(p_tcb->flagsPendOpt)
  {
    case OS_FLAG_WAIT_SET_ALL:
      return ((flags & mask) == mask);
    case OS_FLAG_WAIT_SET_ANY:
      return ((flags & mask) != 0);
    case OS_FLAG_WAIT_CLR_ALL:
      return ((flags & mask) == 0);
    case OS_FLAG_WAIT_CLR_ANY:
      return ((flags & mask) != mask);
    default:
      return 0;
  }
}

/*********************************************************************************************************
* 函数名称：PendListGetHighestMatch
* 函数功能：在挂起队列中查找"满足 flags 条件"的最高优先级任务
* 输入参数：p_pend_list: 事件标志组的挂起队列 flags: 事件标志组的标志值
* 输出参数：void
* 返 回 值：OS_TASK_HANDLE*: 返回匹配成功的最高优先级任务指针 若无任务匹配或队列为空则返回 NULL
* 创建日期：2026年01月29日
* 注    意：
*           (1) 遍历挂起队列，筛选满足条件的任务
*           (2) priority 数值越小表示优先级越高
*           (3) 内部辅助函数
*********************************************************************************************************/
static OS_TASK_HANDLE *PendListGetHighestMatch(OS_PEND_LIST *p_pend_list, u32 flags)
{
  OS_TASK_HANDLE *p_tcb;
  OS_TASK_HANDLE *p_best;

  if (p_pend_list == NULL || p_pend_list->headPtr == NULL)
  {
    return NULL;
  }

  p_best = NULL;
  p_tcb  = p_pend_list->headPtr;

  while (p_tcb != NULL)
  {
    //只筛选 flags 匹配的任务
    if(IsFlagsMatch(flags, p_tcb))
    {
      if (p_best == NULL)
      {
        p_best = p_tcb;
      }
      else
      {
        // 比较优先级，选更高的那个
        if (p_tcb->priority < p_best->priority)
        {
            p_best = p_tcb;
        }
      }
    }

    p_tcb = p_tcb->pendNextPtr;
  }

  return p_best;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：OSFLAGCreate
* 函数功能：创建事件标志组
* 输入参数：p_flag: 事件标志组指针 p_name: 信号量字符串 init_flag: 初始标志组值
* 输出参数：
* 返 回 值：void
* 创建日期：2026年01月29日
* 注    意：
*           (1) 创建时初始化挂起队列为空
*           (2) flags 初始化为 init_flags
*********************************************************************************************************/
void OSFlagCreate(OS_FLAG *p_flag, u8* p_name,u32 init_flags)
{
  p_flag->objType = OS_OBJ_TYPE_FLAG;
  p_flag->pendList.headPtr = NULL;
  p_flag->pendList.tailPtr = NULL;
  p_flag->name = p_name;
  p_flag->flags = init_flags;
}

/*********************************************************************************************************
* 函数名称：OSFlagPost
* 函数功能：设置/清除事件标志位，并尝试唤醒等待该标志组的任务
* 输入参数：p_flag: 事件标志组指针 mask: 需要操作的位掩码 opt: 操作选项(OS_FLAG_SET / OS_FLAG_CLR)
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月29日
* 注    意：
*           (1) 修改 flags 后，若挂起队列中存在满足条件的任务，则唤醒最高优先级任务
*           (2) 本实现中任务等待成功后会消耗标志位（仅对等待 SET_xxx 生效）
*           (3) 可能触发调度（OS_Sched）
*********************************************************************************************************/
void OSFlagPost(OS_FLAG *p_flag, u32 mask, OS_FLAG_SET_OPT opt)
{
  OS_TASK_HANDLE *p_tcb;
  OS_PEND_LIST *p_pend_list;
  u32 curFlags;

  OS_ENTER_CRITICAL();

  if(p_flag == NULL || p_flag->objType != OS_OBJ_TYPE_FLAG)
	{
		printf("ERROR:[OSFlagPost] Illegal argument!\r\n");
		OS_EXIT_CRITICAL();
		return;
	}

  curFlags = p_flag->flags;
  if(opt == OS_FLAG_SET)
  {
    curFlags |= mask;
  }
  else
  {
    curFlags &= ~mask;
  }
  p_flag->flags = curFlags;

  //如果等待列表有符合要求的高优先级等待任务 直接唤醒它
  p_pend_list = &p_flag->pendList;
  if(p_pend_list->headPtr != NULL)
  {
    p_tcb = PendListGetHighestMatch(p_pend_list, curFlags);
    if(p_tcb != NULL)
    {
      //如果任务等待的是 SET_xxx，则消耗清0
      if(p_tcb->flagsPendOpt == OS_FLAG_WAIT_SET_ALL || p_tcb->flagsPendOpt == OS_FLAG_WAIT_SET_ANY)
      {
          p_flag->flags &= ~(p_tcb->flagsMaskPendOn);
      }

      OS_PendListRemove(p_pend_list, p_tcb);
      OS_RdyTaskAdd(p_tcb);
      p_tcb->state = OS_TASK_READY;
      p_tcb->pendObj = NULL;

      OS_EXIT_CRITICAL();
      OS_Sched();
      return;
    }
  }

  OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
* 函数名称：OSFlagPend
* 函数功能：任务等待事件标志组满足指定条件
* 输入参数：p_flag: 事件标志组指针 mask: 需要操作的位掩码 opt: 操作选项(OS_FLAG_SET / OS_FLAG_CLR)
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月29日
* 注    意：
*           (1) 若当前 flags 已满足条件，则直接返回（并可能消耗标志位）
*           (2) 若不满足条件，则任务进入挂起队列，并触发调度
*           (3) 本实现中等待成功会消耗标志位（仅对等待 SET_xxx 生效）
*********************************************************************************************************/
void OSFlagPend(OS_FLAG *p_flag, u32 mask, OS_FLAG_PEDN_OPT opt)
{
  OS_ENTER_CRITICAL();

  if(p_flag == NULL || p_flag->objType != OS_OBJ_TYPE_FLAG)
	{
		printf("ERROR:[OSFlagPost] Illegal argument!\r\n");
		OS_EXIT_CRITICAL();
		return;
	}

  g_pCurrentTask->flagsMaskPendOn = mask;
  g_pCurrentTask->flagsPendOpt = opt;

  //如果当前标志组已经满足任务等待条件 则可以直接返回
  if(IsFlagsMatch(p_flag->flags, g_pCurrentTask))
  {
    if(opt == OS_FLAG_WAIT_SET_ALL || OS_FLAG_WAIT_SET_ANY)
    {
      //只对SET等待生效 消耗标志位清0
      p_flag->flags &= ~mask;
    }
    g_pCurrentTask->flagsMaskPendOn = 0;
    g_pCurrentTask->flagsPendOpt = OS_FLAG_WAIT_NONE;
    OS_EXIT_CRITICAL();
    return;
  }
  else  //当前标志组不满足任务条件
  {
    OS_RdyTaskRemove(g_pCurrentTask);
		OS_PendListInsert(&p_flag->pendList, g_pCurrentTask);
		g_pCurrentTask->state = OS_TASK_PEND;
		g_pCurrentTask->pendObj = (void*)p_flag;
    OS_EXIT_CRITICAL();
    OS_Sched();
  }
}

#endif //OS_CFG_FLAG_EN
