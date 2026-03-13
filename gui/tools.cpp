#include "tools.h"
#include "../include/line.h"
#include "../include/circle.h"
#include "../include/bezier.h"
#include "../include/point.h"
#include "input_map.h"   // 为了 sync_pen
#include "../include/options.h"
#include <vector>
#include <iostream>

static void sync_pen(RenderCore::RenderEngine& eng) {
    RenderCore::PenOptions opt;
    opt.color = InputMap::inst().color();
    opt.width = InputMap::inst().width();
    opt.type  = InputMap::inst().pen();
    opt.color.a = 1.0f;
    eng.set_pen_options(opt);
}

// ---------- LineTool ----------
struct LineTool : Tool {
    RenderCore::Point start;
    int preview = -1;
    void onLButtonDown(RenderCore::RenderEngine& eng,
                       const RenderCore::Point& p) override {
        sync_pen(eng);
        start = p;
        preview = -1;
    }
    void onMouseMove(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (preview >= 0) eng.remove_primitive(preview);
        preview = eng.add_primitive(RenderCore::make_line(start, p));
        eng.render();
    }
    void onLButtonUp(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (preview >= 0) eng.remove_primitive(preview);
        eng.add_primitive(RenderCore::make_line(start, p));
        eng.render();
        preview = -1;
    }
    void onRButtonDown(RenderCore::RenderEngine& eng,const RenderCore::Point& p)override{
        return;
    }
};

// ---------- CircleTool ----------
struct CircleTool : Tool {
    RenderCore::Point center;
    int preview = -1;
    void onLButtonDown(RenderCore::RenderEngine& eng,
                       const RenderCore::Point& p) override {
        sync_pen(eng);
        center = p;
        preview = -1;
    }
    void onMouseMove(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (preview >= 0) eng.remove_primitive(preview);
        int r = (int)RenderCore::vector_length(p - center);
        preview = eng.add_primitive(
            RenderCore::make_circle_center_radius(center, r));
        eng.render();
    }
    void onLButtonUp(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (preview >= 0) eng.remove_primitive(preview);
        int r = (int)RenderCore::vector_length(p - center);
        eng.add_primitive(RenderCore::make_circle_center_radius(center, r));
        eng.render();
        preview = -1;
    }
    void onRButtonDown(RenderCore::RenderEngine& eng,const RenderCore::Point& p)override{
        return;
    }
};

// ---------- ArcTool（三点弧） ----------
struct ArcTool : Tool {
    std::vector<RenderCore::Point> pts;
    int preview = 0;
    void onLButtonDown(RenderCore::RenderEngine& eng,
                       const RenderCore::Point& p) override {          
        sync_pen(eng);
        pts.push_back(p);
        preview++;
    }
    void onMouseMove(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if(preview < 2) return;
        
        // 只有起点+当前点，先画直线示意
        eng.add_primitive(RenderCore::make_arc_three_points(pts[0],pts[1],p));
        eng.render();
        eng.remove_primitive();
    }
    void onLButtonUp(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if(preview < 2) return;
        pts.push_back(p);
        // 三点确定弧
        eng.add_primitive(RenderCore::make_arc_three_points(pts[0], pts[1], pts[2]));
        eng.render();
        pts.clear();
        preview = 0;
    }
    void onRButtonDown(RenderCore::RenderEngine& eng,const RenderCore::Point& p)override{
        return;
    }
};

// ---------- BezierTool（任意次，左键连点，右键闭合） ----------
struct BezierTool : Tool {
    std::vector<RenderCore::Point> cpts;
    int preview = 0;
    void onLButtonDown(RenderCore::RenderEngine& eng,
                       const RenderCore::Point& p) override {
        sync_pen(eng);
        if(preview>1) return;
        cpts.push_back(p);
        preview++;
    }
    void onMouseMove(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (cpts.empty()) return;
        if (preview < 2) return;
        cpts.push_back(p);

        eng.add_primitive(RenderCore::make_polygon(cpts));
        eng.add_primitive(RenderCore::make_bezier_curve(cpts));
        // 临时画控制折线
        eng.render();
        cpts.pop_back();
        eng.remove_primitive();
        eng.remove_primitive();
    }
    void onLButtonUp(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        // 右键闭合：点数>=2 就生成曲线
        if(preview < 2) return;
        preview ++;
        cpts.push_back(p);
        if(preview < 5){
            eng.add_primitive(RenderCore::make_bezier_curve(cpts));
            eng.render();
            eng.remove_primitive();
            return;
        }
        eng.add_primitive(RenderCore::make_bezier_curve(cpts));
        eng.render();
        cpts.clear();
        preview = 0;
    }
    void onRButtonDown(RenderCore::RenderEngine& eng,const RenderCore::Point& p)override{
        return;
    }
};

// ---------- BsplineTool（任意次，左键连点，右键闭合） ----------
struct BsplineTool : Tool {
    std::vector<RenderCore::Point> cpts;
    int preview = -1;
    int reg = 0;
    void onLButtonDown(RenderCore::RenderEngine& eng,
                       const RenderCore::Point& p) override { 
        if(reg < 2){
            if(reg < 1) cpts.push_back(p);
            reg ++ ;
        }
        sync_pen(eng);
        preview = -1;
    }
    void onMouseMove(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (cpts.empty()) return;
        if (reg < 2) return;
        cpts.push_back(p);
        if (preview >= 0) eng.remove_primitive(preview);
        preview = eng.add_primitive(RenderCore::make_polygon(cpts));
        eng.render();
        cpts.pop_back();
    }
    void onLButtonUp(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (preview >= 0) eng.remove_primitive(preview);
        if(reg < 2) return;
        cpts.push_back(p);
        if (cpts.size() >= 5) { // 至少 5 个控制点
            // 简单生成均匀开节点矢量
            size_t n = cpts.size();
            size_t p = 3; // 三次
            std::vector<float> knots(n + p + 1);
            for (size_t i = 0; i <= n + p; ++i) knots[i] = float(i);
            eng.add_primitive(RenderCore::make_bspline_curve(cpts, knots));
            eng.render();
            cpts.clear();
            reg =0;
        }
    }
    void onRButtonDown(RenderCore::RenderEngine& eng,const RenderCore::Point& p)override{
        return;
    }
};

struct PolygenTool: Tool {
    std::vector<RenderCore::Point> pts;
    int preview = 0;
    void onLButtonDown(RenderCore::RenderEngine& eng,
                       const RenderCore::Point& p) override { 
        sync_pen(eng);
        if(preview > 1) return;
        if(preview == 1) {
            preview ++ ;
            return;
        }
        pts.push_back(p);
        preview ++ ;
    }
    void onMouseMove(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if (pts.empty()) return;
        if (preview < 2) return;
        pts.push_back(p);
        eng.add_primitive(RenderCore::make_polygon(pts));
        eng.render();
        eng.remove_primitive();
        pts.pop_back();
    }
    void onLButtonUp(RenderCore::RenderEngine& eng,
                     const RenderCore::Point& p) override {
        if(preview < 2) return;
        pts.push_back(p);
        eng.add_primitive(RenderCore::make_polygon(pts));
        eng.render();
        if (pts.size() >= 5) { // 至少 5 个控制点
            pts.clear();
            preview = 0;
            return;
        }
        eng.remove_primitive();
    }
    void onRButtonDown(RenderCore::RenderEngine& eng,const RenderCore::Point& p)override{
        return;
    }

};

// ---------- 工厂 ----------
std::unique_ptr<Tool> makeLineTool()  { return std::make_unique<LineTool>(); }
std::unique_ptr<Tool> makeCircleTool(){ return std::make_unique<CircleTool>(); }
std::unique_ptr<Tool> makeArcTool()   { return std::make_unique<ArcTool>(); }
std::unique_ptr<Tool> makeBezierTool(){ return std::make_unique<BezierTool>(); }
std::unique_ptr<Tool> makeBsplineTool(){return std::make_unique<BsplineTool>();}
std::unique_ptr<Tool> makePolygenTool(){return std::make_unique<PolygenTool>();}