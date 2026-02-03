/*********************************************************************************************************
* 模块名称：os_cfg.h
* 摘    要：CasyOS 配置文件（功能裁剪 + 移植参数）
* 版    本：1.0.0
* 作    者：Chill
* 日    期：2026-01-15
*
* 用    途：
*   1) 配置OS功能开关（内存/信号量/队列/互斥量/标志组）
*   2) 配置OS关键参数（优先级数量、内存池大小、对齐方式）
*
* 移植注意：
*   - 移植到其他MCU时，需替换/删除本文件中的芯片外设相关 include
*   - Cortex-M 推荐 OS_MEM_ALIGN=8（栈/异常帧对齐更安全）
*********************************************************************************************************/
#ifndef _OS_CFG_H_
#define _OS_CFG_H_

/*
* 需要替换为对应芯片的外设库文件 比如stm32f10x_conf.h
*/
#include "stm32f10x_conf.h"

/*
* 可选：仅用于printf调试打印，移植时可删除或替换 比如UART1.h 或任何包含printf实现的头文件
*/
#include "UART1.h"



#define OS_CFG_FPU_EN                     0           //1-使能FPU，0=关闭 某些芯片可能没有FPU相关寄存器 不允许启用该宏定义

#define OS_CFG_PRIO_MAX                   32          //最大优先级数量：0~31（当前实现固定32级）

#define OS_CFG_TASK_NAME_EN               1           //1=支持按任务名查找任务
#define OS_CFG_TASK_FUNC_EN               1           //1=支持按入口函数地址查找任务

#define OS_CFG_MEM_EN                     1           //1=启用内存管理，0=关闭
#define OS_MEM_MAX_SIZE                   20 * 1024   //内存池最大管理容量(字节) 默认为20K
#define OS_MEM_ALIGN                      8           //内存池对齐字节数(建议8，且必须为2的幂)

#define OS_CFG_SEM_EN                     1           //1=启用信号量组件

#define OS_CFG_Q_EN                       1           //1=启用消息队列组件（每条消息固定4字节u32）
#define OS_CFG_Q_FLUSH_EN                 1           //1=包含 OSQFlush()

#define OS_CFG_MUTEX_EN                   1           //1=启用互斥量组件

#define OS_CFG_FLAG_EN                    1           //1=启用事件标志组组件

/*--------------------------------------------------------------------------------------------------------
					示例工程main.c Demo/测试开关 (建议一个一个启用, 如果不需要可以将以下宏定义全部删掉)
					其中LED 和三个独立按键 需要根据开发板配置相应的引脚
--------------------------------------------------------------------------------------------------------*/
#define LED_TEST                          1
#define FPU_TEST                          1
#define MEM_TEST                          1
#define SEM_TEST                          1
#define Q_TEST                            1
#define MUTEX_TEST                        1
#define FLAG_TEST                         1

#endif  //_OS_CFG_H_
