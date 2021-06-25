#include <memory>
#include "GPixel.h"
#include "GFilter.h"
#include "blend.h"
#include "Utils.h"

class GColor;

class BlendFilter: public GFilter {
public:
    BlendFilter (GBlendMode mode, const GColor& src) : fBlendMode(mode), fSrc(colortoPixel(src)) {}

    virtual bool preservesAlpha(){
      return false;
    }

    virtual void filter(GPixel output[], const GPixel input[], int count) {
      Blend blend = getBlend(fBlendMode);
      for (int i = 0; i < count; ++i) {
         output[i] = blend(fSrc, input[i]);
      }
   }

 private:
     GBlendMode fBlendMode;
     GPixel fSrc;
};
std::unique_ptr<GFilter> GCreateBlendFilter(GBlendMode mode, const GColor& src){
    return std::unique_ptr<GFilter>(new BlendFilter(mode, src));
}
