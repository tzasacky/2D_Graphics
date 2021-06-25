#include "GPoint.h"
#include "GRect.h"
#include "GPaint.h"
#include "GBitmap.h"
#include "blend.h"

/*
 * Layer for layer stack
 */
struct Layer {
    GBitmap bitmap;
    GPoint  translation;
    GPaint  paint;

    Layer(GBitmap bitmap, GPoint translation, GPaint paint);
    void drawLayer(Layer topLayer);
};

Layer::Layer(GBitmap bitmap, GPoint translation, GPaint paint) {
  this->bitmap = bitmap;
  this->translation = translation;
  this->paint = paint;
}

void Layer::drawLayer(Layer topLayer) {
    GBitmap dstLayer    = topLayer.bitmap;
    GBitmap bmap        = this->bitmap;
    GPoint  translation = this->translation;
    GPaint  paint       = this->paint;

    //Get paint info
    Blend blend = getBlend(paint.getBlendMode());
    GFilter* filter = paint.getFilter();
    int count = bmap.width();

    for(int y = 0; y < bmap.height(); ++y){
          //Blend and fill row memory using filter and blendmode
          for (int x = 0; x < count; ++x) {
                GPixel* addr = bmap.getAddr(x, y);
                int r = GPixel_GetR(*addr);
                int b = GPixel_GetB(*addr);
                if(filter){
                    filter->filter(addr, addr, 1);
                }
                GPixel* otheraddr = dstLayer.getAddr(x + translation.x(), y + translation.y());
                *otheraddr = blend(*addr, *otheraddr);
          }
    }
}
