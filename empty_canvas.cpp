#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GMath.h"
#include "GPixel.h"
#include "GRect.h"
#include "GPoint.h"
#include "GPaint.h"
#include "GShader.h"
#include "GFilter.h"
#include <stack>
#include <tuple>
#include <deque>
#include "matrix.h"
#include "clip.h"
#include "layer.h"
#include "path.h"
#include "blend.h"
#include "Utils.h"
#include "math.h"
#include "pathEdger.h"
#include "RadialGradientShader.h"
#include "TriangleGradientShader.h"

using namespace std;

class EmptyCanvas : public GCanvas {
  public:
    EmptyCanvas(const GBitmap& device) : fDevice(device), fCTMStack(), fLayerStack() {
      GPoint trans = GPoint::Make(0, 0);
      GPaint* paint = new GPaint();
      Layer* layer = new Layer(device, trans, *paint);
      fLayerStack.push(*layer);
      fLayerBool.push(true);

      GMatrix I;
      I.setIdentity();
      fCTMStack.push(I);
      fLayerBool.push(false);
    }

////////////////// Final Methods /////////////////////////////

std::unique_ptr<GShader> final_createRadialGradient(
      GPoint center, float radius, const GColor colors[], int count) override{
      if(count < 2){return nullptr;}
      RadialGradientShader* r = new RadialGradientShader(center, radius, colors, count);
      return std::unique_ptr<GShader>(r);
}

virtual std::unique_ptr<GShader> final_createTriangleGradient(const GPoint pts[3],
                                                                      const GColor colors[3]) override {
      TriangleGradientShader* t = new TriangleGradientShader(pts, colors);
      return std::unique_ptr<GShader>(t);
}
////////////////// DRAW METHODS ///////////////////////////

    /*
     * Fill canvas with a single color
     */
    void drawPaint(const GPaint& paint) override {
      for(int y = 0; y < fLayerStack.top().bitmap.height(); ++y){
          drawRow(0, fLayerStack.top().bitmap.width(), y, paint);
      }
    }

    /*
     * Fill a rectangle with given locations. Blend with canvas color.
     */
    void drawRect(const GRect& rect, const GPaint& paint) override {
        GPoint points[4] = {
               GPoint::Make(rect.left(), rect.top()),
               GPoint::Make(rect.right(), rect.top()),
               GPoint::Make(rect.right(), rect.bottom()),
               GPoint::Make(rect.left(), rect.bottom())
        };
        drawConvexPolygon(points, 4, paint);
    }

    /*
     * Clip and draw arbitrary convex polygon
     */
    virtual void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
        //Map point locations from CTM. Must use auxillary array due to method signature.
        GPoint CTMpoints[count];
        fCTMStack.top().mapPoints(CTMpoints, points, count);


        GBitmap layer = fLayerStack.top().bitmap;

        //Correct for translation of current layer. (I should do this in CTM but not right now)
        GPoint translation = fLayerStack.top().translation;
        for(int i = 0; i < count; ++i){
            CTMpoints[i].fX += -translation.x();
            CTMpoints[i].fY += -translation.y();
        }
        //Start by building edge deque and ordering them correctly
        GRect sides = GRect::MakeWH(layer.width(), layer.height());

        //Clip edges and place in deque - for autosizing, front&back access
        std::deque<Edge> edges;
        for (int i = 0; i < count; ++i) {
          GPoint p0 = CTMpoints[i];
          GPoint p1 = CTMpoints[(i + 1) % count];
          clip(p0, p1, sides, edges);
        }

        // We only draw between edges: 0 or 1 has no result
        if(edges.size() < 2){
          return;
        }

        //Sort using predicate function defined in clip.cpp
        std::sort(edges.begin(), edges.end(), compareEdge);

        // Set up boundary conditions
        Edge left = edges.front();
        edges.pop_front();
        Edge right = edges.front();
        edges.pop_front();

        int y = GRoundToInt(left.topY);
        float leftX = left.curX;
        float rightX = right.curX;
        int bottom =  GRoundToInt(edges.back().botY);
        // Draw 1-Pixel high rectangles for each row
        for(y; y < bottom; ++y) {
            int l = GRoundToInt(std::min(leftX, rightX));
            int r = GRoundToInt(std::max(leftX, rightX));
            drawRow(l, r, y, paint);

            //Check to see if completed left or right edge
            //If so, replace with next edge
            if (y >= left.botY) {
                left = edges.front();
                edges.pop_front();
                leftX = left.curX;
            } else {
                leftX += left.slope;
            }

            if (y >= right.botY) {
                right = edges.front();
                edges.pop_front();
                rightX = right.curX;
            } else {
                rightX += right.slope;
            }
        }
    }

    virtual void drawPath(const GPath& path, const GPaint& paint) override {
        GPath nPath = path;
        nPath.transform(fCTMStack.top());
        GBitmap layer = fLayerStack.top().bitmap;
        GRect sides = GRect::MakeWH(layer.width(), layer.height());

        std::deque<Edge> edges;
        edges = clipPath(nPath, sides);
        // We only draw between edges: 0 or 1 has no result
        if(edges.size() < 2){
          return;
        }

        //Sort using predicate function defined in clip.cpp
        std::sort(edges.begin(), edges.end(), compareEdge);

        int y = edges.front().topY;
        float x0;
        float x1;
        int winding = 0;
        std::deque<Edge>::iterator edge;
        while(y < GRoundToInt(sides.bottom())){
            edge = edges.begin();
            winding = 0;
            while (edge != edges.end() && edge->topY <= y){
                if (winding == 0){
                    x0 = edge->curX;
                }
                winding += edge->winding;
                if (winding == 0){
                    x1 = edge->curX;
                    if(x0 > x1){
                        std::swap(x0, x1);
                    }
                    drawRow(GRoundToInt(x0), GRoundToInt(x1), y, paint);
                }
                if (edge->botY <= y + 1) {	// we’re done with edge
                    Edge toErase = *edge;
                    if(toErase == edges.back()){
                        edges.pop_back();
                        edge = edges.begin();
                    }else{
                        edges.erase(std::remove(edges.begin(), edges.end(), toErase), edges.end());
                    }
                    if(edges.empty()){return;}
                } else {
                    float newCurX = edge->curX + edge->slope;
                    edge->curX = newCurX;
                    std::sort(edges.begin(), ++edge, resortCompare);
                }
            }
            ++y;
            while (edge != edges.end() && edge->topY == y) { // pick up new edges for the next line
                std::sort(edges.begin(), ++edge, resortCompare);
            }
        }
        return;
    }

////////////////// MATRIX METHODS ///////////////////////////

    virtual void concat(const GMatrix& matrix) override{
      fCTMStack.top().preConcat(matrix);
    }

    virtual void restore() override {
        if(fLayerBool.top()){
          //do layer stuff
          Layer layer = fLayerStack.top();
          fLayerStack.pop();
          layer.drawLayer(fLayerStack.top());
        }else{
          fCTMStack.pop();
        }
        fLayerBool.pop();
    }

    virtual void save() override {
        GMatrix top = fCTMStack.top();
        GMatrix deepCopy(
            top[0], top[1], top[2],
            top[3], top[4], top[5]);

        fCTMStack.push(deepCopy);
        fLayerBool.push(false);
    }

  protected:
    virtual void onSaveLayer(const GRect* bounds, const GPaint& GPaint) {
        GBitmap bitmap =  fLayerStack.top().bitmap;
        GPoint bitmapTranslation = fLayerStack.top().translation;
        GMatrix ctm =  fCTMStack.top();
        fLayerBool.push(true);
        if (bounds == nullptr){
            int width  = bitmap.width();
            int height = bitmap.height();
            GBitmap* bmap = new GBitmap();
            bmap->alloc(width, height);

            Layer* layer = new Layer(*bmap, bitmapTranslation, GPaint);
            fLayerStack.push(*layer);
        }else{
            //Convert bounds to mapped points
            GPoint CTMpoints[4] = {
                 GPoint::Make(bounds->left(), bounds->top()),
                 GPoint::Make(bounds->right(), bounds->top()),
                 GPoint::Make(bounds->right(), bounds->bottom()),
                 GPoint::Make(bounds->left(), bounds->bottom())
            };
            ctm.mapPoints(CTMpoints, CTMpoints, 4);

            //Find bounds. Ugly but should work. Modularize later.
            float left   = CTMpoints[0].x();
            float right  = CTMpoints[0].x();
            float top    = CTMpoints[0].y();
            float bottom = CTMpoints[0].y();
            for(int i = 1; i < 4; ++i){
                  if(CTMpoints[i].x() < left){
                    left = CTMpoints[i].x();
                  }
                  if(CTMpoints[i].x() > right){
                    right = CTMpoints[i].x();
                  }
                  if(CTMpoints[i].y() < top){
                    top = CTMpoints[i].y();
                  }
                  if(CTMpoints[i].y() > bottom){
                    bottom = CTMpoints[i].y();
                  }
            }
            left  = max(left, (float) bitmapTranslation.x());
            right = min(right, (float) bitmap.width() + bitmapTranslation.x());
            top = max(top, (float) bitmapTranslation.y());
            bottom = min(bottom, (float) bitmap.height() + bitmapTranslation.y());

            int width = GRoundToInt(right - left);
            int height = GRoundToInt(bottom - top);
            if(width <= 0 || height <= 0){
              fLayerStack.push(fLayerStack.top());
              return;
            }
            GBitmap* newBitmap = new GBitmap();
            newBitmap->alloc(width, height);

            GPoint translation = GPoint::Make(GRoundToInt(left), GRoundToInt(top));
            Layer* layer = new Layer(*newBitmap, translation, GPaint);
            fLayerStack.push(*layer);
        }
    }

  private:
    const GBitmap fDevice;
    std::stack<GMatrix> fCTMStack;
    std::stack<bool> fLayerBool;
    std::stack<Layer> fLayerStack;

    void drawRow(int leftX, int rightX, int y, const GPaint& paint){
        if(leftX == rightX){return;}
        GBitmap bitmap = fLayerStack.top().bitmap;
        leftX = std::max(0, leftX);
        rightX = std::min(bitmap.width(), rightX);
        y = std::min(bitmap.height() - 1, y);
        int count = rightX - leftX;

        //Get paint info
        Blend blend = getBlend(paint.getBlendMode());
        GShader* shader = paint.getShader();
        GFilter* filter = paint.getFilter();

        if(shader){
            if (!shader->setContext(fCTMStack.top())) {
                return;
            }
            GPixel thisRow[count];
            shader->shadeRow(leftX, y, count, thisRow);

            if(filter){
              filter->filter(thisRow, thisRow, count);
            }

            //Blend and fill row memory with shaded pixels
            for (int x = leftX; x < rightX; ++x) {
                GPixel* addr = bitmap.getAddr(x, y);
                *addr = blend(thisRow[x - leftX], *addr);
            }
        }else{
            GPixel src[1] = {colortoPixel(paint.getColor())};

            if(filter){
              filter->filter(src, src, 1);
            }

            for (int x = leftX; x < rightX; ++x) {
                GPixel* addr = bitmap.getAddr(x, y);
                *addr = blend(src[0], *addr);
            }
        }
    }

};
/*
 * Canvas factory
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }
    return std::unique_ptr<GCanvas>(new EmptyCanvas(device));
}
