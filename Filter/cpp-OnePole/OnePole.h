//
//  OnePole.h
//

#ifndef OnePole_h
#define OnePole_h

#include <math.h>

class OnePole
{
public:
    OnePole();
    OnePole(double Fc);
    ~OnePole();
    
    void setFc(double Fc);
    float process(float in);

protected:
    double a0, b1, z1;
};

#endif