#include "fft.h"
#include "stm32_dsp.h"
#include "adc.h"
#include "delay.h"
#include "math.h"
#include "usart.h"
#include "stm32f10x_tim.h"
extern u8 send_enable;
extern u16 main_loop;
extern u8 ADC_ReadDone;
extern uint8_t Quality;

u16 Checksum = 0;	 // У���
u16 Delta = 0;		 // Delta��0~4Hz��ʵ��2~4
u16 Theta = 0;		 // Theta��4~8Hz��ʵ��5~8
u16 LowAlpha = 0;	 // ��Alpha��8~10Hz��ʵ��9~10
u16 HighAlpha = 0;	 // ��Alpha��11~12
u16 LowBeta = 0;	 // ��Beta��12~26��ʵ��13~26
u16 HighBeta = 0;	 // ��Beta��27~40
u16 LowGama = 0;	 // ��Gama��40~60��ʵ��41~60
u16 MiddleGamma = 0; // ��Gama��61~80

u8 Attention = 0;  // רע�ȣ�0~100
u8 Meditation = 0; // ���ɶȣ�0~100

u16 Alpha = 0;
u16 MaxAlpha = 0;
u16 MinAlpha = 0xffff;
u16 Beta = 0;
u16 MaxBeta = 0;
u16 MinBeta = 0xffff;
u8 send_once = 0;
float LOD = 0.0;

u8 LOD_Data = 0x00;

u16 Alpha_Data[10];
u16 Beta_Data[10];
u8 Send_Data[36] = {
	0xaa, 0xaa, 0x20, 0x02, 0x00, 0x83, 0x18,
	0x00, 0x00, 0x00, // Delta
	0x00, 0x00, 0x00, // Theta
	0x00, 0x00, 0x00, // LowAlpha
	0x00, 0x00, 0x00, // HighAlpha
	0x00, 0x00, 0x00, // LowBeta
	0x00, 0x00, 0x00, // HighBeta
	0x00, 0x00, 0x00, // LowGamma
	0x00, 0x00, 0x00, // MiddleGamma
	0x04, 0x00,		  // רע��
	0x05, 0x00,		  // ���ɶ�
	0x00			  // У���
};

/*
����Ƶ�ʣ�FS
�ź�Ƶ�ʣ�F
����������N
�źŷ�ֵ��A
FS>=2F  : ���ݲ������������ʱ�������ź�Ƶ�ʵ�2��
N���Ƶ��Ϊ��Fn=(N-1)*Fs/N
*/
/*-----------��������----------------*/
// long FFT_OUT[NPT];                //FFT������� ��16λ�鲿����16λʵ��
// long FFT_IN[NPT];                 //FFT����ϵ�� ��16λʵ������16λ�鲿
// long FFT_Fmplitude[NPT/2];			  //����Ƶ���ֵ
// long White_Noise=0;               //Ӳ���˷ŵĵ���

// ��ȡ�˷ŵ��뺯��������Ӳ�������Լ�����������
// void FFT_Start_Initialization(void)
//{
//	u8 i=0;
//	for(i=0;i<50;i++)
//	{
//		White_Noise=Get_Adc(ADC_Channel_0)+White_Noise;
//		delay_ms(10);
//	}
//	White_Noise=White_Noise/50;//�����ɼ�50��ȡƽ��ֵ
// }

// ���йٷ��⽫ADC�ɼ��������ݽ���FFT�任
// void FFT_Running(void)
//{
//	unsigned short i;
//	u8 j;

//	//��ʼFFTת��
//	cr4_fft_256_stm32(FFT_OUT,FFT_IN,NPT);
//	GetPowerMag();//����Ƶ���ֵ
//	TIM_Cmd(TIM2, ENABLE);//һ��ѭ����ʼ �򿪶�ʱ��
//

// 0.5-3
//	Delta = FFT_Fmplitude[1]+FFT_Fmplitude[2]+FFT_Fmplitude[3];

// 4-7
//	Theta = FFT_Fmplitude[4]+FFT_Fmplitude[5]+FFT_Fmplitude[6]+FFT_Fmplitude[7];

// 8-13
//	LowAlpha = FFT_Fmplitude[8]+FFT_Fmplitude[9];
//	HighAlpha = FFT_Fmplitude[10]+FFT_Fmplitude[11];

// 14-30
//	LowBeta = FFT_Fmplitude[12]+FFT_Fmplitude[13]+FFT_Fmplitude[14]+FFT_Fmplitude[15]
//						+FFT_Fmplitude[16]+FFT_Fmplitude[17]+FFT_Fmplitude[18]+FFT_Fmplitude[19]
//						+FFT_Fmplitude[20]+FFT_Fmplitude[21]+FFT_Fmplitude[22]+FFT_Fmplitude[23]
//						+FFT_Fmplitude[24]+FFT_Fmplitude[25];
//	HighBeta = FFT_Fmplitude[26]+FFT_Fmplitude[27]+FFT_Fmplitude[28]+FFT_Fmplitude[29]
//						+FFT_Fmplitude[30]+FFT_Fmplitude[31]+FFT_Fmplitude[32]+FFT_Fmplitude[33]
//						+FFT_Fmplitude[34]+FFT_Fmplitude[35]+FFT_Fmplitude[36]+FFT_Fmplitude[37]
//						+FFT_Fmplitude[38]+FFT_Fmplitude[39];

// 40-80  31-50
//	LowGama =  FFT_Fmplitude[40]+FFT_Fmplitude[41]+FFT_Fmplitude[42]+FFT_Fmplitude[43]
//						+FFT_Fmplitude[44]+FFT_Fmplitude[45]+FFT_Fmplitude[46]+FFT_Fmplitude[47]
//						+FFT_Fmplitude[48]+FFT_Fmplitude[49]+FFT_Fmplitude[50]+FFT_Fmplitude[51]
//						+FFT_Fmplitude[52]+FFT_Fmplitude[53]+FFT_Fmplitude[54]+FFT_Fmplitude[55]
//						+FFT_Fmplitude[56]+FFT_Fmplitude[57]+FFT_Fmplitude[58]+FFT_Fmplitude[59];
//	MiddleGamma = FFT_Fmplitude[60]+FFT_Fmplitude[61]+FFT_Fmplitude[62]+FFT_Fmplitude[63]
//						+FFT_Fmplitude[64]+FFT_Fmplitude[65]+FFT_Fmplitude[66]+FFT_Fmplitude[67]
//						+FFT_Fmplitude[68]+FFT_Fmplitude[69]+FFT_Fmplitude[70]+FFT_Fmplitude[71]
//						+FFT_Fmplitude[72]+FFT_Fmplitude[73]+FFT_Fmplitude[74]+FFT_Fmplitude[75]
//						+FFT_Fmplitude[76]+FFT_Fmplitude[77]+FFT_Fmplitude[78]+FFT_Fmplitude[79];
//
//
//	/*-----------רע�ȼ���----------------*/
//	Alpha = (LowAlpha+HighAlpha);
//
//	for(j = 0;j<9;j++)
//	{
//		Alpha_Data[j] = Alpha_Data[j+1];
//	}
//	Alpha_Data[9] = Alpha;
//	MinAlpha =0xffff;
//	MaxAlpha = 0;
//	for(j = 0;j<10;j++)
//	{
//		if(Alpha_Data[j]<MinAlpha)
//		{
//			MinAlpha = Alpha_Data[j];
//		}
//		else if(Alpha_Data[j]>MaxAlpha)
//		{
//			MaxAlpha = Alpha_Data[j];
//		}
//	}
//	Attention = (((Alpha-MinAlpha)*100/(MaxAlpha-MinAlpha))+Attention)/2;	//��һ��
//
//	/*-----------���ɶȼ���----------------*/
//	Beta = (LowBeta+HighBeta);
//	for(j = 0;j<9;j++)
//	{
//		Beta_Data[j] = Beta_Data[j+1];
//	}
//	Beta_Data[9] = Beta;
//	MinBeta =0xffff;
//	MaxBeta = 0;
//	for(j = 0;j<10;j++)
//	{
//			if(Beta_Data[j]<MinBeta)
//			{
//				MinBeta = Beta_Data[j];
//			}
//			else if(Beta_Data[j]>MaxBeta)
//			{
//				MaxBeta = Beta_Data[j];
//			}
//	}
//	Meditation = (((Beta-MinBeta)*100/(MaxBeta-MinBeta))+Meditation)/2;	//��һ��
//
//	Send_Data[8] = Delta>>8;
//	Send_Data[9] = Delta;
//	Send_Data[11] = Theta>>8;
//	Send_Data[12] = Theta;
//	Send_Data[14] = LowAlpha>>8;
//	Send_Data[15] = LowAlpha;
//	Send_Data[17] = HighAlpha>>8;
//	Send_Data[18] = HighAlpha;
//	Send_Data[20] = LowBeta>>8;
//	Send_Data[21] = LowBeta;
//	Send_Data[23] = HighBeta>>8;
//	Send_Data[24] = HighBeta;
//	Send_Data[26] = LowGama>>8;
//	Send_Data[27] = LowGama;
//	Send_Data[29] = MiddleGamma>>8;
//	Send_Data[30] = MiddleGamma;
//	Send_Data[32] = Attention;
//	Send_Data[34] = Meditation;
//
//	if(KEY0 == 0)
//	{
//		Send_Data[4] = 0x00;
//	}
//	else
//	{
//		Send_Data[4] = 0xC8;
//		Send_Data[32] = 0;
//		Send_Data[34] = 0;
//	}
//
//	 Checksum = 0;
//

//	for(j = 0;j<32;j++)
//	{
//		Checksum +=Send_Data[j+3];
//	}
//	Checksum = (~Checksum)&0xff;
//	Send_Data[35] = Checksum;
//	for(i = 0;i<36;i++)
//	{
//		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
//		USART_SendData(USART1, (uint8_t) Send_Data[i]);
//	}
//
////		printf("A:%ld,B:%ld,C:%ld,D:%ld,E:%ld,F:%ld,G:%ld,H:%ld,I:%ld,J:%ld,K:%ld,L:%ld\r\n",
////		FFT_Fmplitude[1],FFT_Fmplitude[3],FFT_Fmplitude[5],FFT_Fmplitude[7],FFT_Fmplitude[9],FFT_Fmplitude[11],
////		FFT_Fmplitude[13],FFT_Fmplitude[15],FFT_Fmplitude[17],FFT_Fmplitude[19],FFT_Fmplitude[21],FFT_Fmplitude[23]);
//
//}

/*
����Ƶ���ֵ+��λ
�Ƚ�lBUFOUT�ֽ��ʵ��(X)���鲿(Y)��
Ȼ������ֵ(sqrt(X*X+Y*Y)
*/
// void GetPowerMag(void)
//{
//	signed short lX,lY;
//	float X,Y,Mag;
//	unsigned short i;
//
//	for (i=0; i < NPT/2; i++) //ʵ��ӦΪNPT/2 �Գ���
//	{
//		// ��λ�Ժ���û�иı�,ֻ����λ�Ӹ�ֵ�Ż�ı�ԭ�������������
//
//		lX= (FFT_OUT[i]<<16)>>16;		/* sine_cosine --> cos */ //ʵ��
//		lY= (FFT_OUT[i] >> 16);  		/* sine_cosine --> sin */ //�鲿

//		X=  NPT*((float)lX)/32768.0; //ʵ��
//		Y = NPT*((float)lY)/32768.0; //�鲿
//		//  ��ֵ����
//		Mag = sqrt(X*X+ Y*Y)/NPT;  // ��ƽ����,�ٿ���
//		if(i==0)
//			FFT_Fmplitude[i] = (unsigned long)(Mag*32768.0);
//		else
//			FFT_Fmplitude[i] = (unsigned long)(Mag*65536.0);
//	}
//}

void GetPwrMag(long FFTOut[], long FFTFmplitude[])
{
	signed short lX, lY;
	float X, Y, Mag;
	unsigned short i;
	for (i = 0; i < NPT / 2; i++)								// ʵ��ӦΪNPT/2 �Գ���
	{															// ��λ�Ժ���û�иı䣬ֻ����λ�Ӹ�ֵ�Ż�ı�ԭ�������������
		lX = (FFTOut[i] << 16) >> 16; /* sine_cosine --> cos */ // ʵ��
		lY = (FFTOut[i] >> 16); /* sine_cosine --> sin */		// �鲿

		X = NPT * ((float)lX) / 32768.0; // ʵ��
		Y = NPT * ((float)lY) / 32768.0; // �鲿
		//  ��ֵ����
//		Mag = sqrt(X * X + Y * Y) / NPT; // ��ƽ����,�ٿ���
//		if (i == 0)
//			FFTFmplitude[i] = (unsigned long)(Mag * 32768.0);
//		else
//			FFTFmplitude[i] = (unsigned long)(Mag * 65536.0);
			Mag = X * X;
			if (i == 0)
				FFTFmplitude[i] = 0;
			else
				FFTFmplitude[i] = (unsigned long)(Mag );
		
	}
}

void wave2Pwr(long FFTFmplitude[], sWavePwr_t *wave)
{

	// 0.5-3
	wave->Delta = FFTFmplitude[1] +
				  FFTFmplitude[2] * FFTFmplitude[2] / 2 +
				  FFTFmplitude[3] * FFTFmplitude[3] / 3;

	// 4-7
	wave->Theta = FFTFmplitude[4] * FFTFmplitude[4] / 4 +
				  FFTFmplitude[5] * FFTFmplitude[5] / 5 +
				  FFTFmplitude[6] * FFTFmplitude[6] / 6 +
				  FFTFmplitude[7] * FFTFmplitude[7] / 7;

	// 8-9
	wave->lowAlpha = FFTFmplitude[8] * FFTFmplitude[8] / 8 +
					 FFTFmplitude[9] * FFTFmplitude[9] / 9;

	wave->highAlpha = FFTFmplitude[10] * FFTFmplitude[10] / 10 +
					  FFTFmplitude[11] * FFTFmplitude[11] / 11;
	// 8-13
	wave->Alpha = FFTFmplitude[8] * FFTFmplitude[8] / 8 +
				  FFTFmplitude[9] * FFTFmplitude[9] / 9 +
				  FFTFmplitude[10] * FFTFmplitude[10] / 10 +
				  FFTFmplitude[11] * FFTFmplitude[11] / 11;

	wave->lowBeta = FFTFmplitude[12] * FFTFmplitude[12] / 12 +
					FFTFmplitude[13] * FFTFmplitude[13] / 13 +
					FFTFmplitude[14] * FFTFmplitude[14] / 14 +
					FFTFmplitude[15] * FFTFmplitude[15] / 15 +
					FFTFmplitude[16] * FFTFmplitude[16] / 16 +
					FFTFmplitude[17] * FFTFmplitude[17] / 17 +
					FFTFmplitude[18] * FFTFmplitude[18] / 18 +
					FFTFmplitude[19] * FFTFmplitude[19] / 19 +
					FFTFmplitude[20] * FFTFmplitude[20] / 20 +
					FFTFmplitude[21] * FFTFmplitude[21] / 21 +
					FFTFmplitude[22] * FFTFmplitude[22] / 22 +
					FFTFmplitude[23] * FFTFmplitude[23] / 23 +
					FFTFmplitude[24] * FFTFmplitude[24] / 24 +
					FFTFmplitude[25] * FFTFmplitude[25] / 25;
	wave->highBeta = FFTFmplitude[26] * FFTFmplitude[26] / 26 +
					 FFTFmplitude[27] * FFTFmplitude[27] / 27 +
					 FFTFmplitude[28] * FFTFmplitude[28] / 28 +
					 FFTFmplitude[29] * FFTFmplitude[29] / 29 +
					 FFTFmplitude[30] * FFTFmplitude[30] / 30 +
					 FFTFmplitude[31] * FFTFmplitude[31] / 31 +
					 FFTFmplitude[32] * FFTFmplitude[32] / 32 +
					 FFTFmplitude[33] * FFTFmplitude[33] / 33 +
					 FFTFmplitude[34] * FFTFmplitude[34] / 34 +
					 FFTFmplitude[35] * FFTFmplitude[35] / 35 +
					 FFTFmplitude[36] * FFTFmplitude[36] / 36 +
					 FFTFmplitude[37] * FFTFmplitude[37] / 37 +
					 FFTFmplitude[38] * FFTFmplitude[38] / 38 +
					 FFTFmplitude[39] * FFTFmplitude[39] / 39;

	// 14-30
	wave->Beta =  wave->lowBeta + wave->highBeta;

	// 40-80
	wave->lowGama = FFTFmplitude[40] * FFTFmplitude[40] / 40 +
					FFTFmplitude[41] * FFTFmplitude[41] / 41 +
					FFTFmplitude[42] * FFTFmplitude[42] / 42 +
					FFTFmplitude[43] * FFTFmplitude[43] / 43 +
					FFTFmplitude[44] * FFTFmplitude[44] / 44 +
					FFTFmplitude[45] * FFTFmplitude[45] / 45 +
					FFTFmplitude[46] * FFTFmplitude[46] / 46 +
					FFTFmplitude[47] * FFTFmplitude[47] / 47 +
					FFTFmplitude[48] * FFTFmplitude[48] / 48 +
					FFTFmplitude[49] * FFTFmplitude[49] / 49 +
					FFTFmplitude[50] * FFTFmplitude[50] / 50 +
					FFTFmplitude[51] * FFTFmplitude[51] / 51 +
					FFTFmplitude[52] * FFTFmplitude[52] / 52 +
					FFTFmplitude[53] * FFTFmplitude[53] / 53 +
					FFTFmplitude[54] * FFTFmplitude[54] / 54 +
					FFTFmplitude[55] * FFTFmplitude[55] / 55 +
					FFTFmplitude[56] * FFTFmplitude[56] / 56 +
					FFTFmplitude[57] * FFTFmplitude[57] / 57 +
					FFTFmplitude[58] * FFTFmplitude[58] / 58 +
					FFTFmplitude[59] * FFTFmplitude[59] / 59;

	wave->middleGamma = FFTFmplitude[60] * FFTFmplitude[60] / 60 +
					 FFTFmplitude[61] * FFTFmplitude[61] / 61 +
					 FFTFmplitude[62] * FFTFmplitude[62] / 62 +
					 FFTFmplitude[63] * FFTFmplitude[63] / 63 +
					 FFTFmplitude[64] * FFTFmplitude[64] / 64 +
					 FFTFmplitude[65] * FFTFmplitude[65] / 65 +
					 FFTFmplitude[66] * FFTFmplitude[66] / 66 +
					 FFTFmplitude[67] * FFTFmplitude[67] / 67 +
					 FFTFmplitude[68] * FFTFmplitude[68] / 68 +
					 FFTFmplitude[69] * FFTFmplitude[69] / 69 +
					 FFTFmplitude[70] * FFTFmplitude[70] / 70 +
					 FFTFmplitude[71] * FFTFmplitude[71] / 71 +
					 FFTFmplitude[72] * FFTFmplitude[72] / 72 +
					 FFTFmplitude[73] * FFTFmplitude[73] / 73 +
					 FFTFmplitude[74] * FFTFmplitude[74] / 74 +
					 FFTFmplitude[75] * FFTFmplitude[75] / 75 +
					 FFTFmplitude[76] * FFTFmplitude[76] / 76 +
					 FFTFmplitude[77] * FFTFmplitude[77] / 77 +
					 FFTFmplitude[78] * FFTFmplitude[78] / 78 +
					 FFTFmplitude[79] * FFTFmplitude[79] / 79 ;

	// 31-50
	wave->Gama = wave->lowGama + wave->middleGamma;
}
// ����Ŀ��Ƶ�ʴ����������������Ƶ�ʵ�ƽ������
// ���ݲ���������ֵ������Ƶ�ʵ㡢����Ƶ�ʵ㡢������ƽ�����ʡ�������ƽ������
// void windowMean(long* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean)
//{
//	int i;
//	*windowMean = 0;
//	*otherMean = 0;
//	//ע���һ��ֵ��Ҫ��������Ϊ����ʾ�źŵ�ƽ�����ʣ�ֱ��������
//	for (i = 1; i < NPT/2; ++i)
//	{
//		if (i >= lowBin && i <= highBin)//�ҵ�Ŀ��Ƶ�ʵĴ���
//		{
//			*windowMean += magnitudes[i];//�����ڷ�ֵ���
//		}
//		else
//		{
//			*otherMean += magnitudes[i];//�������ֵ���
//		}
//	}
//	*windowMean /= (highBin - lowBin) + 1;//��ô�����ƽ����ֵ
//	*otherMean /= (NPT / 2 - (highBin - lowBin)-1);//��ô�����ƽ����ֵ
// }

// ��ȡĳ��Ƶ����Ƶ��ͼ�е�����λ��
// int frequencyToBin(float frequency)
//{
//   float binFrequency = (float)(Fs / NPT);//10000/256 = 39.0625HZ
//   return (int)(frequency / binFrequency);//�ҵ��ض�Ƶ�����ڵ��Ǹ�����λ�ã������ڵ�93�к͵�94��
// }

/*
�ض���������㷨
*/
// void Sound_Detected(void)
//{
// }

// void Key_Init(void)
//{
//	 GPIO_InitTypeDef  GPIO_InitStructure;
//
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
//
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //LED0-->PA.8 �˿�����
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //�������
//  GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.8
// }
