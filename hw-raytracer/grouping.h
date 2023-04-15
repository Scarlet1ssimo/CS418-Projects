#pragma once
#include "geometry.h"
#include <X11/Xlib.h>
#include <bits/stdc++.h>
#include <sstream>
using namespace std;
struct FastGroup : Boundable {
  vector<shared_ptr<BoundableLeaf>> Lchild;
  vector<shared_ptr<FastGroup>> Gchild;
  void extendBy(shared_ptr<Boundable> l) {
    for (auto &p : l->getBoundingVertices())
      extendByPoint(p);
  }
  void extendBy(const Boundable &l) {
    for (auto &p : l.getBoundingVertices())
      extendByPoint(p);
  }
  void add(shared_ptr<BoundableLeaf> l) {
    extendBy(l);
    Lchild.push_back(l);
  }
  void add(shared_ptr<FastGroup> g) {
    extendBy(g);
    Gchild.push_back(g);
  }
  virtual void extendByPoint(const vec3 &p) = 0;
  virtual bool fastIntersect(const Ray &r, const double &bestT) const = 0;
  string str() const {
    ostringstream oss;
    oss << "FG(" << Lchild.size();
    for (auto &p : Gchild)
      oss << "," << p->str();
    oss << ")";
    return oss.str();
  }
  virtual void selfBalance() = 0;
  virtual double metric() = 0;
  virtual double ndmetric() = 0;
};
struct BVH : FastGroup {
  double a, b, c, A, B, C;
  BVH() : a(INF), b(INF), c(INF), A(-INF), B(-INF), C(-INF){};
  void reset() {
    a = INF;
    b = INF;
    c = INF;
    A = -INF;
    B = -INF;
    C = -INF;
  }
  bool fastIntersect(const Ray &r, const double &bestT) const override;
  void extendByPoint(const vec3 &p) override {
    a = min(a, p.x());
    b = min(b, p.y());
    c = min(c, p.z());
    A = max(A, p.x());
    B = max(B, p.y());
    C = max(C, p.z());
  }
  vector<vec3> getBoundingVertices() const override {
    return {vec3(a, b, c), vec3(A, B, C)};
  }
  virtual void selfBalance() override{};
  double metric() override { return (A - a) * (B - b) * (C - c); }
  double ndmetric() override {
    if (abs(A - a) < hit_eps)
      return (B - b) * (C - c);
    if (abs(B - b) < hit_eps)
      return (A - a) * (C - c);
    if (abs(C - c) < hit_eps)
      return (A - a) * (B - b);
    return (A - a) * (B - b) * (C - c);
  }
  inline double operator[](int i) const {
    switch (i) {
    case 0:
      return a;
    case 1:
      return b;
    case 2:
      return c;
    case 3:
      return A;
    case 4:
      return B;
    case 5:
      return C;
    }
    return 0;
  }
};
struct HitRecord {
  double t;
  shared_ptr<Leaf> obj;
  HitRecord() = delete;
  HitRecord(double t, shared_ptr<Leaf> obj) : t(t), obj(obj) {}
};
static inline optional<HitRecord> makeRecord(const Ray &r, shared_ptr<Leaf> l,
                                             double &bestT) {
  if (l->intersect(r, bestT))
    return HitRecord{bestT, l};
  return {};
}
// Only return HitRecord if better than bestT
// And only decreases bestT
optional<HitRecord> makeRecordHelper(const Ray &r, shared_ptr<FastGroup> g,
                                     double &bestT);
optional<HitRecord> makeRecord(const Ray &r, shared_ptr<FastGroup> g,
                               double &bestT);

struct Light {
  vec3 color;
  virtual vec3 getDir(vec3 p) = 0;
  virtual double getT(vec3 p) = 0;
  virtual vec3 getColor(vec3 p) = 0;
};
struct Sun : Light {
  vec3 lightDir;
  Sun() = delete;
  Sun(const vec3 &dir) : lightDir(unit_vector(dir)) {}
  inline vec3 getDir(vec3) override { return lightDir; }
  inline double getT(vec3) override { return 1e300; }
  inline vec3 getColor(vec3) override { return color; }
};
struct Bulb : Light {
  vec3 coord;
  Bulb() = delete;
  Bulb(const vec3 &p) : coord(p) {}
  inline vec3 getDir(vec3 p) override { return unit_vector(coord - p); }
  inline double getT(vec3 p) override { return (coord - p).length(); }
  inline vec3 getColor(vec3 p) override {
    return color / (coord - p).squared_length();
  }
};
struct ScarletBVH : BVH {
  void selfBalance() override;
  void settleLayer(double successRate);
  bool Scarlet();
  bool Amethyst(double successRate);
  bool Amethyst2D();
  void PackLchild();
  void ReleaseSingleGchild();
};

struct World {
  vector<shared_ptr<Leaf>> objs;
  vector<shared_ptr<Light>> lights;
  shared_ptr<BVH> bvh;
  // vector<shared_ptr<Material>> materials;
  World();
  void add(shared_ptr<Leaf> obj) {
    if (auto leaf = dynamic_pointer_cast<BoundableLeaf>(obj)) {
      bvh->add(leaf);
    } else
      objs.push_back(obj);
  }
  void add(shared_ptr<Light> light) { lights.push_back(light); }
  optional<HitRecord> intersect(const Ray &r) const;
  optional<vec3> getColor(const Ray &r, int bounces, int gid) const;
};
