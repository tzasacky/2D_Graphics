//Bezier Functions
#include "GPath.h"
#include "math.h"
#include "GPoint.h"
#include "clip.h"

class GRect;

static float vectorLength(float xLen, float yLen){
    return sqrt(pow(xLen, 2) + pow(yLen, 2));
}

static float cubic(float a, float b, float c, float d, float t){
    return a*pow(1-t, 3) + 3*b*t*pow((1 - t), 2) + 3*c*(1 - t)*pow(t,2) + d*pow(t, 3);
}

static float quad(float a, float b, float c, float t){
    return a*pow(1-t, 2) + 2*b*t*(1 - t) + c*pow(t, 2);
}

std::deque<Edge> clipPath(GPath path, GRect sides){
  std::deque<Edge> edges;
  GPoint pts[4];
  GPath::Edger iter = GPath::Edger(path);
  GPath::Verb nextVb = iter.next(pts);
  while(nextVb != GPath::Verb::kDone){
    if (nextVb == GPath::Verb::kLine){
        clip(pts[0], pts[1], sides, edges);
    }else if(nextVb == GPath::Verb::kQuad){
      float D = vectorLength(pts[0].fX - 2*pts[1].fX + pts[2].fX, pts[0].fY - 2*pts[1].fY + pts[2].fY) / 4;
      int num = (int) ceil(sqrt(D / 0.25));
      float interval = 1.0f/num;
      float t = 0;
      float prevX = quad(pts[0].fX, pts[1].fX, pts[2].fX, t);
      float prevY = quad(pts[0].fY, pts[1].fY, pts[2].fY, t);
      for(int i = 0; i < num; ++i){
          t += interval;
          float currX = quad(pts[0].fX, pts[1].fX, pts[2].fX, t);
          float currY = quad(pts[0].fY, pts[1].fY, pts[2].fY, t);
          clip(GPoint::Make(prevX, prevY), GPoint::Make(currX, currY), sides, edges);
          prevX = currX;
          prevY = currY;
      }
    }else{
      float lenABC = vectorLength(pts[0].fX - 2*pts[1].fX + pts[2].fX, pts[0].fY - 2*pts[1].fY + pts[2].fY);
      float lenBCD = vectorLength(pts[1].fX - 2*pts[2].fX + pts[3].fX, pts[1].fY - 2*pts[2].fY + pts[3].fY);
      float D = 0.75f * std::max(lenABC, lenBCD);
      int num = (int) ceil(sqrt(D / 0.25));
      float interval = 1.0f/num;
      float t = 0;
      float prevX = cubic(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, t);
      float prevY = cubic(pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY, t);
      for(int i = 0; i < num; ++i){
          t += interval;
          float currX = cubic(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, t);
          float currY = cubic(pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY, t);
          clip(GPoint::Make(prevX, prevY), GPoint::Make(currX, currY), sides, edges);
          prevX = currX;
          prevY = currY;
      }
    }
    nextVb = iter.next(pts);
  }
  return edges;
}
