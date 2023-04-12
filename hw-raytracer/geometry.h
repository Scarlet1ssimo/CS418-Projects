#pragma once
#include "vec.h"
#include <bits/stdc++.h>
#include <memory>
#include <optional>
using namespace std;
const double hit_eps = 1e-6;
struct Ray {
  vec3 A, B;

public:
  Ray() {}
  Ray(const vec3 &a, const vec3 &b) : A(a), B(b) {}
  vec3 origin() const { return A; }
  vec3 direction() const { return B; }
  vec3 point_at_parameter(double t) const { return A + B * t; }
};
struct Intersectable;
struct Leaf;
struct Material {
  vec3 shininess;
  vec3 transparency;
  double ior;
  double roughness;
  vec3 st;
  Material(const vec3 &shininess, const vec3 &transparency, double ior,
           double roughness, const vec3 &st)
      : shininess(shininess), transparency(transparency), ior(ior),
        roughness(roughness), st(st) {}
};
struct HitRecord {
  double t;
  shared_ptr<Leaf> obj;
};
struct Intersectable {
  virtual optional<HitRecord> intersect(const Ray &r) const = 0;
};
struct Leaf : Intersectable {
  vec3 color;
  shared_ptr<Material> material;
  virtual vec3 getNormal(const vec3 &p) const = 0;
  virtual vec3 getColor(const vec3 &p) const = 0;
};
// struct BVH : Intersectable {};
struct Sphere : Leaf {
  vec3 center;
  double radius;
  Sphere() = delete;
  Sphere(const vec3 &c, double r) : center(c), radius(r) {}
  optional<HitRecord> intersect(const Ray &r) const override {
    vec3 oc = r.origin() - center;
    double a = dot(r.direction(), r.direction());
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
  vec3 getNormal(const vec3 &p) const override {
    return unit_vector(p - center);
  }
  vec3 getColor(const vec3 &) const override { return color; }
};
struct Plane : Leaf {
  vec3 normal;
  double d;
  Plane() = delete;
  Plane(const vec3 &n, double d) : normal(n), d(d) {}
  optional<HitRecord> intersect(const Ray &r) const override {
    double denom = dot(normal, r.direction());
    if (abs(denom) > hit_eps) {
      double t = -(dot(normal, r.origin()) + d) / denom;
      if (t > hit_eps) {
        HitRecord res;
        res.t = t;
        res.obj = make_shared<Plane>(*this);
        return res;
      }
    }
    return {};
  }
  vec3 getNormal(const vec3 &) const override { return unit_vector(normal); }
  vec3 getColor(const vec3 &) const override { return color; }
};
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
  vec3 getDir(vec3) override { return lightDir; }
  double getT(vec3) override { return 1e300; }
  vec3 getColor(vec3) override { return color; }
};
struct Bulb : Light {
  vec3 coord;
  Bulb() = delete;
  Bulb(const vec3 &p) : coord(p) {}
  vec3 getDir(vec3 p) override { return unit_vector(coord - p); }
  double getT(vec3 p) override { return (coord - p).length(); }
  vec3 getColor(vec3 p) override {
    return color / (coord - p).squared_length();
  }
};
struct World {
  vector<shared_ptr<Intersectable>> objs;
  vector<shared_ptr<Light>> lights;
  // vector<shared_ptr<Material>> materials;
  void add(shared_ptr<Intersectable> obj) { objs.push_back(obj); }
  void add(shared_ptr<Light> light) { lights.push_back(light); }
  // void add(shared_ptr<Material> material) { materials.push_back(material); }
  optional<HitRecord> intersect(const Ray &r) const {
    optional<HitRecord> res;
    for (auto &obj : objs) {
      auto tmp = obj->intersect(r);
      if (tmp.has_value()) {
        if (!res.has_value()) {
          res = tmp;
          continue;
        }
        if (tmp.value().t < res.value().t) {
          res = tmp;
        }
      }
    }
    return res;
  }
  optional<vec3> getColor(const Ray &r, int bounces) const {
    if (bounces == 0)
      return {};
    auto rec = intersect(r);
    if (rec.has_value()) {
      vec3 p = r.point_at_parameter(rec.value().t); // intersection
      vec3 normal = rec.value().obj->getNormal(p);
      if (dot(normal, r.direction()) > 0)
        // if the normal points away from the eye, invert normal
        normal = -normal;

      vec3 shininess = rec.value().obj->material->shininess;
      vec3 transparency = rec.value().obj->material->transparency;

      vec3 specularColor = vec3(0, 0, 0);
      if (shininess.squared_length() > 0)
        specularColor =
            getColor(Ray(p, reflect(r.direction(), normal)), bounces - 1)
                .value_or(vec3(0, 0, 0));

      vec3 refractiveColor = vec3(0, 0, 0);
      if (transparency.squared_length() > 0)
        refractiveColor =
            getColor(
                Ray(p, refract_or_total_reflect(r.direction(), normal, 1.0)),
                bounces - 1)
                .value_or(vec3(0, 0, 0));

      vec3 objColor = rec.value().obj->getColor(p);

      vec3 diffuseColor = vec3(0, 0, 0);
      for (auto &light : lights) {
        vec3 dir = light->getDir(p); // Get light to source
        Ray shadowRay(p, dir);       // Ray from p to light
        auto shadowRec = intersect(shadowRay);
        if (shadowRec.has_value() && shadowRec->t < light->getT(p))
          continue;

        auto intensity = dot(dir, normal);
        auto lightColor = objColor * light->getColor(p) * max(intensity, 0.0);
        if (lightColor.r() >= 1)
          cout << lightColor << endl;
        diffuseColor += lightColor;
      }
      // diffuseColor.mapeach([](double x) { return min(max(0.0, x), 1.0); });
      vec3 finalColor = specularColor * shininess +
                        refractiveColor * (1 - shininess) * transparency +
                        diffuseColor * (1 - shininess) * (1 - transparency);
      finalColor.mapeach([](double x) { return min(max(0.0, x), 1.0); });
      return finalColor;
    } else {
      return {};
    }
  }
};