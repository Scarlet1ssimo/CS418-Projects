#pragma once
#include "geometry.h"
static inline vec3 Barycentric(const vec3 &a, const vec3 &b, const vec3 &c,
                               const vec3 &p) {
  vec3 v0 = b - a, v1 = c - a, v2 = p - a;
  double d00 = dot(v0, v0);
  double d01 = dot(v0, v1);
  double d11 = dot(v1, v1);
  double d20 = dot(v2, v0);
  double d21 = dot(v2, v1);
  double denom = d00 * d11 - d01 * d01;
  double v = (d11 * d20 - d01 * d21) / denom;
  double w = (d00 * d21 - d01 * d20) / denom;
  double u = 1.0 - v - w;
  return vec3(u, v, w);
}
struct Sphere : Boundable {
  vec3 center;
  double radius;
  Sphere() = delete;
  Sphere(const vec3 &c, double r) : center(c), radius(r) {}
  optional<HitRecord> intersect(const Ray &r) const override {
    vec3 oc = r.origin() - center;
    double a = r.direction().squared_length();
    double b = dot(oc, r.direction());
    double c = dot(oc, oc) - radius * radius;
    double discriminant = b * b - a * c;
    if (discriminant < 0) {
      return {};
    } else {
      double t = (-b - sqrt(discriminant)) / a;
      if (t > hit_eps) {
        HitRecord res;
        res.t = t;
        res.obj = make_shared<Sphere>(*this);
        return res;
      }
      t = (-b + sqrt(discriminant)) / a;
      if (t > hit_eps) {
        HitRecord res;
        res.t = t;
        res.obj = make_shared<Sphere>(*this);
        return res;
      }
      return {};
    }
  }
  inline vec3 getNormal(const vec3 &p) const override {
    return (p - center) / radius;
  }
  inline vec3 getColor(const vec3 &p) const override {
    if (material->tex && material->tex->valid) {
      // map vec3 to s and t
      vec3 normalized = unit_vector(p - center);
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
struct Triangle : Boundable {
  vec3 a, b, c;
  vec3 sta, stb, stc;
  Triangle() = delete;
  Triangle(const vec3 &a, const vec3 &b, const vec3 &c) : a(a), b(b), c(c) {
    normedNormalCalculated = true;
    normedNormal = unit_vector(cross(b - a, c - a));
  }
  Triangle(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &sta,
           const vec3 &stb, const vec3 &stc)
      : a(a), b(b), c(c), sta(sta), stb(stb), stc(stc) {
    normedNormalCalculated = true;
    normedNormal = unit_vector(cross(b - a, c - a));
  }
  optional<HitRecord> intersect(const Ray &r) const override {
    // Möller-Trumbore algorithm
    vec3 e1 = b - a, e2 = c - a, s = r.origin() - a;
    vec3 s1 = cross(r.direction(), e2), s2 = cross(s, e1);
    if (abs(dot(s1, e1)) < hit_eps)
      return {};
    double coeff = 1.0 / dot(s1, e1), t = coeff * dot(s2, e2),
           b1 = coeff * dot(s1, s), b2 = coeff * dot(s2, r.direction());
    if (t > hit_eps && b1 > -hit_eps && b2 > -hit_eps &&
        b1 + b2 < 1 + hit_eps) {
      HitRecord res;
      res.t = t;
      res.obj = make_shared<Triangle>(*this); // How to merge this?
      return res;
    }
    return {};
  }
  vec3 getNormal(const vec3 &) const override { return normedNormal; }
  vec3 getColor(const vec3 &p) const override {
    if (material->tex && material->tex->valid) {
      vec3 bary = Barycentric(a, b, c, p);
      vec3 mix = sta * bary.x() + stb * bary.y() + stc * bary.z();
      double s = mix.x(), t = mix.y();
      return material->tex->getColor(s, t);
    } else
      return color;
  }
  vector<vec3> getBoundingVertices() const override { return {a, b, c}; }
};

struct TriangleNormal : Triangle {
  vec3 na, nb, nc;
  TriangleNormal() = delete;
  TriangleNormal(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &na,
                 const vec3 &nb, const vec3 &nc)
      : Triangle(a, b, c), na(na), nb(nb), nc(nc) {}
  optional<HitRecord> intersect(const Ray &r) const override {
    // Möller-Trumbore algorithm
    vec3 e1 = b - a, e2 = c - a, s = r.origin() - a;
    vec3 s1 = cross(r.direction(), e2), s2 = cross(s, e1);
    if (abs(dot(s1, e1)) < hit_eps)
      return {};
    double coeff = 1.0 / dot(s1, e1), t = coeff * dot(s2, e2),
           b1 = coeff * dot(s1, s), b2 = coeff * dot(s2, r.direction());
    if (t > hit_eps && b1 > hit_eps && b2 > hit_eps && b1 + b2 < 1 + hit_eps) {
      HitRecord res;
      res.t = t;
      res.obj = make_shared<TriangleNormal>(*this); // How to merge this?
      return res;
    }
    return {};
  }
  vec3 getNormal(const vec3 &p) const override {
    vec3 bary = Barycentric(a, b, c, p);
    return unit_vector(bary.x() * na + bary.y() * nb + bary.z() * nc);
  }
};
