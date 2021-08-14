#include "wrapper.hpp"

float module(float s){
    if(s >= 0)
        return s;
    else
        return -s;
    return 0;
}


float interpolation_kernel(float s, float a) {
   float mod = module(s);

   if ((mod>= 0) && (mod<= 1))
    {
        return (a + 2)*mod*mod*mod - (a + 3)*mod*mod + 1;
    }
    else
      if ((mod > 1) && (mod <= 2))
        {
           return a*mod*mod*mod - (5*a)*mod*mod + (8*a)*mod - 4*a;
        }

    return 0;
}

