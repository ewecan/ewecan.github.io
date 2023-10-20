#include "OnePole.h"

#define Fc 0.6
#define sampleRate 500
#define F Fc / sampleRate

#define bufSize 256

float in;

OnePole dcBlockerLp(F);

void main()
{
    // 直流阻断器
    in -= dcBlockerLp.process(in);
}
