#ifndef __FFT_H
#define __FFT_H

#include "sys.h"
#include "key.h"

typedef struct
{
    uint16_t Delta;
    uint16_t Theta;
    uint16_t Alpha;
    uint16_t Beta;
    uint16_t Gama;
		
	uint16_t lowAlpha;
	uint16_t highAlpha;
	
	uint16_t lowBeta;
	uint16_t highBeta;
	
	uint16_t lowGama;
	uint16_t middleGamma;
}  sWavePwr_t,*pWavePwr_t; 

#define NPT 256 // FFT��������
#define Fs 256  // ����Ƶ��Ϊ10Khz

extern long White_Noise;
extern long FFT_IN[NPT];

void FFT_Start_Initialization(void);
void GetPowerMag(void);
void GetPwrMag(long FFTOut[], long FFTFmplitude[]);
void FFT_Running(void);
void Sound_Detected(void);
void Key_Init(void);
void wave2Pwr( long FFTFmplitude[],sWavePwr_t *wave);

#endif
