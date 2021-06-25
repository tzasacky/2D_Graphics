#include "Utils.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"
#include "math.h"
#include <algorithm>

class TriangleGradientShader: public GShader {
public:
    TriangleGradientShader(const GPoint pts[], const GColor colors[]){
        fColors = new GColor[3];
        memcpy(fColors, colors, 3 * sizeof(GColor));
        fPts = new GPoint[3];
        memcpy(fPts, pts, 3 * sizeof(GPoint));
        //Scale
        fLocalMatrix.setIdentity();
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
           GPoint point = fInverse.mapXY(x + i, y);

           //find midpoints of opposing side
           //project inner point onto line and normalize
           GPoint midpoints[3];
           for(int p = 0; p < 3; ++p){
              GPoint p0 = fPts[p];
              GPoint p1;
              if(p == 3){
                p1 = fPts[0];
              }else{
                p1 = fPts[p + 1];
              }
              midpoints[p] = GPoint::Make((p0.x() + p1.x())/2 , (p0.x() + p1.x())/2);
           }

          //project inner point onto midpoint lines and normalize
          GPoint ptVec = GPoint::Make(point.x() - midpoints[0].x(), point.y() - midpoints[0].y());
          GPoint lineVec =  GPoint::Make(fPts[2].x() - midpoints[0].x(), fPts[2].y() -  midpoints[0].y());
          //do dot product and normalize

          float c3Ratio = abs(ptVec.x() * lineVec.x() + ptVec.y() * lineVec.y()) / (pow(lineVec.x(), 2) + pow(lineVec.y(), 2));
          c3Ratio =  std::min(std::max(c3Ratio, 0.0f), 1.0f);

          //fuck it copy paste for time
          //project inner point onto midpoint lines and normalize
          ptVec = GPoint::Make(point.x() - midpoints[2].x(), point.y() - midpoints[2].y());
          lineVec =  GPoint::Make(fPts[1].x() - midpoints[2].x(), fPts[1].y() -  midpoints[2].y());
          //do dot product and normalize
          float c2Ratio = abs(ptVec.x() * lineVec.x() + ptVec.y() * lineVec.y()) / (pow(lineVec.x(), 2) + pow(lineVec.y(), 2));
          c2Ratio =  std::min(std::max(c2Ratio, 0.0f), 1.0f);

          //project inner point onto midpoint lines and normalize
          ptVec = GPoint::Make(point.x() - midpoints[1].x(), point.y() - midpoints[1].y());
          lineVec =  GPoint::Make(fPts[0].x() - midpoints[1].x(), fPts[0].y() -  midpoints[1].y());
          //do dot product and normalize
          float c1Ratio = abs(ptVec.x() * lineVec.x() + ptVec.y() * lineVec.y()) / (pow(lineVec.x(), 2) + pow(lineVec.y(), 2));
          c1Ratio =  std::min(std::max(c1Ratio, 0.0f), 1.0f);

           GColor c1 = fColors[0];
           GColor c2 = fColors[1];
           GColor c3 = fColors[2];
           float a = std::min(c1.fA * c1Ratio + c2.fA * c2Ratio + c3.fA * c3Ratio, 1.0f);
           GColor interpolatedColor = GColor::MakeARGB(
               a,
               a*(std::min(c1.fR * c1Ratio + c2.fR * c2Ratio + c3.fR * c3Ratio, 1.0f)),
               a*(std::min(c1.fG * c1Ratio + c2.fG * c2Ratio + c3.fG * c3Ratio, 1.0f)),
               a*(std::min(c1.fB * c1Ratio + c2.fB * c2Ratio + c3.fB * c3Ratio, 1.0f)));
           row[i] = colortoPixel(interpolatedColor);
     }
     return;
   }

private:
  GColor* fColors;
  GMatrix fLocalMatrix;
  GMatrix fInverse;
  GPoint* fPts;
};
