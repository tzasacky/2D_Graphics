#ifndef Utils_H
#define Utils_H

#include "GColor.h"
#include "GMath.h"
#include "GPixel.h"
#include "GPoint.h"


/*
 * Convert GColor to GPixel
 */
inline GPixel colortoPixel(const GColor& color){
    //Pin to legal range, scale up, pre-multiply, and round
    GColor colorPin = color.pinToUnit();
    int a = GRoundToInt(colorPin.fA * 255);
    int r = GRoundToInt(colorPin.fA * colorPin.fR * 255);
    int g = GRoundToInt(colorPin.fA * colorPin.fG * 255);
    int b = GRoundToInt(colorPin.fA * colorPin.fB * 255);
    //Pack 8-bit values into GPixel
    return GPixel_PackARGB(a, r, g, b);
}

#endif
