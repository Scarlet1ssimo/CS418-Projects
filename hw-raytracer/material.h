#pragma once
#define cimg_use_png
#define cimg_display 0

#include "vec.h"
#include <bits/stdc++.h>
struct Texture {
  int h, w;
  bool valid;
  vector<vec3> data;
  Texture() = delete;
  Texture(const string &s);
  vec3 getColor(double s, double t) {
    if (!valid)
      return vec3(0, 0, 0);
    s = s - floor(s);
    t = t - floor(t);
    int x = s * (w - 1), y = t * (h - 1);
    return data[y * w + x];
  }
};
struct Material {
  vec3 shininess;
  vec3 transparency;
  double ior;
  double roughness;
  shared_ptr<Texture> tex;
  Material(const vec3 &shininess, const vec3 &transparency, double ior,
           double roughness, shared_ptr<Texture> tex)
      : shininess(shininess), transparency(transparency), ior(ior),
        roughness(roughness), tex(tex) {}
};

struct MatrialFactory {
  bool changed = true;
  vec3 Shininess;
  vec3 Transparency;
  double Ior;
  double Roughness;
  shared_ptr<Texture> Tex;

  MatrialFactory()
      : Shininess(vec3(0, 0, 0)), Transparency(vec3(0, 0, 0)), Ior(1.458),
        Roughness(0), Tex(nullptr) {}
  void setShininess(vec3 x) {
    changed = true;
    Shininess = x;
  }
  void setShininess(double x) {
    changed = true;
    Shininess = vec3(x, x, x);
  }
  void setTransparency(vec3 x) {
    changed = true;
    Transparency = x;
  }
  void setTransparency(double x) {
    changed = true;
    Transparency = vec3(x, x, x);
  }
  void setIor(double x) {
    changed = true;
    Ior = x;
  }
  void setRoughness(double x) {
    changed = true;
    Roughness = x;
  }
  void setTexture(const string &s) {
    changed = true;
    Tex = make_shared<Texture>(s);
  }

  shared_ptr<Material> curMaterial() {
    static shared_ptr<Material> lastMaterial = nullptr;
    if (changed) {
      lastMaterial =
          make_shared<Material>(Shininess, Transparency, Ior, Roughness, Tex);
      changed = false;
    }
    return lastMaterial;
  }
};
