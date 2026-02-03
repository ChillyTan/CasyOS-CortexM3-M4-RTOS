/*********************************************************************************************************
* 模块名称：KeyOne.c
* 摘    要：KeyOne模块，进行独立按键初始化，以及按键扫描函数实现
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
#include "KeyOne.h"
#include "gd32f30x_conf.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//KEY1为读取PA0引脚电平
#define KEY1    (gpio_input_bit_get(GPIOA, GPIO_PIN_0)) 
//KEY2为读取PG13引脚电平
#define KEY2    (gpio_input_bit_get(GPIOG, GPIO_PIN_13)) 
//KEY3为读取PG14引脚电平
#define KEY3    (gpio_input_bit_get(GPIOG, GPIO_PIN_14))  

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//按键按下时的电平，0xFF表示按下为高电平，0x00表示按下为低电平
static  unsigned char  s_arrKeyDownLevel[KEY_NAME_MAX];      //使用前要在InitKeyOne函数中进行初始化 

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  ConfigKeyOneGPIO(void); //配置按键的GPIO 

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigKeyOneGPIO
* 函数功能：配置按键的GPIO 
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：
*********************************************************************************************************/
static  void  ConfigKeyOneGPIO(void)
{
  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_GPIOA); //使能GPIOA的时钟
  rcu_periph_clock_enable(RCU_GPIOG); //使能GPIOG的时钟
  
  gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_0);         //配置PA0为下拉输入
  gpio_init(GPIOG, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_13);        //配置PG13为上拉输入
  gpio_init(GPIOG, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_14);        //配置PG14为上拉输入
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitKeyOne
* 函数功能：初始化KeyOne模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2026年01月31日
* 注    意：
*********************************************************************************************************/
void InitKeyOne(void)
{
  ConfigKeyOneGPIO(); //配置按键的GPIO 
                                                                
  s_arrKeyDownLevel[KEY_NAME_KEY1] = KEY_DOWN_LEVEL_KEY1;  //按键KEY1按下时为高电平
  s_arrKeyDownLevel[KEY_NAME_KEY2] = KEY_DOWN_LEVEL_KEY2;  //按键KEY2按下时为低电平
  s_arrKeyDownLevel[KEY_NAME_KEY3] = KEY_DOWN_LEVEL_KEY3;  //按键KEY3按下时为低电平
}

/*********************************************************************************************************
* 函数名称：ScanKeyOne
* 函数功能：按键扫描，每10ms调用一次
* 输入参数：keyName-按键名，OnKeyOneUp-按键弹起响应函数的指针，OnKeyOneDown-按键按下响应函数的指针
* 输出参数：void
* 返 回 值：1-按键按下，0-无效值
* 创建日期：2026年01月31日
* 注    意：如果s_arrKeyDownLevel[keyName] = 0xFF，对s_arrKeyDownLevel[keyName]直接取反得出的是256，而非0
*           正确的做法是(unsigned char)(~s_arrKeyDownLevel[keyName])，这样得出的才是0。
*********************************************************************************************************/
u32 ScanKeyOne(unsigned char keyName, void(*OnKeyOneUp)(void), void(*OnKeyOneDown)(void))
{
  static  unsigned char  s_arrKeyVal[KEY_NAME_MAX];    //定义一个unsigned char类型的数组，用于存放按键的数值
  static  unsigned char  s_arrKeyFlag[KEY_NAME_MAX];   //定义一个unsigned char类型的数组，用于存放按键的标志位
  
  s_arrKeyVal[keyName] = s_arrKeyVal[keyName] << 1;   //左移一位

  switch (keyName)
  {
    case KEY_NAME_KEY1:
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY1; //按下/弹起时，KEY1为1/0
      break;                                            
    case KEY_NAME_KEY2:                                 
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY2; //按下/弹起时，KEY2为0/1
      break;                                            
    case KEY_NAME_KEY3:                                 
      s_arrKeyVal[keyName] = s_arrKeyVal[keyName] | KEY3; //按下/弹起时，KEY3为0/1
      break;                                            
    default:
      break;
  }
  
  //按键标志位的值为TRUE时，判断是否有按键有效按下
  if(s_arrKeyVal[keyName] == s_arrKeyDownLevel[keyName] && s_arrKeyFlag[keyName] == TRUE)
  {
    //执行按键按下的响应函数
    if(NULL != OnKeyOneDown)
    {
      (*OnKeyOneDown)();
    }

    //表示按键处于按下状态，按键标志位的值更改为FALSE
    s_arrKeyFlag[keyName] = FALSE;

    //表示有按键按下
    return 1;
  }
  
  //按键标志位的值为FALSE时，判断是否有按键有效弹起
  else if(s_arrKeyVal[keyName] == (unsigned char)(~s_arrKeyDownLevel[keyName]) && s_arrKeyFlag[keyName] == FALSE)
  {
    //执行按键弹起的响应函数
    if(NULL != OnKeyOneUp)
    {
      (*OnKeyOneUp)();
    }

    //表示按键处于弹起状态，按键标志位的值更改为TRUE
    s_arrKeyFlag[keyName] = TRUE;
  }

  return 0;
}
