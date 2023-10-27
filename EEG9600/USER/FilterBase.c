#include "FilterBase.h"


// 计算双二阶
void calc_biquad(bq_coeff_p coeff_p);

// 设置双二阶类型
void biquad_set_type(bq_coeff_p coeff_p, uint8_t filter_type)
{
    coeff_p->type = (filter_type_e)filter_type;
    calc_biquad(coeff_p);
}

// 设置Q
void biquad_set_Q(bq_coeff_p coeff_p, double Q)
{
    coeff_p->Q = Q;
    calc_biquad(coeff_p);
}

// 设置频率
void biquad_set_fc(bq_coeff_p coeff_p, double fc)
{
    coeff_p->fc = fc / coeff_p->sample_rate;
    calc_biquad(coeff_p);
}

// 设置顶峰增益
void biquad_set_peak_gain(bq_coeff_p coeff_p, double peak_gainDB)
{
    coeff_p->peak_gain = peak_gainDB;
    calc_biquad(coeff_p);
}

// 双二阶配置
void biquad_config(bq_coeff_p coeff_p, uint8_t filter_type, double fc, uint32_t sample_rate, double Q, double peak_gainDB)
{
    coeff_p->type = (filter_type_e)filter_type;
    coeff_p->fc = fc / (double)sample_rate;
    coeff_p->sample_rate = sample_rate;
    coeff_p->Q = Q;
    coeff_p->peak_gain = peak_gainDB;
    calc_biquad(coeff_p);
}

// 计算双二阶
void calc_biquad(bq_coeff_p coeff_p)
{
    double norm;
    double a0 = 0, a1 = 0, a2 = 0,  b1 = 0, b2 = 0;

    double Q = coeff_p->Q;
    double V = pow(10, fabs(coeff_p->peak_gain) / 20.0);
    double K = tan(M_PI * coeff_p->fc);

    switch (coeff_p->type)
    {
    // 低通
    case bq_type_lowpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    // 高通
    case bq_type_highpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = 1 * norm;
        a1 = -2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    // 带通
    case bq_type_bandpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K / Q * norm;
        a1 = 0;
        a2 = -a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    // 带阻
    case bq_type_notch:
        norm = 1 / (1 + K / Q + K * K);
        a0 = (1 + K * K) * norm;
        a1 = 2 * (K * K - 1) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1 - K / Q + K * K) * norm;
        break;
 
    case bq_type_peak:
        if (coeff_p->peak_gain >= 0) // boost
        {
            norm = 1 / (1 + 1 / Q * K + K * K);
            a0 = (1 + V / Q * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - V / Q * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - 1 / Q * K + K * K) * norm;
        }
        else // cut
        {
            norm = 1 / (1 + V / Q * K + K * K);
            a0 = (1 + 1 / Q * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - 1 / Q * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - V / Q * K + K * K) * norm;
        }

        break;

    case bq_type_lowshelf:
        if (coeff_p->peak_gain >= 0) // boost
        {
            norm = 1 / (1 + sqrt(2) * K + K * K);
            a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
            a1 = 2 * (V * K * K - 1) * norm;
            a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrt(2) * K + K * K) * norm;
        }
        else // cut
        {
            norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
            a0 = (1 + sqrt(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrt(2) * K + K * K) * norm;
            b1 = 2 * (V * K * K - 1) * norm;
            b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
        }

        break;

    case bq_type_highshelf:
        if (coeff_p->peak_gain >= 0) // boost
        {
            norm = 1 / (1 + sqrt(2) * K + K * K);
            a0 = (V + sqrt(2 * V) * K + K * K) * norm;
            a1 = 2 * (K * K - V) * norm;
            a2 = (V - sqrt(2 * V) * K + K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - sqrt(2) * K + K * K) * norm;
        }
        else // cut
        {
            norm = 1 / (V + sqrt(2 * V) * K + K * K);
            a0 = (1 + sqrt(2) * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - sqrt(2) * K + K * K) * norm;
            b1 = 2 * (K * K - V) * norm;
            b2 = (V - sqrt(2 * V) * K + K * K) * norm;
        }

        break;
    }

    coeff_p->a0 = a0;
    coeff_p->a1 = a1;
    coeff_p->a2 = a2;
    coeff_p->b1 = b1;
    coeff_p->b2 = b2;

    return;
}

// 双二阶计算
float biquad_process(bq_coeff_p coeff_p, float in)
{
    double out = in * coeff_p->a0 + coeff_p->z1;
    coeff_p->z1 = in * coeff_p->a1 + coeff_p->z2 - coeff_p->b1 * out;
    coeff_p->z2 = in * coeff_p->a2 - coeff_p->b2 * out;
    return out;
}
