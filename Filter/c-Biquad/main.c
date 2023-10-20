// 双二阶滤波
#include "Biquad.h"

#define LEN 250

bq_coeff_t bsf;
bq_coeff_t hpf;
bq_coeff_t lpf;

float raw_in[LEN];
float bsf_out[LEN];
float hpf_out[LEN];
float lpf_out[LEN];

void main()
{
    /****************************** 随机生成 ********************************/ 
    // RAW_IN 生成 𝒔𝒊𝒏( 2 × 3.1415926 × 50 × 𝑖 / 250 )
    for (int i = 0; i < LEN; i++)
    {
        raw_in[i] = sin(2 * 3.1415926 * 50 * i / 250);
    }
    /***********************************************************************/
    biquad_config(&bsf, bq_type_notch, 50, 250, 0.7071, 6.0);     // 配置一个陷波器，FC 50Hz,采样频率250Hz，Q=0.7071，Gain=6
    biquad_config(&hpf, bq_type_highpass, 0.6, 250, 0.7071, 6.0); // 配置一个高通滤波器，FC 0.6Hz,采样频率250Hz，Q=0.7071，Gain=6
    biquad_config(&lpf, bq_type_lowpass, 35, /* FC/Samples */ 250, 0.7071, 6.0);   // 配置一个低通滤波器，FC35Hz,采样频率250Hz，Q=0.7071，Gain=6
    calc_biquad(&bsf);
    calc_biquad(&hpf);
    calc_biquad(&lpf);

    for (int idx = 0; idx < LEN; idx++)
    {
        bsf_out[idx] = biquad_process(&bsf, raw_in[idx]);
        hpf_out[idx] = biquad_process(&hpf, bsf_out[idx]);
        lpf_out[idx] = biquad_process(&lpf, hpf_out[idx]);
    }
    for (int idx = 0; idx < LEN; idx++)
    {
        printf("raw=%6d,bsf=%6d,hpf=%6d,lpf=%6d\r\n", raw_in[idx], (int)bsf_out[idx], (int)hpf_out[idx], (int)lpf_out[idx]);
    }
}
