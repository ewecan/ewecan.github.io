
#ifndef Biquad_h
#define Biquad_h

#include "stdint.h"
#include "math.h"
#include "stdio.h"

typedef enum
{
    bq_type_lowpass = 0,
    bq_type_highpass,
    bq_type_bandpass,
    bq_type_notch,
    bq_type_peak,
    bq_type_lowshelf,
    bq_type_highshelf
} filter_type_e;

typedef struct
{
    filter_type_e type;
    double a0,a1,a2,b1,b2; 
    double z1,z2; 
    double fc;
    double Q;
    uint32_t sample_rate;
    double peak_gain;
    float out;

} bq_coeff_t, *bq_coeff_p;

#define M_PI 3.14159265358979323846

void biquad_config(bq_coeff_p coeff_p, uint8_t type, double fc, uint32_t sample_rate, double Q, double peak_gainDB);
void calc_biquad(bq_coeff_p coeff_p);
float biquad_process(bq_coeff_p coeff_p, float in);

#endif
