#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stm32f10x_tim.h"
#include "math.h"
#include "adc.h"
#include "fft.h"
#include "key.h"
#include "FilterBase.h"
#include "stm32_dsp.h"
#include "ringbuffer.h"

// RingBuffer
uint8_t buffer[500];
ring_buffer rb;
uint32_t num, i, Aindex;
// ADC
uint8_t ADC_ReadDone = 1; // ADC定时采样时间到标志
uint16_t ADC_ReadCount = 0;
uint16_t counter = 0;
uint8_t Quality = 0;
uint8_t get[NPT];
uint8_t j = 0;
float RBeta, lastRBeta, RAlpha, lastRAlpha;
float filter_White_Noise = 0;

// Filter
float bsf_out[NPT];
float hpf_out[NPT];
float lpf_out[NPT];

float outBeta;
float outAlpha;
bq_coeff_t bsf, hpf, lpf, bpfBeta, bpfAlpha;
float a, b, c;

float outArray[NPT],outArrayB[NPT];
float outBetaArray[NPT];
float outAlphaArray[NPT];

// FFT
sWavePwr_t wave, wave2;
long FFT_OUT[NPT];			  // FFT输出序列 高16位虚部,低16位实部
long FFT_IN[NPT];			  // FFT输入系列 高16位实部,低16位虚部
long FFT_Fmplitude[NPT / 2];  // 计算频点幅值
long FFT_FmplitudeA[NPT / 2]; // 计算频点幅值
long FFT_FmplitudeB[NPT / 2]; // 计算频点幅值

// 基准
float BaseSum, BaseSum2;

float np_mean(float *arr, int length)
{
	float sum = 0.0;
	int i;
	for ( i = 0; i < length; i++)
	{
		sum += arr[i];
	}
	return sum / length;
}

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); /*打开定时器2时钟*/
	NVIC_Configuration();								 // 定时器2中断优先级控制

	delay_init();	   // 延时函数初始化
	uart_init(115200); // 串口初始化为9600
	ADC1_Init();
	KEY_Init();
	printf("AT+NAME=Sichiray\r\n");
	delay_ms(10);
	printf("AT+NAMB=Sichiray\r\n");
	delay_ms(10);
	printf("AT+BAUD8\r\n");
	delay_ms(10);
	delay_ms(500);

	// RingBuffer
	RB_Init(&rb, buffer, 500);
	// Filter
	biquad_config(&bsf, bq_type_notch, 50, NPT, 0.5, 1); // 配置一个陷波器，FC 50Hz,采样频率250Hz，Q=0.7071，Gain=6
	biquad_config(&hpf, bq_type_highpass, 0.6, NPT, 0.5, 1);
	biquad_config(&lpf, bq_type_lowpass, 60, NPT, 0.5, 1);

	biquad_config(&bpfBeta, bq_type_bandpass, 24, NPT, 0.5, 1);	 // 配置一个高通滤波器，FC 0.6Hz,采样频率250Hz，Q=0.7071，Gain=6
	biquad_config(&bpfAlpha, bq_type_bandpass, 12, NPT, 0.5, 1); // 配置一个低通滤波器，FC35Hz,采样频率250Hz，Q=0.7071，Gain=6

	while (1)
	{
		if (ADC_ReadDone)
		{
			ADC_ReadDone = 0;
			filter_White_Noise = (float)(Get_Adc(ADC_Channel_7));
			a = biquad_process(&bsf, filter_White_Noise);
			b = biquad_process(&hpf, a);
			c = biquad_process(&lpf, b);

			outBeta = biquad_process(&bpfBeta, c);
			outAlpha = biquad_process(&bpfAlpha, c);

			//		printf("%d,%d,%d,%d  \r\n",(int)filter_White_Noise, (int)c,(int)outBeta,(int)outAlpha);

			outArray[Aindex] = c;
			outBetaArray[Aindex] = outBeta;
			outAlphaArray[Aindex] = outAlpha;
			Aindex++;

			if (Aindex == 255)
			{
				Aindex = 0; 
				GetPwrMag(FFT_OUT, FFT_Fmplitude);
				cr4_fft_256_stm32(FFT_OUT, outAlphaArray, NPT);
				GetPwrMag(FFT_OUT, FFT_FmplitudeA);
				cr4_fft_256_stm32(FFT_OUT, outBetaArray, NPT);
				GetPwrMag(FFT_OUT, FFT_FmplitudeB);

				for (i = 0; i < 127; i++)
				{
				printf("%ld,%ld,%ld\n", FFT_Fmplitude[i], FFT_FmplitudeA[i], FFT_FmplitudeB[i]);
				}
			}

			TIM_Cmd(TIM2, ENABLE);
		}
		//		 if (ADC_ReadDone)
		//		 {
		//		 	ADC_ReadDone = 0;

		// 	for (i = 0; i < NPT; i++)
		// 	{
		// 		bsf_out[i] = biquad_process(&bsf, (float)get[i]);
		// 		hpf_out[i] = biquad_process(&hpf, bsf_out[i]);
		// 		lpf_out[i] = biquad_process(&lpf, hpf_out[i]);
		// 		hpf_out2[i] = biquad_process(&hpfAlpha, bsf_out[i]);
		// 		lpf_out2[i] = biquad_process(&lpfAlpha, hpf_out2[i]);
		// 	}
		// 	cr4_fft_256_stm32(FFT_OUT, lpf_out, NPT);
		// 	GetPwrMag(FFT_OUT, FFT_Fmplitude);
		// 	wave2Pwr(FFT_Fmplitude, &wave);

		// 	cr4_fft_256_stm32(FFT_OUT, lpf_out2, NPT);
		// 	GetPwrMag(FFT_OUT, FFT_Fmplitude2);
		// 	wave2Pwr(FFT_Fmplitude2, &wave2);
		// 	TIM_Cmd(TIM2, ENABLE); // 一个循环结束关闭定时器

		// 	if (KEY0 == 0) // 已佩戴
		// 	{
		// 		RAlpha = (float)wave.Alpha / (float)(wave.Theta + wave.Alpha);
		// 		RBeta = (float)wave2.Beta / (float)(wave2.Theta + wave2.Beta);
		// 		if (j < 6)
		// 		{
		// 			j++;
		// 			// printf("j:%d\n", j);
		// 			if (j > 3)
		// 			{
		// 				BaseSum += RAlpha;
		// 				BaseSum2 += RBeta;
		// 			}
		// 		}
		// 		else
		// 		{
		// 			if (j == 6)
		// 			{
		// 				BaseSum = BaseSum / 4;
		// 				BaseSum2 = BaseSum2 / 4;
		// 				j = 255;
		// 			}

		// 			if (RAlpha <= BaseSum)
		// 				lastRAlpha = RAlpha * 50 / BaseSum;
		// 			else
		// 				lastRAlpha = (RAlpha - 2 * BaseSum + 1) * 50 / (1 - BaseSum);

		// 			if (RBeta <= BaseSum2)
		// 				lastRBeta = RBeta * 50 / BaseSum2;
		// 			else
		// 				lastRBeta = (RBeta + 1 - 2 * BaseSum2) * 50 / (1 - BaseSum2);
		// 		}
		// 	}
		// 	else
		// 	{
		// 		BaseSum = BaseSum2 = 0;
		// 		RBeta = RAlpha = 0;
		// 		j = 0;
		// 	}

		// 	printf(" %f %f\n", RBeta, RAlpha);
		// }
	}
}

void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	// filter_White_Noise = (float)(Get_Adc(ADC_Channel_7) / 16);
	// RB_Write_Byte(&rb, filter_White_Noise);

	// if (RB_Get_Length(&rb) < NPT)
	// {
	// 	ADC_ReadDone = 0; // 一个周期采样完成标志置0
	// }
	// else
	// {
	// 	TIM_Cmd(TIM2, DISABLE); // 一个循环结束关闭定时器
	// 	ADC_ReadDone = 1;		// 一个周期采样完成标志置1
	// }
	// if(ADC_ReadDone<2)
	ADC_ReadDone = 1;
}
