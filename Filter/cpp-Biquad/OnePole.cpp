//
//  Biquad.cpp
//
//  Created by Nigel Redmon on 11/24/12
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the Biquad code:
//  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code
//  for your own purposes, free or commercial.
//

#include <math.h>
#include "OnePole.h"

inline void OnePole::setFc(double Fc)
{
    b1 = exp(-2.0 * M_PI * Fc);
    a0 = 1.0 - b1;
}

inline float OnePole::process(float in)
{
    return z1 = in * a0 + z1 * b1;
}