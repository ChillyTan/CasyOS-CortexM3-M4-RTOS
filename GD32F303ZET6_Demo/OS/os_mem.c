/*********************************************************************************************************
* 模块名称: os_mem.c
* 摘    要: 内存管理模块
* 当前版本: 1.0.0
* 作    者: Chill
* 完成日期: 2026年01月15日
* 内    容:
*           (1) 使用静态数组作为内存池（s_OSMemoryPool）
*           (2) 使用单链表管理内存块，每个块由 “块头(OS_MEM_NODE) + 数据区” 组成
*           (3) 分配策略：首次适配 First-Fit（从链表头开始找第一个能用的空闲块）
*           (4) 支持块拆分（分配后剩余空间足够则拆成新空闲块）
*           (5) 支持释放时与前/后空闲块合并（减少碎片）
* 注    意:                                                                
*           (1) 该实现属于“简易内存管理”，不支持多内存区域、不支持双向链表
*           (2) OS_MEM_NODE->memUsedSize 高位存“占用标志”，低位存“块大小”
*           (3) 本模块所有接口都应在临界区内运行，避免任务切换造成链表破坏
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
#include "string.h"

#if OS_CFG_MEM_EN != 0u
/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define OS_MEM_ALIGN_UP(x)  		  (((x) + (OS_MEM_ALIGN - 1)) & ~(OS_MEM_ALIGN - 1))	//向上对齐
#define OS_MEM_USED_FLAG     			0x80000000	//内存块是否空闲标志位bit[31] 0-空闲 1-被占用
#define OS_MEM_SIZE_MASK     			0x7FFFFFFF	//用于设置该内存块空间大小
#define OS_MEM_NODE_SIZE		 			sizeof(OS_MEM_NODE)	//一个结点在内存池中占用的空间

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量定义
*********************************************************************************************************/
//内部SRAM内存池 32字节对齐(方便 DMA/FPU/Cache 对齐需求)
__align(32) static u8 s_OSMemoryPool[OS_MEM_MAX_SIZE] = {0};

//内存管理控制器
static OS_MEM_DEV s_OSMemoryDev =
{
  OSInitMemory,     //内存初始化函数指针
  OSMemoryPerused,  //内存使用率函数指针
  NULL,   					//内存池起始地址
  NULL,							//内存终止地址
	NULL,							//内存管理链表
  OS_MEM_NOT_READY	//内存管理未就绪
};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void* MallocMemory(u32 size);	//内存分配

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: MallocMemory
* 函数功能: 内存分配
* 输入参数: memx：所属内存块；size：要分配的内存大小（字节）
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月15日
* 注    意:
*           (1) 该函数不做参数合法性检查，由 OSMalloc() 统一检查
*           (2) 该函数不进入临界区，由 OSMalloc() 负责保证互斥
*           (3) 返回地址是 “块头后面的数据区”，即 (currNode + OS_MEM_NODE_SIZE)
*********************************************************************************************************/
static void* MallocMemory(u32 size)
{
  OS_MEM_NODE* currNode;
	OS_MEM_NODE* newNode;
  u32 reqMemSize;     //需要的内存大小(字节)
	u32 usedFlag;				//当前块是否使用标志
	u32 blockSize;			//当前块的大小
  
  reqMemSize = OS_MEM_ALIGN_UP(size);  //对齐
	currNode = s_OSMemoryDev.memoryList;
	
	while(currNode != NULL)
	{
		usedFlag = currNode->memUsedSize & OS_MEM_USED_FLAG;
		blockSize = currNode->memUsedSize & OS_MEM_SIZE_MASK;
		
		//若找到的块没有被使用且有足够空间
		if(usedFlag == 0 && blockSize >= reqMemSize)
		{
			//检查该block分配完需要的内存后 是否还有足够的空间 可以拆分出一个新的空闲块
			if(blockSize >= reqMemSize + OS_MEM_NODE_SIZE + OS_MEM_ALIGN)
			{
				newNode = (OS_MEM_NODE*)((u8*)currNode + OS_MEM_NODE_SIZE + reqMemSize);	//计算地址偏移以设置新分配的内存块
				newNode->memUsedSize = (blockSize - reqMemSize - OS_MEM_NODE_SIZE) & OS_MEM_SIZE_MASK;	//新拆分的块计算可用空间大小并标记为未使用
				newNode->memNextNode = currNode->memNextNode;	//新结点插入到当前结点之后
				currNode->memNextNode = newNode;
				currNode->memUsedSize = reqMemSize | OS_MEM_USED_FLAG;	//当前块标记为已使用并保存占用空间大小
			}
			else	//不需要拆分
			{
				currNode->memUsedSize |= OS_MEM_USED_FLAG;	//当前块标记为已使用
			}
			return (u8*)currNode + OS_MEM_NODE_SIZE;
		}
		currNode = currNode->memNextNode;
	}

  return NULL;	//未找到符合分配条件的内存块
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称: InitMemory
* 函数功能: 初始化Memory模块
* 输入参数: void
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月15日
* 注    意:
*           (1) 初始化后，链表中只有一个大空闲块（覆盖整个内存池数据区）
*           (2) 该函数必须在任何 OSMalloc/OSFree 调用之前执行
*********************************************************************************************************/
void OSInitMemory(void)
{
	OS_MEM_NODE *headNode;

	OS_ENTER_CRITICAL();

	//初始化内存池起始地址
	s_OSMemoryDev.memoryBase  = s_OSMemoryPool;
	s_OSMemoryDev.memoryEnd   = s_OSMemoryPool + OS_MEM_MAX_SIZE;

	//初始化第一个内存块结点
	headNode = (OS_MEM_NODE*)s_OSMemoryDev.memoryBase;
	//标记第一个内存块为未使用 大小为整个内存池减去该内存块头
	headNode->memUsedSize = (OS_MEM_MAX_SIZE - OS_MEM_NODE_SIZE) & OS_MEM_SIZE_MASK;
	headNode->memNextNode = NULL;

	//置位完成初始化标志
	s_OSMemoryDev.memoryList = headNode;
	s_OSMemoryDev.memoryRdy  = OS_MEM_READY;

	OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
* 函数名称: MemoryPerused
* 函数功能: 获得内存使用率
* 输入参数: void
* 输出参数: void
* 返 回 值: 使用率（扩大了10倍，0~1000，代表0.0%~100.0%）
* 创建日期: 2026年01月15日
* 注    意:
*           (1) 统计口径：所有“已占用块”的 (数据区大小 + 块头大小) 都算作 used
*           (2) total = memoryEnd - memoryBase（整个内存池大小）
*           (3) 返回值扩大10倍：例如 235 表示 23.5%
*********************************************************************************************************/
u16 OSMemoryPerused(void)
{
  OS_MEM_NODE* curr;
  u32 used;
	u32 total;

	OS_ENTER_CRITICAL();
	
	used = 0;
	total = s_OSMemoryDev.memoryEnd - s_OSMemoryDev.memoryBase;

	curr = s_OSMemoryDev.memoryList;
	while(curr != NULL)	//遍历整个内存管理链表
	{
		if((curr->memUsedSize & OS_MEM_USED_FLAG) != 0)	//累加所有被占用内存块的大小
		{
			used += (curr->memUsedSize & OS_MEM_SIZE_MASK) + OS_MEM_NODE_SIZE;
		}
		curr = curr->memNextNode;
	}

	OS_EXIT_CRITICAL();
	return (used * 1000) / total;  
}

/*********************************************************************************************************
* 函数名称: OSMalloc
* 函数功能: 内存分配
* 输入参数: memx：所属内存块；size：内存大小（字节）
* 输出参数: void
* 返 回 值: 分配到的内存首地址
* 创建日期: 2026年01月15日
* 注    意:
*           (1) 该函数是线程安全的（进入临界区）
*           (2) 分配失败会打印错误信息
*           (3) 分配成功返回的数据区首地址可直接写入用户数据
*********************************************************************************************************/
void* OSMalloc(u32 size)
{
  void* ptr;

	OS_ENTER_CRITICAL();

	//检查输入参数
	if(size == 0 || size >= OS_MEM_MAX_SIZE)
	{
		printf("ERROR: [OSMalloc] Illegal Argument!\r\n");
		OS_EXIT_CRITICAL();
		return NULL;
	}

	//检查内存池是否初始化
	if(s_OSMemoryDev.memoryRdy == 0)
	{
		printf("ERROR: MemoryPool Not Ready!\r\n");
		OS_EXIT_CRITICAL();
		return NULL;	
	}
	
	//检查内存是否分配成功
	ptr = MallocMemory(size);
	if(ptr == NULL || (u8*)ptr <= s_OSMemoryDev.memoryBase || (u8*)ptr >= s_OSMemoryDev.memoryEnd)
	{
		printf("ERROR: [OSMalloc]: Fail To Malloc!\r\n");
		OS_EXIT_CRITICAL();
		return NULL;
	}

	OS_EXIT_CRITICAL();
	return ptr;
}

/*********************************************************************************************************
* 函数名称: OSFree
* 函数功能: 内存释放
* 输入参数: memx：所属内存块；ptr：内存首地址 
* 输出参数: void
* 返 回 值: void
* 创建日期: 2026年01月15日
* 注    意: 
*           1) 释放后的数据不会清零，仅清除占用标志位
*           2) 释放后会尝试与相邻空闲块合并以减少碎片：
*              - 优先与后块合并
*              - 再与前块合并
*           3) ptr 必须是合法地址，否则可能导致链表破坏（当前未做严格校验）
*********************************************************************************************************/
void OSFree(void* ptr)
{
  OS_MEM_NODE* currNode;
	OS_MEM_NODE* nextNode;
	OS_MEM_NODE* prevNode;

	OS_ENTER_CRITICAL();

	if(ptr == NULL)
  {
		printf("Warning: Try to Free a NULL pointer!\r\n");
		OS_EXIT_CRITICAL();
    return;
  }
  
	//删除当前结点
	currNode = (OS_MEM_NODE*)((u8*)ptr - OS_MEM_NODE_SIZE);	//得到该内存块内存头
	currNode->memUsedSize &= OS_MEM_SIZE_MASK;	//清除内存块占用标志
	nextNode = currNode->memNextNode;
	
	//与后面的空闲内存块合并
	if(nextNode != NULL && (nextNode->memUsedSize & OS_MEM_USED_FLAG) == 0)	//后面还有内存块且内存块空闲
	{
		currNode->memUsedSize = (currNode->memUsedSize & OS_MEM_SIZE_MASK) + (nextNode->memUsedSize & OS_MEM_SIZE_MASK) + OS_MEM_NODE_SIZE;
		currNode->memNextNode = nextNode->memNextNode;
	}
	
	//与前面的空闲内存块合并
	prevNode = s_OSMemoryDev.memoryList;	//先遍历链表找到前驱结点内存块
	while(prevNode != NULL && prevNode->memNextNode != currNode)
	{
		prevNode = prevNode->memNextNode;
	}
	if(prevNode != NULL && (prevNode->memUsedSize & OS_MEM_USED_FLAG) == 0)	//前面还有内存块且内存块空闲
	{
		prevNode->memUsedSize = (prevNode->memUsedSize & OS_MEM_SIZE_MASK) + (currNode->memUsedSize & OS_MEM_SIZE_MASK) + OS_MEM_NODE_SIZE;
		prevNode->memNextNode = currNode->memNextNode;
	}
	
	//以上两个if都不满足则不合并
	OS_EXIT_CRITICAL();
}

#endif	//OS_CFG_MEM_EN
