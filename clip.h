#include "GPoint.h"
#include "GRect.h"
#include <algorithm>
#include <deque>

#ifndef CLIP_H
#define CLIP_H
/*
 * Edges
 */
struct Edge {
    int botY;
    int topY;
    float curX;
    float slope;
    int winding;

    Edge(GPoint p0, GPoint p1, int winding);
    bool operator==(const Edge& other) const;
};

Edge::Edge(GPoint p0, GPoint p1, int winding) {
  //Switch p0 and p1 if in wrong order
  if (p0.fY > p1.fY) {
      std::swap(p0, p1);
      winding = winding * -1;
  }

  //Set values and return
  this->curX = p0.fX;
  this->botY = GRoundToInt(p1.fY);
  this->topY = GRoundToInt(p0.fY);
  this->slope = (p1.fX - p0.fX) / (p1.fY - p0.fY);
  this->winding = winding;
}

/*
 * Clip edge with canvas, projecting onto edge when out of bounds
 */
static void clip(GPoint p0, GPoint p1, const GRect& sides,  std::deque<Edge>& edges) {
    //Ignore 0-height edges
    if (GRoundToInt(p0.fY) == GRoundToInt(p1.fY)) {
        return;
    }

    //Sort edges on y
    int winding = 1;
    if (p0.fY > p1.fY) {
        std::swap(p0, p1);
        winding = winding * -1;
    }

    // Ignore edges outside canvas vertically
    if (p1.fY <= sides.top() || p0.fY >= sides.bottom()) {
        return;
    }

    //Get dxdy slope for projections
    float slope = (p1.fX - p0.fX) / (p1.fY - p0.fY);

    // Clip on vertical boundaries
    if (p0.fY < sides.top()) {
        p0.fX += slope * (sides.top() - p0.fY);
        p0.fY = sides.top();
    }
    if (p1.fY > sides.bottom()) {
        p1.fX += slope * (sides.bottom() - p1.fY);
        p1.fY = sides.bottom();
    }

    //Sort on x
    if (p0.fX > p1.fX) {
        std::swap(p0, p1);
        winding = winding * -1;
    }

    //Project horizontal edges onto boundaries if outside
    if (p1.fX <= sides.left()) {
        p0.fX = p1.fX = sides.left();
        Edge* edge = new Edge(p0, p1, winding);
        edges.push_back(*edge);
        return;
    }
    if (p0.fX >= sides.right()) {
        p0.fX = p1.fX = sides.right();
        Edge* edge = new Edge(p0, p1, winding);
        edges.push_back(*edge);
        return;
    }

    //Clip and project left
    if (p0.fX < sides.left()) {
      float y = p0.fY + (sides.left() - p0.fX) / slope;
      Edge* edge = new Edge(GPoint::Make(sides.left(), p0.fY), GPoint::Make(sides.left(), y), winding);
      edges.push_back(*edge);
      p0.set(sides.left(), y);
    }

    //Clip and project right
    if (p1.x() > sides.right()) {
      float y = p0.fY + (sides.right() - p0.fX) / slope;
      Edge* edge = new Edge(GPoint::Make(sides.right(), y), GPoint::Make(sides.right(), p1.fY), winding);
      edges.push_back(*edge);
      p1.set(sides.right(), y);
    }

    //Add remaining points as edge
    Edge* edge = new Edge(p0, p1, winding);
    edges.push_back(*edge);
    return;
}


/**
 * Compares edges for sorting
 * Sorted by bot y, bot x, and then slope
 */
bool compareEdge(const Edge e1, const Edge e2) {
    if (e1.topY == e2.topY) {
      if (e1.curX == e2.curX) {
        return e1.slope <= e2.slope;
      }else{
        return e1.curX < e2.curX;
      }
    }else{
      return e1.topY < e2.topY;
    }
}

bool resortCompare(const Edge e1, const Edge e2) {
    return e1.curX < e2.curX;
}

bool Edge::operator==(const Edge& other) const{
    if(this->topY == other.topY &&
       this->botY == other.botY &&
       this->slope == other.slope &&
       this->curX == other.curX){
            return true;
    }
    return false;
}

#endif
