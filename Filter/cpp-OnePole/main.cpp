#include "Biquad.h"

#define Fc 0.6
#define sampleRate 44100
#define bufSize 256

float in[bufSize];

void main()
{
    Biquad *lpFilter = new Biquad(); // create a Biquad, lpFilter;

    lpFilter->setBiquad(bq_type_lowpass, Fc / sampleRate, 0.707, 0);

    // 过滤输入样本的缓冲区，原地操作。
    for (int idx = 0; idx < bufSize; idx++)
    {
        in[idx] = lpFilter->process(in[idx]);
    }
}
