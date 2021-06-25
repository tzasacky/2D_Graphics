#include "GMatrix.h"
#include <math.h>
#include "GPoint.h"
/**
 *  Set this matrix to identity.
 */
void GMatrix::setIdentity(){
  this->set6(1, 0, 0,
             0, 1, 0);
}

/**
 *  Set this matrix to translate by the specified amounts.
 */
void GMatrix::setTranslate(float tx, float ty){
  this->set6(1, 0, tx,
             0, 1, ty);
}


/**
 *  Set this matrix to scale by the specified amounts.
 */
void GMatrix::setScale(float sx, float sy){
  this->set6(sx, 0, 0,
             0, sy, 0);
}


/**
 *  Set this matrix to rotate by the specified radians.
 *
 *  Note: since positive-Y goes down, a small angle of rotation will increase Y.
 */
void GMatrix::setRotate(float radians){
  this->set6(cos(radians), -sin(radians), 0, sin(radians), cos(radians), 0);
}


/**
 *  Set this matrix to the concatenation of the two specified matrices, such that the resulting
 *  matrix, when applied to points will have the same effect as first applying the primo matrix
 *  to the points, and then applying the secundo matrix to the resulting points.
 *
 *  Pts' = Secundo * Primo * Pts
 */
void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo){
  /*
  *[ A B C ] [ a b c ]   [ Aa+Bd  Ab+Be  Ac+Bf+C ]
  *[ D E F ]*[ d e f ] = [ Da+Ed  Db+Ee  Dc+Ef+F ]
  *[ 0 0 1 ] [ 0 0 1 ]   [   0      0       1    ]
  * a = 0 , b = 1, c = 2, d= 3, e = 4, f = 5
  */
  this->set6(secundo[0] * primo[0] + secundo[1] * primo[3],              //Aa+Bd
             secundo[0] * primo[1] + secundo[1] * primo[4],              //Ab+Be
             secundo[0] * primo[2] + secundo[1] * primo[5] + secundo[2], //Ac+Bf+C
             secundo[3] * primo[0] + secundo[4] * primo[3],              //Da+Ed
             secundo[3] * primo[1] + secundo[4] * primo[4],              //Db+Ee
             secundo[3] * primo[2] + secundo[4] * primo[5] + secundo[5]);//Dc+Ef+F
}


/*
 *  If this matrix is invertible, return true and (if not null) set the inverse parameter.
 *  If this matrix is not invertible, return false and ignore the inverse parameter.
 */
bool GMatrix::invert(GMatrix* inverse) const{
  float a = this->fMat[0];
  float b = this->fMat[1];
  float c = this->fMat[2];
  float d = this->fMat[3];
  float e = this->fMat[4];
  float f = this->fMat[5];

  /*
   *      [  E -B  BF-EC ]    1
   * M^ = [ -D  A  DC-AF ] * ---
   *      [  0  0    1   ]   det
   */

  float det =  a * e - b * d;
  if (det == 0) {
    return false;
  }else{
    //Take reciprocal of det to minimize divide ops
    float rDet = 1 / det;
    inverse->set6(e * rDet, -b * rDet, (b * f - c * e) * rDet,
                 -d * rDet, a * rDet, (c * d - a * f) * rDet);
    return true;
  }
}

/**
 *  Transform the set of points in src, storing the resulting points in dst, by applying this
 *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
 *
 *  Note: It is legal for src and dst to point to the same memory (however, they may not
 *  partially overlap). Thus the following is supported.
 *
 *  GPoint pts[] = { ... };
 *  matrix.mapPoints(pts, pts, count);
 */
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const{
  /*
  * x = Sx * x + Tx + (y * Ky)
  * y = Sy * y + Ty + (x * Ky)
  */
  for (int i = 0; i < count; ++i) {
      GPoint point = src[i];
      float x = this->fMat[GMatrix::SX] * point.fX + this->fMat[GMatrix::TX] + this->fMat[GMatrix::KX] * point.fY;
      float y = this->fMat[GMatrix::SY] * point.fY + this->fMat[GMatrix::TY] + this->fMat[GMatrix::KY] * point.fX;
      dst[i] = GPoint::Make(x, y);
  }
}
