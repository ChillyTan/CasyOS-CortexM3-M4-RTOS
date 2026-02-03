--- 
## ✨ 简介
一个面向 Cortex-M3/M4 的轻量级简易 RTOS(学习/教学/自研内核实验用)

> CasyOS 是一个用于学习 RTOS 内核原理的及其简易的轻量级实时操作系统。
> 目前支持 Cortex-M（带 PendSV/SVC/SysTick）平台的基础抢占式调度、任务延时、内建 IPC（信号量 / 消息队列 / 事件标志组 / 互斥量）等功能。

---
## 🎯 适用场景

- 学习 RTOS 的核心机制：**调度器 / 上下文切换 / PendSV / SysTick / 临界区 / IPC**
- 课程设计、比赛项目的简易内核
- 裸机项目希望引入“轻量任务管理”但不想用大型 RTOS 的情况

> ⚠️ 注意：本项目主要定位为**学习型 RTOS**，不以工业级完整性为目标（例如：时间片、多核、MPU隔离、完善的错误码体系等）。

---
## 📁 项目结构（示例）
```text
CasyOS/
├─ os_cfg.h            # 用户可以修改此文件来裁剪功能
├─ CasyOS.h            # 所有API声明在同一个头文件中
├─ CasyOS.c            # 内核核心：调度、异常、上下文切换
├─ os_task.c           # 任务管理/就绪队列/优先级位图
├─ os_tick.c           # Tick 延时链表与 tick 更新
├─ os_pend.c           # PendList 通用挂起队列操作
├─ os_mem.c            # 内存管理（若启用）
├─ os_sem.c            # 信号量
├─ os_mutex.c          # 互斥量
├─ os_q.c              # 消息队列
└─ os_flag.c           # 事件标志组
```
> 为了方便、简易，这里不做复杂的文件结构

--- 
## 🧱 系统架构总览 Architecture

CasyOS 的核心由三部分组成：
1. **异常驱动的任务切换机制**
    - SysTick：产生 OS Tick（节拍）
    - PendSV：完成上下文切换
    - SVC：启动第一个任务
2. **调度器 Scheduler**
    - 维护“就绪任务集合”
    - 选择最高优先级任务运行
    - 在任务态：立即触发 PendSV
    - 在中断态：置位调度标志，统一在 OSIntExit 触发 PendSV
3. **内核对象 Kernel Objects（IPC）**
    - 信号量 / 互斥量 / 消息队列 / 事件标志组
    - 每个对象都维护自己的挂起队列（Pend List）
    - 唤醒时遵循“优先级最高任务优先”原则

---
## 🔁 核心流程 Core Flow

### 1) 系统启动流程
```text
OSStart()
 ├─ 注册 IdleTask（最低优先级）
 ├─ 配置 SysTick = 1ms
 ├─ 配置异常优先级（PendSV 最低）
 └─ 触发 SVC -> 启动第一个任务
```
### 2) Tick 驱动流程（SysTick）
```text
SysTick_Handler()
 ├─ OSIntEnter()
 ├─ OS_TickUpdate()      // 延时任务 tick 递减
 ├─ OS_Sched()           // 请求调度
 └─ OSIntExit()          // 统一触发 PendSV
```
### 3) 任务切换流程（PendSV）
```text
PendSV_Handler()
 ├─ 保存当前任务上下文（R4-R11 + S16-S31 + LR）
 ├─ 保存 SP 到当前 TCB
 ├─ OS_UpdateCurrentTask()  // 选择最高优先级任务
 ├─ 恢复下一个任务 SP
 └─ 恢复下一个任务上下文并返回
```

---
## 🧩 任务模型 Task Model

### 任务控制块（TCB）
每个任务由 `OS_TASK_HANDLE` 描述，核心字段包括：
- `stackTop / stackBase / stackSize`：任务栈信息
- `priority`：优先级
- `state`：任务状态（READY / PEND / DELAY 等）
- `tick`：延时计数
- `pendObj`：当前挂起等待的对象指针
- `rdyNextPtr/rdyPrevPtr`：就绪链表指针
- `tickNextPtr/tickPrevPtr`：延时链表指针
- **内建对象**（可选）：
	- `sem`（信号量）
	- `msgQueue`（消息队列）

### 任务状态（简化）
```text
READY  : 可运行（在就绪队列）
PEND   : 等待某对象（在对象挂起队列）
DELAY  : 延时中（在延时队列）
```
---
## 🧠 调度策略 Scheduling Policy

- **固定优先级抢占式调度**
- 调度触发来源：
    - SysTick Tick 到来
    - IPC Post 导致更高优先级任务就绪
    - 任务主动 Delay / Pend
- “中断中不直接切换任务”原则：
    - ISR 内只置位 `g_OSSchedFlag`
    - 统一由 `OSIntExit()` 触发 PendSV

---
## 🧵 临界区设计 Critical Section

CasyOS 使用 `BASEPRI` 实现临界区：
- `OS_ENTER_CRITICAL()`：设置 BASEPRI 屏蔽部分中断
- `OS_EXIT_CRITICAL()`：恢复 BASEPRI
> 当前版本：临界区尚未实现嵌套计数（可扩展为保存/恢复 BASEPRI 的栈式结构）。

---
## 📦 IPC 设计框架 IPC Design

### 1) 挂起队列 Pend List
CasyOS 的同步对象采用统一挂起机制：
- 任务在等待资源时：
    - 从就绪队列移除
    - 插入对象的挂起队列
    - `state = OS_TASK_PEND`
    - `pendObj = 对象指针`
- 资源释放时：
    - 从挂起队列中选出**最高优先级任务**
    - 移出挂起队列并加入就绪队列
    - 触发调度（必要时抢占）
### 2) 信号量 OS_SEM
- 支持二值/计数语义（基于计数值）
- `Post`：
    - 若有等待者 -> 唤醒最高优先级等待任务
    - 否则计数加 1（不超过上限）
- `Pend`：
    - 若计数>0 -> 直接获取并返回
    - 否则挂起当前任务
### 3) 互斥量 OS_MUTEX（支持递归）
- `ownerTcb`：当前持有者
- `lockCnt`：递归加锁次数
- `Pend`：
    - 空闲 -> 直接获得所有权
    - owner 是自己 -> lockCnt++
    - 被别人占用 -> 挂起
- `Post`：
    - lockCnt-- 到 0 才真正释放
    - 若有等待者 -> 转移所有权给最高优先级任务

> 当前版本未实现优先级继承（Priority Inheritance），后续可扩展。
### 4) 消息队列 OS_Q（内建环形缓冲区）
- 每个任务可拥有自己的 `msgQueue`
- 发送为 **4 字节消息**（u32）
- `Post`：
    - 若目标任务正等待该队列 -> 直接投递到 TCB 临时缓存并唤醒
    - 否则写入环形队列
- `Pend`：
    - 若队列非空 -> 取出返回
    - 否则挂起当前任务等待消息
### 5) 事件标志组 OS_FLAG
- `flags`：32 位标志寄存器
- 支持等待模式：
    - SET_ALL / SET_ANY
    - CLR_ALL / CLR_ANY
- `Post`：
    - 修改 flags
    - 若挂起队列存在匹配任务 -> 唤醒最高优先级匹配者
    - 对 SET 等待者支持“消耗标志位”（自动清零）

---
## 🗺️ Roadmap（可扩展方向，尚未实现）
-  临界区嵌套支持（BASEPRI 保存/恢复）
-  优先级继承（Mutex PI）
-  时间片轮转（同优先级多任务）
-  软件定时器（Timer）
-  事件队列/发布订阅机制
-  更完善的错误码体系（OS_ERR_xxx）
-  Trace/统计（CPU 利用率、上下文切换次数）

---
## 📜 License
**MIT License**

---
## 🙌 参考
- ARM Cortex-M 异常机制：SysTick / SVC / PendSV
- 参考学习：uC/OS、FreeRTOS 等经典 RTOS 的设计思想

---
## 📬 联系方式
作者：Chill  邮箱： 2540483197@qq.com
欢迎提交 Issue / PR，一起完善这个学习型 RTOS ✨
