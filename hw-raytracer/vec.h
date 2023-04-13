#pragma once
#include <bits/stdc++.h>
#include <cmath>
#include <iostream>
#include <optional>

// #include "random.h"
using namespace std;
struct vec3 {
  double e[3];

  vec3() {}
  vec3(double e0, double e1, double e2) { e[0] = e0, e[1] = e1, e[2] = e2; }
  inline double x() const { return e[0]; }
  inline double y() const { return e[1]; }
  inline double z() const { return e[2]; }
  inline double r() const { return e[0]; }
  inline double g() const { return e[1]; }
  inline double b() const { return e[2]; }
  inline double s() const { return e[0]; }
  inline double t() const { return e[1]; }

  inline const vec3 &operator+() const { return *this; }
  inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  inline double operator[](int i) const { return e[i]; }
  inline double &operator[](int i) { return e[i]; }

  inline vec3 &operator+=(const vec3 &v2) {
    e[0] += v2[0];
    e[1] += v2[1];
    e[2] += v2[2];
    return *this;
  }
  inline vec3 &operator-=(const vec3 &v2) {
    e[0] -= v2[0];
    e[1] -= v2[1];
    e[2] -= v2[2];
    return *this;
  }
  inline vec3 &operator*=(const vec3 &v2) {
    e[0] *= v2[0];
    e[1] *= v2[1];
    e[2] *= v2[2];
    return *this;
  }
  inline vec3 &operator/=(const vec3 &v2) {
    e[0] /= v2[0];
    e[1] /= v2[1];
    e[2] /= v2[2];
    return *this;
  }
  inline vec3 &operator*=(const double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }
  inline vec3 &operator/=(const double t) {
    e[0] /= t;
    e[1] /= t;
    e[2] /= t;
    return *this;
  }

  inline double length() const {
    return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
  }
  inline double squared_length() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }
  inline void make_unit_vector() {
    double k = sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
    e[0] /= k;
    e[1] /= k;
    e[2] /= k;
  }
  inline void mapeach(double (*f)(double)) {
    e[0] = f(e[0]);
    e[1] = f(e[1]);
    e[2] = f(e[2]);
  }
};
inline istream &operator>>(istream &is, vec3 &t) {
  is >> t[0] >> t[1] >> t[2];
  return is;
}
inline ostream &operator<<(ostream &ou, const vec3 &t) {
  ou << t[0] << ' ' << t[1] << ' ' << t[2];
  return ou;
}
inline vec3 operator+(const vec3 &v1, const vec3 &v2) {
  return vec3{v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
}
inline vec3 operator-(const vec3 &v1, const vec3 &v2) {
  return vec3{v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}
inline vec3 operator-(const double &v1, const vec3 &v2) {
  return vec3{v1 - v2[0], v1 - v2[1], v1 - v2[2]};
}
inline vec3 operator-(const vec3 &v1, const double &v2) {
  return vec3{v1[0] - v2, v1[1] - v2, v1[2] - v2};
}
inline vec3 operator+(const double &v1, const vec3 &v2) {
  return vec3{v1 + v2[0], v1 + v2[1], v1 + v2[2]};
}
inline vec3 operator+(const vec3 &v1, const double &v2) {
  return vec3{v1[0] + v2, v1[1] + v2, v1[2] + v2};
}
inline vec3 operator*(const vec3 &v1, const vec3 &v2) {
  return vec3{v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2]};
}
inline vec3 operator/(const vec3 &v1, const vec3 &v2) {
  return vec3{v1[0] / v2[0], v1[1] / v2[1], v1[2] / v2[2]};
}
inline vec3 operator*(const vec3 &v1, const double &t) {
  return vec3{v1[0] * t, v1[1] * t, v1[2] * t};
}
inline vec3 operator*(const double &t, const vec3 &v1) {
  return vec3{v1[0] * t, v1[1] * t, v1[2] * t};
}
inline vec3 operator/(const vec3 &v1, const double &t) {
  return vec3{v1[0] / t, v1[1] / t, v1[2] / t};
}
inline double dot(const vec3 &v1, const vec3 &v2) {
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}
inline vec3 cross(const vec3 &v1, const vec3 &v2) {
  return vec3{v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2],
              v1[0] * v2[1] - v1[1] * v2[0]};
}
inline vec3 unit_vector(const vec3 &v) { return v / v.length(); }

inline vec3 reflect(const vec3 &v, const vec3 &n) {
  return v - 2 * dot(v, n) * n;
}
inline vec3 refract_or_total_reflect(const vec3 &v, const vec3 &n,
                                     double ni_over_nt) {
  vec3 uv = v;
  double dt = dot(uv, n);
  double discriminant = 1 - ni_over_nt * ni_over_nt * (1 - dt * dt);
  if (discriminant > 0) {
    return ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
  }
  return reflect(v, n);
}
inline vec3 mapeach(const vec3 &v, double f(double)) {
  return vec3{f(v[0]), f(v[1]), f(v[2])};
}
inline vec3 random_gaussian(double deviation) {
  static std::random_device rd;
  static std::mt19937 e2(rd());
  std::normal_distribution<> dist(0, deviation);
  return vec3(dist(e2), dist(e2), dist(e2));
}
inline double random_uniform() {
  static std::random_device rd;
  static std::mt19937 e2(rd());
  std::uniform_real_distribution<> dist(0, 1);
  return dist(e2);
}
inline vec3 random_in_unit_sphere() {
  vec3 p;
  do {
    p = 2.0 * vec3(random_uniform(), random_uniform(), random_uniform()) - 1;
  } while (p.squared_length() >= 1);
  return p;
}
inline vec3 random_in_unit_disk() {
  vec3 p;
  do {
    p = 2.0 * vec3(random_uniform(), random_uniform(), 0) - vec3(1, 1, 0);
  } while (p.squared_length() >= 1);
  return p;
}
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