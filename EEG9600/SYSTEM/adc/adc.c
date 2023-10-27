#include "adc.h"

//FS = 256
//频率分辨率256/256 = 1HZ

void TIM2_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	
	TIM_TimeBaseStructure.TIM_Period = 3999;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);        //初始化
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2,ENABLE);
}

void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1	, ENABLE );	//使能ADC1通道时钟
	//AD采样时钟72M/6=12M,不能超过14M，否则精度下降
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   
	//PA0--ADC_IN0  PA1--ADC_IN1  PA2--ADC_IN2  PA3--ADC_IN3  PA4--ADC_IN4  PA5--ADC_IN5  PA6--ADC_IN6  PA7--ADC_IN7  
	//PB0--ADC_IN8  PB1--ADC_IN9  PC0--ADC_IN10  PC1--ADC_IN11  PC2--ADC_IN12  PC3--ADC_IN13  PC4--ADC_IN14  PC5--ADC_IN15                                        
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;    //PA4引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//配置为模拟输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);			 

  //------------------------------------ADC设置--------------------------------------------------------
	ADC_DeInit(ADC1);  //将外设 ADC1的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目，此处为1个通道
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	ADC_ResetCalibration(ADC1);	//重置指定的ADC1的校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));	//获取ADC1重置校准寄存器的状态,直到校准寄存器设置完成
	ADC_StartCalibration(ADC1);		//开始指定ADC1的校准状态
	while(ADC_GetCalibrationStatus(ADC1));		//获取指定ADC1的校准程序,直到校准完成
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
}


void ADC1_Init(void)
{
	Adc_Init();
	TIM2_Config();
}

//获得ADC值
u16 Get_Adc(u8 ch)   
{	
	unsigned int result=0;
 //设置指定ADC的规则组通道，设置它们的转化顺序和采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道ch,规则采样顺序值序列为1,采样时间为239.5周期	  			    
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	result=ADC_GetConversionValue(ADC1);
	return result;	//返回平均值
}




