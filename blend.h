/*
 * Blend colors of canvas (src) and the new pixel (dest)
 * All PorterDuff types implemented
 */

#ifndef blend_H
#define blend_H
#include <stdio.h>

/*
 *  printf("SRC: a %d, r %d, g %d, b %d \n", sA, sR, sG, sB);
 *  printf("DST: a %d, r %d, g %d, b %d \n", dA, dR, dG, dB);
 *  printf("BLENT: a %d, r %d, g %d, b %d \n", a, r, g, b);
 */

 static inline GPixel clear(const GPixel source, const GPixel dest) {
     //!< [0, 0]
     return GPixel_PackARGB(0, 0, 0, 0);
 }


 static inline GPixel src(const GPixel source, const GPixel dest) {
     //!< [Sa, Sc]
     return source;
 }


 static inline GPixel dest(const GPixel source, const GPixel dest) {
     //!< [Da, Dc]
     return dest;
 }


static inline GPixel srcOver(GPixel src, GPixel dest){
    //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = sA + ((255 - sA) * dA) / 255;
    int r = sR + ((255 - sA) * dR) / 255;
    int g = sG + ((255 - sA) * dG) / 255;
    int b = sB + ((255 - sA) * dB) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel destOver(GPixel src, GPixel dest){
    //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = dA + ((255 - dA) * sA) / 255;
    int r = dR + ((255 - dA) * sR) / 255;
    int g = dG + ((255 - dA) * sG) / 255;
    int b = dB + ((255 - dA) * sB) / 255;

    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel srcIn(GPixel src, GPixel dest){
    //!< [Sa * Da, Sc * Da]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = (sA * dA) / 255;
    int r = (sR * dA) / 255;
    int g = (sG * dA) / 255;
    int b = (sB * dA) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel destIn(GPixel src, GPixel dest){
    //!< [Da * Sa, Dc * Sa]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = (dA * sA) / 255;
    int r = (dR * sA) / 255;
    int g = (dG * sA) / 255;
    int b = (dB * sA) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel srcOut(GPixel src, GPixel dest){
    //!< [Sa * (1 - Da), Sc * (1 - Da)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = sA * (255 - dA) / 255;
    int r = sR * (255 - dA) / 255;
    int g = sG * (255 - dA) / 255;
    int b = sB * (255 - dA) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel destOut(GPixel src, GPixel dest){
    //!< [Da * (1 - Sa), Dc * (1 - Sa)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = dA * (255 - sA) / 255;
    int r = dR * (255 - sA) / 255;
    int g = dG * (255 - sA) / 255;
    int b = dB * (255 - sA) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel srcATop(GPixel src, GPixel dest){
    //!< [Da, Sc * Da + Dc * (1 - Sa)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = dA;
    int r = (sR * dA + ((255 - sA) * dR)) / 255;
    int g = (sG * dA + ((255 - sA) * dG)) / 255;
    int b = (sB * dA + ((255 - sA) * dB)) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel dstATop(GPixel src, GPixel dest){
    //!< [Sa, Dc * Sa + Sc * (1 - Da)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = sA;
    int r = (dR * sA + ((255 - dA) * sR)) / 255;
    int g = (dG * sA + ((255 - dA) * sG)) / 255;
    int b = (dB * sA + ((255 - dA) * sB)) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel kxor(GPixel src, GPixel dest){
     //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
    int sA = GPixel_GetA(src);
    int sR = GPixel_GetR(src);
    int sG = GPixel_GetG(src);
    int sB = GPixel_GetB(src);

    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = (((255 - dA) * sA) + ((255 - sA) * dA)) / 255;
    int r = (((255 - dA) * sR) + ((255 - sA) * dR)) / 255;
    int g = (((255 - dA) * sG) + ((255 - sA) * dG)) / 255;
    int b = (((255 - dA) * sB) + ((255 - sA) * dB)) / 255;
    return GPixel_PackARGB(a, r, g, b);
}

typedef GPixel (*Blend)(GPixel, GPixel);

static Blend BLEND[] = {
    clear,
    src,
    dest,
    srcOver,
    destOver,
    srcIn,
    destIn,
    srcOut,
    destOut,
    srcATop,
    dstATop,
    kxor,
};


static inline Blend getBlend(const GBlendMode mode) {
    return BLEND[static_cast<int>(mode)];
}

#endif
