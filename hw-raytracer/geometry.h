#pragma once
#include "material.h"
#include "vec.h"
#include <bits/stdc++.h>
#include <memory>
#include <optional>

const double hit_eps = 1e-9;
const double INF = 1e300;
struct Ray {
  vec3 A, B;

public:
  Ray() {}
  Ray(const vec3 &origin, const vec3 &normed_dir, bool doNorm = false)
      : A(origin), B(normed_dir) {
    if (doNorm)
      B.make_unit_vector();
  }
  inline vec3 origin() const { return A; }
  inline vec3 direction() const { return B; }
  inline vec3 point_at_parameter(double t) const { return A + B * t; }
};

struct Leaf {
  vec3 color;
  shared_ptr<Material> material;
  // return true only if better than bestT
  virtual bool intersect(const Ray &r, double &bestT) const = 0;
  virtual vec3 getNormal(const vec3 &p) const = 0;
  virtual vec3 getColor(const vec3 &p) const { return color; }
};
struct Boundable {
  virtual vector<vec3> getBoundingVertices() const = 0;
};
struct BoundableLeaf : Leaf, Boundable {};

struct TriangleSimple : BoundableLeaf {
  vec3 a, b, c;
  vec3 constantNormal;
  TriangleSimple() = delete;
  TriangleSimple(vec3 a, vec3 b, vec3 c) : a(a), b(b), c(c) {
    constantNormal = unit_vector(cross(b - a, c - a));
  };
  bool intersect(const Ray &r, double &bestT) const override;
  virtual vec3 getNormal(const vec3 &) const override { return constantNormal; }
  virtual vec3 getColor(const vec3 &) const override { return color; }
  vector<vec3> getBoundingVertices() const override { return {a, b, c}; }
};
struct TriangleText : TriangleSimple {
  vec3 sta, stb, stc;
  TriangleText() = delete;
  TriangleText(vec3 a, vec3 b, vec3 c, vec3 sta, vec3 stb, vec3 stc)
      : TriangleSimple(a, b, c), sta(sta), stb(stb), stc(stc) {}
  vec3 getColor(const vec3 &p) const override;
};
struct TriangleInterpNormal : TriangleSimple {
  vec3 na, nb, nc;
  TriangleInterpNormal() = delete;
  TriangleInterpNormal(vec3 a, vec3 b, vec3 c, vec3 na, vec3 nb, vec3 nc)
      : TriangleSimple(a, b, c), na(na), nb(nb), nc(nc) {}
  vec3 getNormal(const vec3 &p) const override {
    vec3 bary = Barycentric(a, b, c, p);
    return unit_vector(bary.x() * na + bary.y() * nb + bary.z() * nc);
  }
};
struct Plane : Leaf {
  vec3 normal;
  vec3 constantNormal;
  double d;
  Plane() = delete;
  Plane(const vec3 &n, double d) : normal(n), d(d) {
    constantNormal = unit_vector(normal);
  }
  bool intersect(const Ray &r, double &bestT) const override;
  inline vec3 getNormal(const vec3 &) const override { return constantNormal; }
};

struct Sphere : BoundableLeaf {
  vec3 center;
  double radius;
  Sphere() = delete;
  Sphere(const vec3 &c, double r) : center(c), radius(r) {}
  bool intersect(const Ray &r, double &bestT) const override;
  inline vec3 getNormal(const vec3 &p) const override {
    return (p - center) / radius;
  }
  inline vec3 getColor(const vec3 &p) const override {
    if (material->tex && material->tex->valid) {
      // map vec3 to s and t
      vec3 normalized = getNormal(p);
      double s = (atan2(normalized.z(), -normalized.x())) / (2 * M_PI);
      double t = 0.5 - asin(normalized.y()) / (M_PI);
      return material->tex->getColor(s, t);
    } else
      return color;
  }
  inline vector<vec3> getBoundingVertices() const override {
    return {center + radius, center - radius};
  }
};

struct AABB : BoundableLeaf {
  double a, b, c, A, B, C;
  AABB() = delete;
  AABB(const vec3 &a, const vec3 &b) {
    this->a = a.x();
    this->b = a.y();
    this->c = a.z();
    this->A = b.x();
    this->B = b.y();
    this->C = b.z();
  }
  bool intersect(const Ray &r, double &bestT) const override;
  vec3 getNormal(const vec3 &p) const override;
  vector<vec3> getBoundingVertices() const override {
    return {vec3(a, b, c), vec3(A, B, C)};
  }
};
