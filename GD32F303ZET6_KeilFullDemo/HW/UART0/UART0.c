/*********************************************************************************************************
* 模块名称：UART0.c
* 摘    要：串口模块，包括串口模块初始化，以及中断服务函数处理，以及读写串口函数实现
* 当前版本：1.0.0
* 作    者：Chill
* 完成日期：2026年01月31日 
* 内    容：
* 注    意：
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
#include "UART0.h"
#include "gd32f30x_conf.h"
#include "CirQueue.h"
#include "CasyOS.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/   
static StructCirQue  s_structUARTRecCirQue;       //接收串口循环队列
static unsigned char s_arrRecBuf[UART0_BUF_SIZE]; //接收串口循环队列的缓冲区
#if SEM_TEST != 0
extern OS_TASK_HANDLE g_structTestHandle3;  //测试消息队列
#endif
#if Q_TEST != 0
extern OS_TASK_HANDLE g_structTestHandle4;  //测试消息队列
#endif
#if FLAG_TEST != 0
extern OS_FLAG g_FlagTest;
#endif
// extern OS_TASK_HANDLE g_structTestHandle8;
// extern volatile u8 g_OSIntNestCnt;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigUART(unsigned int bound); //配置串口相关的参数，包括GPIO、RCU、USART和NVIC 

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigUART
* 函数功能：配置串口相关的参数，包括GPIO、RCU、USART和NVIC  
* 输入参数：bound，波特率
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static  void  ConfigUART(unsigned int bound)
{
  nvic_irq_enable(USART0_IRQn, 0, 0);  //使能串口中断，设置优先级
  rcu_periph_clock_enable(RCU_GPIOA);  //使能GPIOA时钟

  //配置TX的GPIO 
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
  
  //配置RX的GPIO
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

  //配置USART的参数
  rcu_periph_clock_enable(RCU_USART0);                  //使能串口时钟
  usart_deinit(USART0);                                 //RCU配置恢复默认值
  usart_baudrate_set(USART0, bound);                    //设置波特率
  usart_stop_bit_set(USART0, USART_STB_1BIT);           //设置停止位
  usart_word_length_set(USART0, USART_WL_8BIT);         //设置数据字长度
  usart_parity_config(USART0, USART_PM_NONE);           //设置奇偶校验位
  usart_receive_config(USART0, USART_RECEIVE_ENABLE);   //使能接收
  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE); //使能发送
  usart_enable(USART0);                                 //使能串口

  usart_interrupt_enable(USART0, USART_INT_RBNE);       //使能接收缓冲区非空中断

  //使能DMA发送
  usart_dma_transmit_config(USART0, USART_DENT_ENABLE);
}

/*********************************************************************************************************
* 函数名称：USART0_IRQHandler
* 函数功能：USART0中断服务函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void USART0_IRQHandler(void)
{
  unsigned char  uData = 0;

  OSIntEnter();

  if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET)  //接收缓冲区非空中断
  {                                                         
    usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);  //清除USART0中断挂起
    uData = usart_data_receive(USART0);                       //将USART0接收到的数据保存到uData
    EnCirQueue(&s_structUARTRecCirQue, &uData, 1);            //将接收到的数据写入接收缓冲区
#if SEM_TEST != 0
    OSSemPost(&g_structTestHandle3.sem);     // ISR里调用统一API
#endif
#if Q_TEST != 0
    OSQPost(&g_structTestHandle4, (u32)"Msg In UART_ISR\r\n");  //在中断中测试消息队列
#endif
#if FLAG_TEST != 0
    OSFlagPost(&g_FlagTest, (1 << 0) | (1 << 1) | (1 << 2), OS_FLAG_SET);
#endif
  }

  if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_ORERR) == SET) //溢出错误标志为1
  {                                                         
    usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR);       //清除溢出错误标志
    usart_data_receive(USART0);  //读取USART_DATA 
  }

  OSIntExit();
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitUART0
* 函数功能：初始化UART模块 
* 输入参数：bound,波特率
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitUART0(unsigned int bound)
{
  //初始化串口缓冲区，包括发送缓冲区和接收缓冲区
  InitCirQueue(&s_structUARTRecCirQue, s_arrRecBuf, UART0_BUF_SIZE);

  //配置串口相关的参数，包括GPIO、RCU、USART和NVIC
  ConfigUART(bound);
}

/*********************************************************************************************************
* 函数名称：WriteUART0
* 函数功能：写串口，即写数据到的串口发送缓冲区  
* 输入参数：pBuf，要写入数据的首地址，len，期望写入数据的个数
* 输出参数：void
* 返 回 值：成功写入数据的个数，不一定与形参len相等
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
unsigned int WriteUART0(unsigned char *pBuf, unsigned int len)
{
  //DMA初始化结构体
  dma_parameter_struct dma_init_struct;

  //使能DMA时钟
  rcu_periph_clock_enable(RCU_DMA0);
  
  //配置DMA
  dma_deinit(DMA0, DMA_CH3);                                      //初始化结构体设置默认值
  dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;        //设置DMA数据传输方向
  dma_init_struct.memory_addr  = (uint32_t)pBuf;                  //内存地址设置
  dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;      //内存增长使能
  dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;           //内存数据位数设置
  dma_init_struct.number       = len;                             //内存数据量设置
  dma_init_struct.periph_addr  = (uint32_t)&(USART_DATA(USART0)); //外设地址设置
  dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;     //外设地址增长失能
  dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;       //外设数据位数设置
  dma_init_struct.priority     = DMA_PRIORITY_MEDIUM;             //优先级设置
  dma_init(DMA0, DMA_CH3, &dma_init_struct);                      //根据参数初始化
  dma_circulation_disable(DMA0, DMA_CH3);                         //单次传输
  dma_memory_to_memory_disable(DMA0, DMA_CH3);                    //禁用内存到内存

  //开启DMA发送
  dma_channel_enable(DMA0, DMA_CH3);

  //等待DMA发送完成
  while(RESET == dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF)){}

  //返回实际写入数据的个数
  return len;
}

/*********************************************************************************************************
* 函数名称：ReadUART0
* 函数功能：读串口，即读取串口接收缓冲区中的数据  
* 输入参数：pBuf，读取的数据存放的首地址，len，期望读取数据的个数
* 输出参数：pBuf，读取的数据存放的首地址
* 返 回 值：成功读取数据的个数，不一定与形参len相等
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
unsigned int ReadUART0(unsigned char *pBuf, unsigned int len)
{
  return DeCirQueue(&s_structUARTRecCirQue, pBuf, len);
}

/*********************************************************************************************************
* 函数名称：fputc
* 函数功能：重定向函数
* 输入参数：ch，f
* 输出参数：void
* 返 回 值：int 
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
int fputc(int ch, FILE *f)
{
  usart_data_transmit(USART0, (uint8_t) ch);  //发送字符函数，专由fputc函数调用

  while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));

  return ch;
}

