#include "Utils.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"
#include "math.h"
#include <algorithm>

class RadialGradientShader: public GShader {
public:
    RadialGradientShader(GPoint center, float radius, const GColor colors[], int count){
        fColors = new GColor[count];
        memcpy(fColors, colors, count * sizeof(GColor));
        fCount = count;
        fCenter = center;
        fRadius = radius;

        //Scale
        fLocalMatrix.setTranslate(fCenter.x(), fCenter.y());
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
           float pointRadius = sqrt(pow(point.x(), 2) + pow(point.y(), 2));
           float t = std::min(std::max(pointRadius, 0.0f), fRadius);
           if(t == fRadius){
              row[i] = colortoPixel(fColors[fCount -1]);
              continue;
           }
           float interval = fRadius / (float) (fCount - 1);
           float trueIndex = t / interval;
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
  GPoint fCenter;
  float fRadius;
};

/*
std::unique_ptr<GShader> GCanvas::final_createRadialGradient(
      GPoint center, float radius, const GColor colors[], int count) override{
      if(count < 2){return nullptr;}
      return std::unique_ptr<GShader>(new RadialGradientShader(center, radius, colors, count));
}
*/
