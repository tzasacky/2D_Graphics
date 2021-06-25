#include "Utils.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"
#include "math.h"
#include <algorithm>

class LinearGradientShader: public GShader {
public:
    LinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tile){
        fColors = new GColor[count];
        memcpy(fColors, colors, count * sizeof(GColor));
        fCount = count;
        fTile = tile;

        if (p0.x() > p1.x()) {
            std::swap(p0, p1);
        }
        //Scale
        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p0.fY;
        fLocalMatrix.set6(dx, -dy, p0.fX,
                          dy, dx, p0.fY);
    }

    virtual bool isOpaque(){
        return false;
    }

   virtual bool setContext(const GMatrix& ctm){
      fInverse.setConcat(ctm, fLocalMatrix);
      return fInverse.invert(&fInverse);
   }

   virtual void shadeRow(int x, int y, int count, GPixel row[]){
     for(int i = 0; i < count; ++i){
          if(fCount == 1){
              row[i] = colortoPixel(fColors[0]);
              continue;
           }

           GPoint point = fInverse.mapXY(x + i, y);
           float t = point.fX;
           if(fTile == kClamp){
                t = std::min(std::max(t, 0.0f), 1.0f);
           }else if(fTile == kRepeat){
                t = t - floor(t);
           }else{
              t = t * 0.5;
              t = t - floor(t);
              if (t > 0.5) {
                 t = 1 - t;
              }
              t = t * 2;
           }

           float trueIndex = t * (fCount - 1);
           int index = floor(trueIndex);
           float c2Ratio = trueIndex - index;

           GColor c1 = fColors[index];
           GColor c2 = fColors[index + 1];
           float a = c1.fA * (1 - c2Ratio) + c2.fA * c2Ratio;
           GColor interpolatedColor = GColor::MakeARGB(
               a,
               a*(c1.fR * (1 - c2Ratio) + c2.fR * c2Ratio),
               a*(c1.fG * (1 - c2Ratio) + c2.fG * c2Ratio),
               a*(c1.fB * (1 - c2Ratio) + c2.fB * c2Ratio));
           row[i] = colortoPixel(interpolatedColor);
     }
     return;
   }

private:
  GColor* fColors;
  int fCount;
  GMatrix fLocalMatrix;
  GMatrix fInverse;
  GShader::TileMode fTile;
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tile){
  if(count == 0){return nullptr;}
  return std::unique_ptr<GShader>(new LinearGradientShader(p0, p1, colors, count, tile));
}
