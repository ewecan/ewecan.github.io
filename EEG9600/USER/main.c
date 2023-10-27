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
uint8_t ADC_ReadDone = 1; // ADC��ʱ����ʱ�䵽��־
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
long FFT_OUT[NPT];			  // FFT������� ��16λ�鲿,��16λʵ��
long FFT_IN[NPT];			  // FFT����ϵ�� ��16λʵ��,��16λ�鲿
long FFT_Fmplitude[NPT / 2];  // ����Ƶ���ֵ
long FFT_FmplitudeA[NPT / 2]; // ����Ƶ���ֵ
long FFT_FmplitudeB[NPT / 2]; // ����Ƶ���ֵ

// ��׼
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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); /*�򿪶�ʱ��2ʱ��*/
	NVIC_Configuration();								 // ��ʱ��2�ж����ȼ�����

	delay_init();	   // ��ʱ������ʼ��
	uart_init(115200); // ���ڳ�ʼ��Ϊ9600
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
	biquad_config(&bsf, bq_type_notch, 50, NPT, 0.5, 1); // ����һ���ݲ�����FC 50Hz,����Ƶ��250Hz��Q=0.7071��Gain=6
	biquad_config(&hpf, bq_type_highpass, 0.6, NPT, 0.5, 1);
	biquad_config(&lpf, bq_type_lowpass, 60, NPT, 0.5, 1);

	biquad_config(&bpfBeta, bq_type_bandpass, 24, NPT, 0.5, 1);	 // ����һ����ͨ�˲�����FC 0.6Hz,����Ƶ��250Hz��Q=0.7071��Gain=6
	biquad_config(&bpfAlpha, bq_type_bandpass, 12, NPT, 0.5, 1); // ����һ����ͨ�˲�����FC35Hz,����Ƶ��250Hz��Q=0.7071��Gain=6

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
		// 	TIM_Cmd(TIM2, ENABLE); // һ��ѭ�������رն�ʱ��

		// 	if (KEY0 == 0) // �����
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
	// 	ADC_ReadDone = 0; // һ�����ڲ�����ɱ�־��0
	// }
	// else
	// {
	// 	TIM_Cmd(TIM2, DISABLE); // һ��ѭ�������رն�ʱ��
	// 	ADC_ReadDone = 1;		// һ�����ڲ�����ɱ�־��1
	// }
	// if(ADC_ReadDone<2)
	ADC_ReadDone = 1;
}
