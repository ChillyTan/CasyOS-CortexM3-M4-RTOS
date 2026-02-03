1. **加入源码**
    - 在基准工程上，把 `CasyOS` 所有 `.c/.h` 文件加入工程
    - 配好头文件路径（Include Path）
2. **配置 cfg 头文件**
    - `include` 芯片外设库配置头文件（如 `stm32f10x_conf.h / gd32f30x_conf.h`）
    - （可选）`include` 已自行实现 `printf` 的头文件（用于Demo调试输出）
    - 无 FPU 的芯片：`OS_CFG_FPU_EN = 0`
    - 按 SRAM 大小调整内存池：`OS_MEM_MAX_SIZE`
    - 按需用宏裁剪功能
    - （可选）打开 Demo 测试宏
3. **编译器设置**
    - Keil 选择 **Version 5**
    - C/C++ 建议开启 **C99**
4. **接管中断入口（必须）**
    - 重定向/接入以下三个函数到 CasyOS：
        - `SVC_Handler()`
        - `PendSV_Handler()`
        - `SysTick_Handler()`
    - 处理方法：把工程里其他文件的同名函数注释掉，或改为转调 CasyOS
    - 通过“重复定义/缺少定义”的编译报错逐个解决即可
5. **编写 main 并启动系统**
    - 可直接复制 Demo 的 `main.c` 进行测试
    - 上电初始化后调用：`InitCasyOS()`
6. **全功能用法CasyOS_FullDemo**
	- 见 `STM32F103RCT6_FullDemo` 或 `GD32F303ZET6_FullDemo`

## 🧪 调试建议 Debug Tips
- 建议保留串口日志接口（如 `printf`）用于观察：
    - 任务切换是否发生
    - `SVC_Handler()` `PendSV_Handler()` `SysTick_Handler()` 三个函数是否接入CasyOS
    - Pend/Post 是否正确唤醒任务
    - 队列是否溢出
- 发生 HardFault 时：
    - 检查任务栈是否溢出（stack size）
    - 检查 PendSV/SVC 中的 SP 保存恢复是否正确
    - 检查临界区是否遗漏 `OS_EXIT_CRITICAL()`