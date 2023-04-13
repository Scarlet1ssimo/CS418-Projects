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
  void add(shared_ptr<BoundableLeaf> l) {
    extendBy(l);
    Lchild.push_back(l);
  }
  void add(shared_ptr<FastGroup> g) {
    extendBy(g);
    Gchild.push_back(g);
  }
  virtual void extendByPoint(const vec3 &p) = 0;
  virtual bool fastIntersect(const Ray &r) const = 0;
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
  bool fastIntersect(const Ray &r) const override {
    double txmin = (a - r.origin().x()) / r.direction().x();
    double txmax = (A - r.origin().x()) / r.direction().x();
    if (txmin > txmax)
      swap(txmin, txmax);
    double tymin = (b - r.origin().y()) / r.direction().y();
    double tymax = (B - r.origin().y()) / r.direction().y();
    if (tymin > tymax)
      swap(tymin, tymax);
    if (txmin > tymax || tymin > txmax)
      return false;
    if (tymin > txmin)
      txmin = tymin;
    if (tymax < txmax)
      txmax = tymax;
    double tzmin = (c - r.origin().z()) / r.direction().z();
    double tzmax = (C - r.origin().z()) / r.direction().z();
    if (tzmin > tzmax)
      swap(tzmin, tzmax);
    if (txmin > tzmax || tzmin > txmax)
      return false;
    if (tzmin > txmin)
      txmin = tzmin;
    if (tzmax < txmax)
      txmax = tzmax;
    // cout << "txmin: " << txmin << " txmax: " << txmax << endl;
    return txmax >= hit_eps;
  }
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
  void selfBalance() override{};
  double metric() override { return (A - a) * (B - b) * (C - c); }
};
struct HitRecord {
  double t;
  shared_ptr<Leaf> obj;
  HitRecord() = delete;
  HitRecord(double t, shared_ptr<Leaf> obj) : t(t), obj(obj) {}
};
static inline optional<HitRecord> makeRecord(const Ray &r, shared_ptr<Leaf> l) {
  double t;
  if (l->intersect(r, t))
    return HitRecord{t, l};
  return {};
}
static inline optional<HitRecord> makeRecord(const Ray &r,
                                             shared_ptr<FastGroup> g) {
  if (!g->fastIntersect(r)) {
    // cout << "f" << endl;
    return {};
  }
  optional<HitRecord> res;
  for (auto &l : g->Lchild) {
    auto tmp = makeRecord(r, l);
    if (tmp && (!res || tmp->t < res->t)) {
      res = tmp;
    }
  }
  for (auto &l : g->Gchild) {
    auto tmp = makeRecord(r, l);
    if (tmp && (!res || tmp->t < res->t)) {
      res = tmp;
    }
  }
  return res;
}

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
  void selfBalance() {
    if (Lchild.size() < 8)
      return;
    double mymetric = metric();
    double ma = (a + A) / 2, mb = (b + B) / 2, mc = (c + C) / 2;
    vector<shared_ptr<BoundableLeaf>> leaves;
    vector<shared_ptr<ScarletBVH>> tmp(8);
    for (int i = 0; i < 8; i++) {
      tmp[i] = make_shared<ScarletBVH>();
    }
    swap(Lchild, leaves);
    for (auto &l : leaves) {
      auto B = BVH();
      B.add(l);
      if (B.metric() > mymetric / 8) {
        Lchild.push_back(l);
      } else {
        int i = 0, j = 0, k = 0;
        if (B.A >= ma)
          i = 1;
        if (B.B >= mb)
          j = 1;
        if (B.C >= mc)
          k = 1;
        int idx = i * 4 + j * 2 + k;
        tmp[idx]->add(l);
      }
    }
    int nonempty = 0;
    for (auto &bvh : tmp) {
      if (!bvh->Lchild.empty())
        nonempty++;
    }
    if (nonempty <= 1) {
      for (auto &bvh : tmp) {
        for (auto &l : bvh->Lchild) {
          Lchild.push_back(l);
        }
      }
    } else {
      for (auto &bvh : tmp) {
        if (bvh->Lchild.size() <= 1) {
          for (auto &l : bvh->Lchild) {
            Lchild.push_back(l);
          }
        } else {
          bvh->selfBalance();
          Gchild.push_back(bvh);
        }
      }
    }
  }
};
struct World {
  vector<shared_ptr<Leaf>> objs;
  vector<shared_ptr<Light>> lights;
  shared_ptr<BVH> bvh;
  // vector<shared_ptr<Material>> materials;
  World() : bvh(make_shared<ScarletBVH>()) {}
  void add(shared_ptr<Leaf> obj) {
    if (auto leaf = dynamic_pointer_cast<BoundableLeaf>(obj)) {
      bvh->add(leaf);
    } else
      objs.push_back(obj);
  }
  void add(shared_ptr<Light> light) { lights.push_back(light); }
  optional<HitRecord> intersect(const Ray &r) const {
    auto res = makeRecord(r, bvh);
    for (auto &obj : objs) {
      auto tmp = makeRecord(r, obj);
      if (tmp && (!res || tmp->t < res->t)) {
        res = tmp;
      }
    }
    return res;
  }

  optional<vec3> getColor(const Ray &r, int bounces, int gid) const {
    auto rec = intersect(r);
    if (rec.has_value()) {
      vec3 p = r.point_at_parameter(rec.value().t); // intersection
      vec3 outwardNormal = rec.value().obj->getNormal(p);
      auto material = rec.value().obj->material;

      if (material->roughness > 0)
        outwardNormal =
            unit_vector(outwardNormal + random_gaussian(material->roughness));

      vec3 facingNormal = dot(outwardNormal, r.direction()) > 0 ? -outwardNormal
                                                                : outwardNormal;

      vec3 shininess = material->shininess;
      vec3 transparency = material->transparency;
      double ior = material->ior;

      vec3 specularColor = vec3(0, 0, 0);
      if (shininess.squared_length() > 0 && bounces > 0)
        specularColor =
            getColor(Ray(p, reflect(r.direction(), facingNormal), true),
                     bounces - 1, gid)
                .value_or(vec3(0, 0, 0));

      vec3 refractiveColor = vec3(0, 0, 0);
      if (transparency.squared_length() > 0 && bounces > 0) {
        bool into_obj = dot(outwardNormal, facingNormal) > 0;
        double ni_over_nt = into_obj ? 1.0 / ior : ior;
        refractiveColor =
            getColor(Ray(p,
                         refract_or_total_reflect(r.direction(), facingNormal,
                                                  ni_over_nt),
                         true),
                     bounces - 1, gid)
                .value_or(vec3(0, 0, 0));
      }

      vec3 objColor = rec.value().obj->getColor(p);

      vec3 diffuseColor = vec3(0, 0, 0);
      if (gid > 0) {
        vec3 dir = facingNormal + random_in_unit_sphere();
        Ray giRay(p, dir, true); // Ray from p to random dir
        auto giColor = getColor(giRay, bounces - 1, gid - 1);

        auto intensity = dot(dir, facingNormal);
        if (giColor)
          diffuseColor += objColor * giColor.value() * max(intensity, 0.0);
      }
      for (auto &light : lights) {
        vec3 dir = light->getDir(p); // Get light to source
        Ray shadowRay(p, dir);       // Ray from p to light
        auto shadowRec = intersect(shadowRay);
        if (shadowRec.has_value() && shadowRec->t < light->getT(p))
          continue;

        auto intensity = dot(dir, facingNormal);
        auto lightColor = objColor * light->getColor(p) * max(intensity, 0.0);
        diffuseColor += lightColor;
      }
      return specularColor * shininess +
             refractiveColor * (1 - shininess) * transparency +
             diffuseColor * (1 - shininess) * (1 - transparency);
    } else {
      return {};
    }
  }
};
