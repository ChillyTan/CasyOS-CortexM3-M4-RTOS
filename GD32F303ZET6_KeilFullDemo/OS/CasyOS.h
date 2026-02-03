/*********************************************************************************************************
* 模块名称：CasyOS.h
* 摘    要：简易操作系统实现
* 当前版本：1.0.0
* 作    者：Chill
* 完成日期：2026年01月31日 
* 内    容：
*           (1) 内核核心数据结构定义：TCB、就绪链表、延时链表、挂起链表
*           (2) 内核对象定义：信号量、互斥量、消息队列、事件标志组
*           (3) 内存管理接口：OSMalloc/OSFree（可裁剪）
*           (4) 内核 API 声明：调度、任务注册、延时、同步对象操作等
*
* 注    意：
*						(1) 此文件一般不允许用户修改
*           (2) 本OS适用于 Cortex-M3/M4 内核（依赖 PendSV/SVC/SysTick 进行任务切换）
*           (3) 任务栈必须满足 8 字节对齐要求（ARM EABI / 异常入栈要求）
*           (4) 组件通过 os_cfg.h 进行裁剪开关控制（OS_CFG_xxx_EN）
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/
#ifndef _CASY_OS_H_
#define _CASY_OS_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "os_cfg.h"				//OS裁剪配置、宏定义（是否使能某组件、最大优先级等）
#include "os_datatype.h"	//OS基础数据类型定义（u8/u16/u32/u64 等）

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
//前置声明 避免结构体互相包含
typedef struct OS_TASK_HANDLE OS_TASK_HANDLE;
typedef struct OS_RDY_LIST OS_RDY_LIST;
typedef struct OS_TICK_LIST OS_TICK_LIST;
typedef struct OS_PEND_LIST OS_PEND_LIST;
typedef struct OS_Q OS_Q;

/*--------------------------------------------------------------------------------------------------------
                                               内核对象通用类型
--------------------------------------------------------------------------------------------------------*/
/*
 * OS_OBJ_TYPE
 * 功能：标识一个内核对象的类型（用于运行时检查、调试、参数合法性判断）
 */
typedef enum OS_OBJ_TYPE
{
	OS_OBJ_TYPE_NONE,		//可作为对象无效标志
	OS_OBJ_TYPE_SEM,		//信号量
	OS_OBJ_TYPE_MUTEX,	//互斥信号量
	OS_OBJ_TYPE_Q,			//消息队列
	OS_OBJ_TYPE_FLAG		//事件标志组
}OS_OBJ_TYPE;

/*
 * OS_PEND_LIST
 * 功能：挂起队列（等待某对象的任务链表）
 * 说明：
 *   headPtr/tailPtr 指向等待该对象的任务队列头/尾
 *   队列内部按优先级排序（你后续实现 OS_PendListInsert 时体现）
 */
typedef struct OS_PEND_LIST
{
	OS_TASK_HANDLE* headPtr;	//头指针
	OS_TASK_HANDLE* tailPtr;	//尾指针
}OS_PEND_LIST;

/*--------------------------------------------------------------------------------------------------------
                                               内存管理
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_MEM_EN != 0u

/*
 * OS_MEM_NODE
 * 功能：内存池管理链表结点（每个分配块的头部）
 * 说明：
 *   memUsedSize:
 *      bit[31] = 1 已占用，bit[31] = 0 空闲
 *      bit[30:0] 表示该块的大小（单位：字节）
 *   memNextNode:
 *      指向下一块内存结点
 */
typedef struct OS_MEM_NODE
{
	u32 								memUsedSize;	//内存块已占用大小
	struct OS_MEM_NODE* memNextNode;	//内存块头(记录占用信息)
}OS_MEM_NODE, *OS_MEM_LIST;

/*
 * 内存管理模块状态枚举
 */
typedef enum OS_MEM_STAT
{
	OS_MEM_NOT_READY,		//内存管理模块未初始化
	OS_MEM_READY				//内存管理模块已就绪
}OS_MEM_STAT;

/*
 * OS_MEM_DEV
 * 功能：内存管理控制器
 * 成员说明：
 *   init      : 初始化内存管理模块
 *   perused   : 获取内存使用率（0~1000 或 0~100，取决于实现）
 *   memoryBase: 内存池起始地址
 *   memoryEnd : 内存池终止地址
 *   memoryList: 内存块链表头
 *   memoryRdy : 模块状态
 */
typedef struct OS_MEM_DEV
{
  void					(*init)(void);		//初始化
  u16						(*perused)(void);	//内存使用率
  u8*						memoryBase;				//内存池起始地址
	u8*						memoryEnd;				//内存池终止地址
  OS_MEM_LIST   memoryList;				//内存管理链表
  OS_MEM_STAT   memoryRdy;				//内存管理是否就绪
}OS_MEM_DEV;

#endif	//OS_CFG_MEM_EN

/*--------------------------------------------------------------------------------------------------------
                                               信号量组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_SEM_EN != 0

/*
 * OS_SEM
 * 功能：信号量对象（计数信号量/二值信号量）
 * 成员说明：
 *   objType  : 必须为 OS_OBJ_TYPE_SEM
 *   pendList : 等待该信号量的任务列表
 *   count    : 当前可用资源数
 *   countMax : 最大资源数（count 不得超过 countMax）
 */
typedef struct OS_SEM
{
	OS_OBJ_TYPE 	objType;		//对象类型
	OS_PEND_LIST 	pendList;		//挂起列表
	u32 					count;			//当前计数量
	u32 					countMax;		//最大计数量
}OS_SEM;

#endif	//OS_CFG_SEM_EN

/*--------------------------------------------------------------------------------------------------------
                                               内建消息队列组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_Q_EN != 0

/*
 * OS_Q
 * 功能：消息队列对象（循环队列）
 * 说明：
 * 	 objType	: 必须为 OS_OBJ_TYPE_Q
 *   msgBase  : 动态分配的消息缓冲区首地址（每个元素 u32）
 *   countMax : 队列最大长度（元素个数）
 *   count    : 当前队列中消息数量
 *   inIdx    : 写索引（入队位置）
 *   outIdx   : 读索引（出队位置）
 */
typedef struct OS_Q
{
	OS_OBJ_TYPE    objType;     //对象类型
	u32           *msgBase;     //动态分配的消息缓冲区首地址
	u32            countMax;    //队列最大长度
	u32            count;       //当前消息数量
	u32            inIdx;       //写指针索引
	u32            outIdx;      //读指针索引
}OS_Q;

#endif	//OS_CFG_Q_EN

/*--------------------------------------------------------------------------------------------------------
                                               互斥量组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_MUTEX_EN != 0

/*
 * OS_MUTEX
 * 功能：互斥量对象（支持递归锁）
 * 成员说明：
 *   ownerTcb : 当前持有互斥量的任务
 *   lockCnt  : 递归锁计数（同一任务重复 Pend 则计数+1）
 */
typedef struct OS_MUTEX
{
	OS_OBJ_TYPE 		objType;		//对象类型
	OS_PEND_LIST 		pendList;		//挂起列表
	u8							*name;			//互斥量名称
	OS_TASK_HANDLE 	*ownerTcb;	//当前持有互斥量的任务
	u32 						lockCnt;		//递归锁计数
}OS_MUTEX;

#endif	//OS_CFG_MUTEX_EN

/*--------------------------------------------------------------------------------------------------------
                                               事件标志组
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_FLAG_EN != 0

/*
 * OS_FLAG
 * 功能：事件标志组对象
 * 成员说明：
 *   flags : 32位事件标志集合
 */
typedef struct OS_FLAG
{
	OS_OBJ_TYPE 		objType;		//对象类型
	OS_PEND_LIST 		pendList;		//挂起列表
	u8							*name;			//事件标志组名称
	u32							flags;			//标志位组合
}OS_FLAG;

/*
 * 标志组等待选项：
 * - OS_FLAG_WAIT_SET_ALL : 等待 mask 指定的所有位都为1
 * - OS_FLAG_WAIT_SET_ANY : 等待 mask 指定的任意一位为1
 * - OS_FLAG_WAIT_CLR_ALL : 等待 mask 指定的所有位都为0
 * - OS_FLAG_WAIT_CLR_ANY : 等待 mask 指定的任意一位为0
 */
typedef enum OS_FLAG_PEDN_OPT
{
	OS_FLAG_WAIT_NONE,		//表示不等待
	OS_FLAG_WAIT_SET_ALL,
	OS_FLAG_WAIT_SET_ANY,
	OS_FLAG_WAIT_CLR_ALL,
	OS_FLAG_WAIT_CLR_ANY
}OS_FLAG_PEDN_OPT;

/*
 * 标志位设置选项：
 * OS_FLAG_SET : 置位
 * OS_FLAG_CLR : 清零
 */
typedef enum OS_FLAG_SET_OPT
{
	OS_FLAG_SET,
	OS_FLAG_CLR
}OS_FLAG_SET_OPT;

#endif	//OS_CFG_FLAG_EN

/*--------------------------------------------------------------------------------------------------------
                                               任务管理
--------------------------------------------------------------------------------------------------------*/
/*
 * OS_TASK_STAT
 * 功能：任务状态枚举
 * 说明：
 *   RUNNING : 当前正在运行的任务（OSTCBCur）
 *   READY   : 就绪态，可被调度运行
 *   DELAY   : 延时态，位于 tick 增量链表中
 *   PEND    : 挂起态，等待某对象（信号量/互斥量/队列/标志组）
 *   STOP    : 停止态（保留扩展）
 */
typedef enum OS_TASK_STAT
{
	OS_TASK_RUNNING,
	OS_TASK_READY,
	OS_TASK_DELAY,
	OS_TASK_PEND,
	OS_TASK_STOP
}OS_TASK_STAT;

/*
 * OS_RDY_LIST
 * 功能：就绪任务链表（通常按进入顺序或优先级组织）
 */
typedef struct OS_RDY_LIST
{
	OS_TASK_HANDLE* headPtr;	//指向最先进入ReadyList的任务
	OS_TASK_HANDLE* tailPtr;	//指向最后进入ReadyList的任务
}OS_RDY_LIST;

/*
 * OS_TICK_LIST
 * 功能：延时增量链表（Delta List）
 * 说明：
 *   - headPtr 指向最近到期的任务（每次 SysTick 只需减 head 的 tick）
 *   - 任务插入时按到期时间排序，并存储相对 tick（delta）
 */
typedef struct OS_TICK_LIST
{
	OS_TASK_HANDLE* headPtr;	//指向最先到期的任务(每次只减第一个任务的tick)
	OS_TASK_HANDLE* tailPtr;	//指向最后到期的任务
}OS_TICK_LIST;

/*
 * OS_TASK_ID_TYPE
 * 功能：任务查找方式
 * 说明：
 *   OS_TASK_ID_TCB  : 通过 TCB 指针查找
 *   OS_TASK_ID_NAME : 通过任务名称查找（需使能 OS_CFG_TASK_NAME_EN）
 *   OS_TASK_ID_FUNC : 通过入口函数指针查找（需使能 OS_CFG_TASK_FUNC_EN）
 */
typedef enum OS_TASK_ID_TYPE
{
    OS_TASK_ID_TCB,
#if OS_CFG_TASK_NAME_EN != 0
    OS_TASK_ID_NAME,
#endif
#if OS_CFG_TASK_FUNC_EN != 0
    OS_TASK_ID_FUNC,
#endif
}OS_TASK_ID_TYPE;

/*
 * OS_TASK_HANDLE (TCB)
 * 功能：任务控制块（Task Control Block）
 * 关键字段说明：
 *   stackTop/stackBase/stackSize : 任务栈信息（stackTop 必须在结构体首地址）
 *   priority                    : 任务优先级（数值越小优先级越高）
 *   tick                        : 延时计数（用于 OSDelay / TickList）
 *   state                       : 当前任务状态
 *   pendObj                     : 当前挂起等待的对象指针（NULL表示未挂起）
 *   nextPtr                     : 全局任务单链表指针
 *   rdyNextPtr/rdyPrevPtr       : 就绪链表双向指针
 *   tickNextPtr/tickPrevPtr     : 延时链表双向指针
 *   pendNextPtr/pendPrevPtr     : 挂起链表双向指针
 *
 * 可裁剪字段：
 *   sem       : 任务内建信号量（OS_CFG_SEM_EN）
 *   msgQueue  : 任务内建消息队列（OS_CFG_Q_EN）
 *   flagsPend : 事件标志组等待参数（OS_CFG_FLAG_EN）
 */
typedef struct OS_TASK_HANDLE
{
  u32*  						stackTop;  				//栈顶指针，8字节对齐，必须位于起始位置
  u32*  						stackBase; 				//栈区首地址
  u32   						stackSize; 				//栈区大小，按4字节计算
  void* 						func;      				//任务入口，为 void (*)(void) 类型的函数指针
	char* 						taskName;	 				//任务名称
  u32   						priority;  				//任务优先级，0为最大优先级
  u64   						tick;      				//延时计数，单位ms
	OS_TASK_STAT 			state;						//任务当前状态
	void* 						pendObj;					//当前挂起等待的对象(NULL表示没有挂起等待)
	OS_TASK_HANDLE* 	nextPtr;					//全局任务单链表后向指针
	OS_TASK_HANDLE* 	rdyNextPtr;				//就绪链表后向指针
	OS_TASK_HANDLE* 	rdyPrevPtr;				//就绪列表前向指针
	OS_TASK_HANDLE* 	tickNextPtr;			//延时增量列表后向指针
	OS_TASK_HANDLE* 	tickPrevPtr;			//延时增量列表前向指针
	OS_TASK_HANDLE* 	pendNextPtr;			//挂起列表后向指针
	OS_TASK_HANDLE* 	pendPrevPtr;			//挂起列表前向指针
#if OS_CFG_SEM_EN != 0
	OS_SEM 						sem;							//任务内建信号量
#endif
#if OS_CFG_Q_EN != 0
	OS_Q 							msgQueue;					//内建消息队列
	u32 							msgTemp;					//当前收到的消息
#endif
#if OS_CFG_FLAG_EN != 0
	u32 							flagsMaskPendOn;	//等待哪几位标志位(位掩码)
	OS_FLAG_PEDN_OPT 	flagsPendOpt;			//等待条件（ALL/ANY SET/CLR）
#endif
}OS_TASK_HANDLE;

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------
                                               CasyOS内核
--------------------------------------------------------------------------------------------------------*/
//OS内部函数 这些函数一般由内核调用 不建议用户直接调用
void OS_Sched(void); 							//触发调度(通常触发PendSV)
void OS_UpdateCurrentTask(void);	//更新当前应运行任务(选最高优先级任务)
void SysTick_Handler(void);   		//SysTick中断服务函数
void SVC_Handler(void);       		//SVC中断服务函数
void PendSV_Handler(void);    		//PendSV中断服务函数
void OS_ENTER_CRITICAL(void);			//进入临界区
void OS_EXIT_CRITICAL(void);			//退出临界区
u8	 OS_InISR(void);

//用户函数 用户可以调用
void InitCasyOS(void);
u32  OSRegister(OS_TASK_HANDLE* p_tcb, 
								void* 					func, 
								char*						p_name, 
								u32 						prio, 
								u32* 						stkBase, 
								u32 						stkSize, 
								u32 						semSize, 
								u32 						queSize); //任务注册

void OSStart(void);     					//系统开启
void OSDelay(u32 time); 					//任务延时
void OSIntEnter(void);						//通知CasyOS进入中断或中断嵌套
void OSIntExit(void);							//通知CasyOS退出中断

/*--------------------------------------------------------------------------------------------------------
                                               任务管理
--------------------------------------------------------------------------------------------------------*/
/*
 * 任务管理模块：
 * - 全局任务链表：用于遍历/查找任务
 * - 就绪链表+优先级位图：用于快速找到最高优先级任务
 * - 延时增量链表：用于高效管理延时任务
 */

//用户函数
void OSInitTaskSched(void);																	//初始化任务调度管理模块
OS_TASK_HANDLE* OSTaskFind(OS_TASK_ID_TYPE opt, void *id);	//按照不同类型信息查找任务

//OS内部函数
void OS_TaskListInit(void);                          				//初始化全局任务链表
void OS_TaskListAdd(OS_TASK_HANDLE *p_tcb);          				//添加任务到全局任务链表
void OS_TaskListRemove(OS_TASK_HANDLE *p_tcb);       				//从全局任务链表删除任务

void OS_RdyListInit(void);                           				//初始化就绪链表
void OS_RdyTaskAdd(OS_TASK_HANDLE *p_tcb);           				//加入就绪链表
void OS_RdyTaskRemove(OS_TASK_HANDLE *p_tcb);        				//移出就绪链表

void OS_TickListInit(void);                          				//初始化延时链表
void OS_TickTaskInsert(OS_TASK_HANDLE *p_tcb, u64 time); 		//插入延时任务（time为延时tick）
void OS_TickTaskRemove(OS_TASK_HANDLE *p_tcb);       				//移除延时任务
void OS_TickUpdate(void);                            				//滴答更新（处理延时到期任务）

void OS_BitMapInit(void);                            				//初始化优先级位图
void OS_BitMapSet(u32 prio);                         				//置位某优先级
void OS_BitMapClr(u32 prio);                         				//清除某优先级
u32  OS_BitMapGetHighest(void);                      				//获取最高优先级（数值最小）

void OS_PendListInit(OS_PEND_LIST *p_pend_list);                 					//初始化挂起队列
void OS_PendListInsert(OS_PEND_LIST *p_pend_list, OS_TASK_HANDLE *p_tcb); //按优先级插入挂起队列
void OS_PendListRemove(OS_PEND_LIST *p_pend_list, OS_TASK_HANDLE *p_tcb); //移除指定任务
void OS_PendListPopAlltoRdyList(OS_PEND_LIST *p_pend_list);      					//将挂起队列全部任务转入就绪队列
OS_TASK_HANDLE* OS_PendListGetHighest(OS_PEND_LIST *p_pend_list);					//获取最高优先级等待任务


/*--------------------------------------------------------------------------------------------------------
                                               内存管理
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_MEM_EN != 0

//用户函数
void  OSInitMemory(void);     	//初始化Malloc模块
u16   OSMemoryPerused(void);  	//获得内存使用率(外/内部调用) 
void* OSMalloc(u32 size); 			//内存分配(单位字节)
void  OSFree(void* ptr);  			//内存释放

#endif	//OS_CFG_MEM_EN

/*--------------------------------------------------------------------------------------------------------
                                               内建信号量组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_SEM_EN != 0
/*
 * 信号量接口说明：
 * OS_SemInit  : 初始化信号量（设置最大计数/清空挂起队列）
 * OSSemPost   : 释放信号量（计数+1，必要时唤醒挂起任务）
 * OSSemPend   : 等待信号量（计数>0则直接获取，否则挂起）
 */

//OS内部函数
void OS_SemInit(OS_SEM *p_sem, u32 countMax);	//初始化任务内部的信号量
//用户函数
void OSSemPost(OS_SEM *p_sem);								//释放1个信号量
void OSSemPend(OS_SEM *p_sem);								//等待1个信号量

#endif	//OS_CFG_SEM_EN

/*--------------------------------------------------------------------------------------------------------
                                               内建消息队列组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_Q_EN != 0
/*
 * 消息队列接口说明（内建队列）：
 * OS_QInit     : 为某任务初始化消息队列（动态申请缓冲区）
 * OSQPost      : 向某任务队列发送一个 4字节消息
 * OSQPend      : 当前任务阻塞等待消息
 * OSQFlush     : 清空队列
 * OSQGetCount  : 获取队列当前消息数量
 * OSQIsEmpty   : 队列是否为空
 * OSQIsFull    : 队列是否已满
 */

//OS内部函数
void OS_QInit(OS_TASK_HANDLE* p_tcb, u32 countMax);	//在任务内部创建消息队列

//用户函数
void OSQPost(OS_TASK_HANDLE* p_tcb, u32 msg);				//按照4字节发送消息到某个任务
void OSQPend(u32 *p_msg);														//阻塞等待消息队列里的消息
void OSQFlush(OS_TASK_HANDLE* p_tcb);								//清空消息队列中的消息
u32  OSQGetCount(OS_TASK_HANDLE* p_tcb);						//获取消息队列中消息数量
u8 	 OSQIsEmpty(OS_TASK_HANDLE* p_tcb);							//判断消息队列是否空
u8 	 OSQIsFull(OS_TASK_HANDLE* p_tcb);							//判断消息队列是否满

#endif //OS_CFG_Q_EN

/*--------------------------------------------------------------------------------------------------------
                                               互斥量组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_MUTEX_EN != 0
/*
 * 互斥量接口说明：
 * OSMutexCreate : 创建互斥量（初始化挂起队列/owner/lockCnt）
 * OSMutexPost   : 释放互斥量（递归计数清零后释放并唤醒等待任务）
 * OSMutexPend   : 获取互斥量（若被占用则挂起等待）
 */

void OSMutexCreate(OS_MUTEX *p_mutex, u8* p_name);	//创建互斥量
void OSMutexPost(OS_MUTEX *p_mutex);								//释放互斥量
void OSMutexPend(OS_MUTEX *p_mutex);								//挂起等待互斥量

#endif //OS_CFG_MUTEX_EN

/*--------------------------------------------------------------------------------------------------------
                                               消息队列组件
--------------------------------------------------------------------------------------------------------*/
#if OS_CFG_FLAG_EN != 0
/*
 * 事件标志组接口说明：
 * OSFlagCreate : 创建标志组并设置初始 flags
 * OSFlagPost   : 置位/清除某些标志位，并检查是否需要唤醒等待任务
 * OSFlagPend   : 等待某些标志位满足条件（不满足则挂起）
 * 注意: 始终消耗标志位
 */
void OSFlagCreate(OS_FLAG *p_flag, u8 *p_name, u32 init_flags);		//创建事件标志组
void OSFlagPost(OS_FLAG *p_flag, u32 mask, OS_FLAG_SET_OPT opt);	//释放标志位
void OSFlagPend(OS_FLAG *p_flag, u32 mask, OS_FLAG_PEDN_OPT opt);	//挂起等待并消耗标志位

#endif

#endif	//_CASY_OS_H_
