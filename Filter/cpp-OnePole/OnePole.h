//
//  OnePole.h
//

#ifndef OnePole_h
#define OnePole_h

#include <math.h>

class OnePole {
public:
    OnePole() {a0 = 1.0; b1 = 0.0; z1 = 0.0;};
    OnePole(double Fc) {z1 = 0.0; setFc(Fc);};
    ~OnePole();
    void setFc(double Fc);
    float process(float in);
    
protected:    
    double a0, b1, z1;
};

#endif