#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"

class BitmapShader: public GShader {
public:
  BitmapShader(const GBitmap& bitmap, const GMatrix& localInv, GShader::TileMode tile): fBitmap(bitmap) , fLocalMatrix(localInv), fTile(tile) {}

  // Return true iff all of the GPixels that may be returned by this shader will be opaque.
  bool isOpaque(){
    return false;
  }

  bool setContext(const GMatrix& ctm){
    //Place inverse in fInverse, return False if non-invertible
    if (!ctm.invert(&fInverse)) {
            return false;
        }
    //postconcat because fInverse is 1st matrix
    fInverse.postConcat(fLocalMatrix);
    return true;
  }

  /**
   *  Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
   *  corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
   *  can hold at least [count] entries.
   */
  virtual void shadeRow(int x, int y, int count, GPixel row[]){
    GPoint loc = fInverse.mapXY(x + 0.5f, y + 0.5f);
    for (int i = 0; i < count; ++i) {
        //Pin to legal range
        if(fTile == kClamp){
          x = std::max(0, std::min(fBitmap.width() - 1, GRoundToInt(loc.x())));
          y = std::max(0, std::min(fBitmap.height() - 1, GRoundToInt(loc.y())));
        }else if(fTile == kRepeat){
          float x1 = ((float)loc.x())/fBitmap.width();
          float y1 = ((float)loc.y())/fBitmap.height();
          x1 = x1 - floor(x1);
          y1 = y1 - floor(y1);
          x = x1*fBitmap.width();
          y = y1*fBitmap.height();
        }else{
          y = loc.y() * 0.5;
          y = abs(GRoundToInt(y % fBitmap.height()));
          if (y > 0.5 * fBitmap.height()) {
             y = fBitmap.height() - y;
          }
          y = y * 2;

          x = loc.x() * 0.5;
          x = abs(GRoundToInt(x % fBitmap.width()));
          if (x > 0.5 * fBitmap.width()) {
             x = fBitmap.width() - x;
          }
          x = x * 2;
        }

        row[i] = *fBitmap.getAddr(x , y);
        loc.fX += fInverse[GMatrix::SX];
        loc.fY += fInverse[GMatrix::KY];
    }
}

private:
    GBitmap fBitmap;
    GMatrix fInverse;
    GMatrix fLocalMatrix;
    GShader::TileMode fTile;
};


/**
*  Return a subclass of GShader that draws the specified bitmap and the inverse of a local matrix.
*  Returns null if the either parameter is invalid.
*/
std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localInv, GShader::TileMode tile) {
  //One of the parameters has incorrect data passed
  if (!bitmap.pixels()) {
      return nullptr;
  }
  //Valid, return shader
  return std::unique_ptr<GShader>(new BitmapShader(bitmap, localInv, tile));
}
