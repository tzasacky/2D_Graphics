#include "GPath.h"
#include "GPoint.h"
#include "GMatrix.h"
#include <vector>
#include "math.h"

/*
 *  Adds 4 points to the path, traversing the rect in the specified direction, beginning
 *  with the top-left corner.
 */
GPath& GPath::addRect(const GRect& rect, GPath::Direction dir) {
    GPoint lt = GPoint::Make(rect.left(), rect.top());
    GPoint rt = GPoint::Make(rect.right(), rect.top());
    GPoint rb = GPoint::Make(rect.right(), rect.bottom());
    GPoint lb = GPoint::Make(rect.left(), rect.bottom());

    this->moveTo(lt);
    if(dir == GPath::Direction::kCW_Direction){
      this->lineTo(rt);
      this->lineTo(rb);
      this->lineTo(lb);
    }else{
      this->lineTo(lb);
      this->lineTo(rb);
      this->lineTo(rt);
    }
    return *this;
}

// moveTo(pts[0]), lineTo(pts[1..count-1])
GPath& GPath::addPolygon(const GPoint pts[], int count) {
  if(count < 2){return *this;}

  GPath::moveTo(pts[0]);
  for(int i = 1; i < count; ++i){
      GPath::lineTo(pts[i]);
  }
  return *this;
}

GPath& GPath::addCircle(GPoint center, float radius, Direction dir){
  this->moveTo(GPoint::Make(radius + center.x(), center.y()));
  GPoint quads[2];
  if(dir == kCW_Direction){
      quads[0] = GPoint::Make(1.0f, -tan(M_PI/8));
      quads[1] = GPoint::Make(sqrt(2)/2, -sqrt(2)/2);
  }else{
      quads[0] = GPoint::Make(1.0f, tan(M_PI/8));
      quads[1] = GPoint::Make(sqrt(2)/2, sqrt(2)/2);
  }

  GMatrix rotMatrix;
  GMatrix matrix;
  matrix.set6(radius, 0, center.x(),
              0, radius, center.y());
  matrix.mapPoints(quads, quads, 2);
  GPoint transQuads[2];
  for (float angle = 0; angle < 2*M_PI - 0.001f; angle += 2*M_PI/8) {
      if(dir == kCW_Direction){
            rotMatrix.setRotate(-angle);
      }else{
            rotMatrix.setRotate(angle);
      }
      rotMatrix.mapPoints(transQuads, quads, 2);
      this->quadTo(transQuads[0], transQuads[1]);
  }
  return *this;
}

/**
 *  Return the bounds of all of the control-points in the path.
 *
 *  If there are no points, return {0, 0, 0, 0}
 */
GRect GPath::bounds() const {
    int count = this->fPts.size();
    if(count == 0){
        return GRect::MakeLTRB(0, 0, 0, 0);
    }
    float left  = fPts[0].fX;
    float right = fPts[0].fX;
    float top   = fPts[0].fX;
    float bot   = fPts[0].fX;
    for(int i = 0; i < count; ++i){
        if(fPts[i].fX < left){
          left = fPts[i].fX;
        }
        if(fPts[i].fX > right){
          right = fPts[i].fX;
        }
        if(fPts[i].fY < top){
          top = fPts[i].fY;
        }
        if(fPts[i].fY > bot){
          bot = fPts[i].fY;
        }
    }
    return GRect::MakeLTRB(left, top, right, bot);
}

/**
 *  Given 0 < t < 1, subdivide the src[] quadratic bezier at t into two new quadratics in dst[]
 *  such that
 *  0...t is stored in dst[0..2]
 *  t...1 is stored in dst[2..4]
 */
void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
  GPoint a = src[0];
  GPoint b = src[1];
  GPoint c = src[2];

  dst[0] = a;
  dst[4] = c;

  dst[1] = GPoint::Make((1-t)*a.x()+t*b.x(), (1-t)*a.y()+t*b.y());
  dst[3] = GPoint::Make((1-t)*b.x()+ t*c.x(), (1-t)*b.y()+t*c.y());

  dst[2] = GPoint::Make(pow(1-t, 2)*a.x() + 2*t*b.x()*(1-t) + c.x()*pow(t,2), pow(1-t, 2)*a.y() + 2*t*b.y()*(1-t) + c.y()*pow(t,2));
  return;
}

/**
 *  Given 0 < t < 1, subdivide the src[] cubic bezier at t into two new cubics in dst[]
 *  such that
 *  0...t is stored in dst[0..3]
 *  t...1 is stored in dst[3..6]
 */
void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t){
  //cubic(t) = at^3 + 3bt(1 - t)^2 + 3c(1 - t)t^2 + dt^3
  // number of edges = sqrt (D / error)
  // D = 3/4 * max(length(a - 2b + c), length(b - 2c + d))


  GPoint a = src[0];
  GPoint b = src[1];
  GPoint c = src[2];
  GPoint d = src[3];

  dst[0] = a;
  dst[6] = d;

  dst[1] = GPoint::Make((1-t)*a.x() + t*b.x(), (1-t)*a.y() +t*b.y());
  dst[5] = GPoint::Make((1-t)*c.x() + t*d.x(), (1-t)*c.y()+t*d.y());

  dst[2] = GPoint::Make(pow(1-t, 2)*a.x() + 2*t*b.x()*(1-t) + c.x()*pow(t,2), pow(1-t, 2)*a.y() + 2*t*b.y()*(1-t) + c.y()*pow(t,2));
  dst[4] = GPoint::Make(pow(1-t, 2)*b.x() + 2*t*c.x()*(1-t) + d.x()*pow(t,2), pow(1-t, 2)*b.y() + 2*t*c.y()*(1-t) + d.y()*pow(t,2));

  dst[3] = GPoint::Make((pow(1-t, 3)*a.x() + 3*t*b.x()*pow(1-t, 2) + 3*(1-t)*c.x()*pow(t, 2) + d.x()*pow(t,3)),
                        (pow(1-t, 3)*a.y() + 3*t*b.y()*pow(1-t, 2) + 3*(1-t)*c.y()*pow(t, 2) + d.y()*pow(t,3)));
}

/**
 *  Transform the path in-place by the specified matrix.
 */
void GPath::transform(const GMatrix& matrix){
    matrix.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size());
}
