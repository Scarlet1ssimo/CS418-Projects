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
  virtual bool intersect(const Ray &r, double &t) const = 0;
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
  }
  bool intersect(const Ray &r, double &t) const override {
    vec3 e1 = b - a, e2 = c - a, s = r.origin() - a;
    vec3 s1 = cross(r.direction(), e2), s2 = cross(s, e1);
    if (abs(dot(s1, e1)) < hit_eps)
      return false;
    double coeff = 1.0 / dot(s1, e1);
    t = coeff * dot(s2, e2);
    double b1 = coeff * dot(s1, s), b2 = coeff * dot(s2, r.direction());
    if (t > hit_eps && b1 > -hit_eps && b2 > -hit_eps && b1 + b2 < 1 + hit_eps)
      return true;
    return false;
  }
  virtual vec3 getNormal(const vec3 &) const override { return constantNormal; }
  virtual vec3 getColor(const vec3 &) const override { return color; }
  vector<vec3> getBoundingVertices() const override { return {a, b, c}; }
};
struct TriangleText : TriangleSimple {
  vec3 sta, stb, stc;
  TriangleText() = delete;
  TriangleText(vec3 a, vec3 b, vec3 c, vec3 sta, vec3 stb, vec3 stc)
      : TriangleSimple(a, b, c), sta(sta), stb(stb), stc(stc) {}
  vec3 getColor(const vec3 &p) const override {
    if (material->tex && material->tex->valid) {
      vec3 bary = Barycentric(a, b, c, p);
      vec3 mix = sta * bary.x() + stb * bary.y() + stc * bary.z();
      double s = mix.x(), t = mix.y();
      return material->tex->getColor(s, t);
    } else
      return color;
  }
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
  inline bool intersect(const Ray &r, double &t) const override {
    double denom = dot(normal, r.direction());
    if (abs(denom) > hit_eps) {
      t = -(dot(normal, r.origin()) + d) / denom;
      if (t > hit_eps)
        return true;
    }
    return false;
  }
  inline vec3 getNormal(const vec3 &) const override { return constantNormal; }
};

struct Sphere : BoundableLeaf {
  vec3 center;
  double radius;
  Sphere() = delete;
  Sphere(const vec3 &c, double r) : center(c), radius(r) {}
  bool intersect(const Ray &r, double &t) const override {
    vec3 oc = r.origin() - center;
    double a = r.direction().squared_length();
    double b = dot(oc, r.direction());
    double c = dot(oc, oc) - radius * radius;
    double discriminant = b * b - a * c;
    if (discriminant < 0) {
      return false;
    } else {
      t = (-b - sqrt(discriminant)) / a;
      if (t > hit_eps)
        return true;
      t = (-b + sqrt(discriminant)) / a;
      if (t > hit_eps)
        return true;
      return false;
    }
  }
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
