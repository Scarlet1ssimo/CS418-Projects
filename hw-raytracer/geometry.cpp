#include "geometry.h"

bool TriangleSimple::intersect(const Ray &r, double &bestT) const {
  double t;
  vec3 e1 = b - a, e2 = c - a, s = r.origin() - a;
  vec3 s1 = cross(r.direction(), e2), s2 = cross(s, e1);
  if (abs(dot(s1, e1)) < hit_eps)
    return false;
  double coeff = 1.0 / dot(s1, e1);
  t = coeff * dot(s2, e2);
  double b1 = coeff * dot(s1, s), b2 = coeff * dot(s2, r.direction());
  if (t > hit_eps && b1 > -hit_eps && b2 > -hit_eps && b1 + b2 < 1 + hit_eps)
    if (t < bestT) {
      bestT = t;
      return true;
    }
  return false;
}

vec3 TriangleText::getColor(const vec3 &p) const {
  if (material->tex && material->tex->valid) {
    vec3 bary = Barycentric(a, b, c, p);
    vec3 mix = sta * bary.x() + stb * bary.y() + stc * bary.z();
    double s = mix.x(), t = mix.y();
    return material->tex->getColor(s, t);
  } else
    return color;
}

bool Plane::intersect(const Ray &r, double &bestT) const {
  double denom = dot(normal, r.direction());
  double t;
  if (abs(denom) > hit_eps) {
    t = -(dot(normal, r.origin()) + d) / denom;
    if (t > hit_eps && t < bestT) {
      bestT = t;
      return true;
    }
  }
  return false;
}

bool Sphere::intersect(const Ray &r, double &bestT) const {
  double t;
  vec3 oc = r.origin() - center;
  double a = r.direction().squared_length();
  double b = dot(oc, r.direction());
  double c = dot(oc, oc) - radius * radius;
  double discriminant = b * b - a * c;
  if (discriminant < 0) {
    return false;
  } else {
    t = (-b - sqrt(discriminant)) / a;
    if (t > hit_eps && t < bestT) {
      bestT = t;
      return true;
    }
    t = (-b + sqrt(discriminant)) / a;
    if (t > hit_eps && t < bestT) {
      bestT = t;
      return true;
    }
    return false;
  }
}

bool AABB::intersect(const Ray &r, double &bestT) const {
  double txmin = (a - r.origin().x()) / r.direction().x();
  double txmax = (A - r.origin().x()) / r.direction().x();
  if (txmin > txmax)
    swap(txmin, txmax);
  if (txmin > bestT)
    return false;
  if (txmax > bestT)
    txmax = bestT;
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
  if (txmin > hit_eps && txmin < bestT) {
    bestT = txmin;
    return true;
  } else if (txmax > hit_eps && txmax < bestT) {
    bestT = txmax;
    return true;
  }
  return false;
}
vec3 AABB::getNormal(const vec3 &p) const {
  vec3 n;
  if (abs(p.x() - a) < hit_eps)
    return vec3(-1, 0, 0);
  else if (abs(p.x() - A) < hit_eps)
    return vec3(1, 0, 0);
  else if (abs(p.y() - b) < hit_eps)
    return vec3(0, -1, 0);
  else if (abs(p.y() - B) < hit_eps)
    return vec3(0, 1, 0);
  else if (abs(p.z() - c) < hit_eps)
    return vec3(0, 0, -1);
  else if (abs(p.z() - C) < hit_eps)
    return vec3(0, 0, 1);
  assert(false);
}