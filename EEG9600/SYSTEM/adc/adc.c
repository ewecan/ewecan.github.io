#include "adc.h"

//FS = 256
//Ƶ�ʷֱ���256/256 = 1HZ

void TIM2_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	
	TIM_TimeBaseStructure.TIM_Period = 3999;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);        //��ʼ��
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2,ENABLE);
}

void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1	, ENABLE );	//ʹ��ADC1ͨ��ʱ��
	//AD����ʱ��72M/6=12M,���ܳ���14M�����򾫶��½�
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   
	//PA0--ADC_IN0  PA1--ADC_IN1  PA2--ADC_IN2  PA3--ADC_IN3  PA4--ADC_IN4  PA5--ADC_IN5  PA6--ADC_IN6  PA7--ADC_IN7  
	//PB0--ADC_IN8  PB1--ADC_IN9  PC0--ADC_IN10  PC1--ADC_IN11  PC2--ADC_IN12  PC3--ADC_IN13  PC4--ADC_IN14  PC5--ADC_IN15                                        
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;    //PA4����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//����Ϊģ������
	GPIO_Init(GPIOA, &GPIO_InitStructure);			 

  //------------------------------------ADC����--------------------------------------------------------
	ADC_DeInit(ADC1);  //������ ADC1��ȫ���Ĵ�������Ϊȱʡֵ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ���˴�Ϊ1��ͨ��
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	ADC_ResetCalibration(ADC1);	//����ָ����ADC1��У׼�Ĵ���
	while(ADC_GetResetCalibrationStatus(ADC1));	//��ȡADC1����У׼�Ĵ�����״̬,ֱ��У׼�Ĵ����������
	ADC_StartCalibration(ADC1);		//��ʼָ��ADC1��У׼״̬
	while(ADC_GetCalibrationStatus(ADC1));		//��ȡָ��ADC1��У׼����,ֱ��У׼���
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������
}


void ADC1_Init(void)
{
	Adc_Init();
	TIM2_Config();
}

//���ADCֵ
u16 Get_Adc(u8 ch)   
{	
	unsigned int result=0;
 //����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��ch,�������˳��ֵ����Ϊ1,����ʱ��Ϊ239.5����	  			    
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������
	result=ADC_GetConversionValue(ADC1);
	return result;	//����ƽ��ֵ
}




