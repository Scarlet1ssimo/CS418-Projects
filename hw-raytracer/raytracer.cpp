#include "vec.h"
#define cimg_use_png
#define cimg_display 0
#include "CImg.h"
#include "geometry.h"
#include <bits/stdc++.h>
#include <fstream>
using namespace std;
std::vector<std::string> split(std::string s, std::string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

World world;
optional<double> expose_v;
vec3 rgb2srgb(vec3 rgb) {
  if (expose_v.has_value())
    rgb = mapeach(rgb, [](double x) { return 1 - exp(-x * expose_v.value()); });
  return mapeach(rgb, [](double x) {
    return x <= 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
  });
}
struct MatrialFactory {
  bool changed = true;
  vec3 Shininess;
  vec3 Transparency;
  double Ior;
  double Roughness;
  vec3 ST;
  MatrialFactory()
      : Shininess(vec3(0, 0, 0)), Transparency(vec3(0, 0, 0)), Ior(1.458),
        Roughness(0), ST(vec3(0, 0, 0)) {}
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
  void setST(vec3 x) {
    changed = true;
    ST = x;
  }

  shared_ptr<Material> curMaterial() {
    static shared_ptr<Material> lastMaterial = nullptr;
    if (changed) {
      lastMaterial =
          make_shared<Material>(Shininess, Transparency, Ior, Roughness, ST);
      changed = false;
    }
    return lastMaterial;
  }
} MF;

vec3 curColor = vec3(1, 1, 1);
int bounces = 4;
vec3 Eye(0, 0, 0);
vec3 Forward(0, 0, -1);
vec3 Right(1, 0, 0);
vec3 Up(0, 1, 0);
void calibration() {
  Right = unit_vector(cross(Forward, Up));
  Up = unit_vector(cross(Right, Forward));
}
void makeObj(shared_ptr<Leaf> objptr) {
  objptr->color = curColor;
  objptr->material = MF.curMaterial();
  world.add(objptr);
}
void makeLight(shared_ptr<Light> lightptr) {
  lightptr->color = curColor;
  world.add(lightptr);
}
int main(int argc, char *argv[]) {
  int width, height;
  string filename;
  (void)argc;

  // first pass: read scene
  ifstream fin(argv[1]);
  for (string s; getline(fin, s);) {
    bool prev_is_space = true;
    s.erase(std::remove_if(s.begin(), s.end(),
                           [&prev_is_space](unsigned char curr) {
                             bool r = std::isspace(curr) && prev_is_space;
                             prev_is_space = std::isspace(curr);
                             return r;
                           }),
            s.end());
    auto ss = split(s, " ");
    auto keyword = ss[0];
    if (keyword == "png") { // png width height filename
      width = stoi(ss[1]);
      height = stoi(ss[2]);
      filename = ss[3];
    } else if (keyword == "sphere") { // sphere x y z r
      makeObj(make_shared<Sphere>(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3])),
                                  stod(ss[4])));
    } else if (keyword == "plane") { // plane A B C D
      makeObj(make_shared<Plane>(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3])),
                                 stod(ss[4])));
    } else if (keyword == "sun") { // sun x y z
      makeLight(make_shared<Sun>(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]))));
    } else if (keyword == "bulb") { // bulb x y z
      makeLight(make_shared<Bulb>(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]))));
    } else if (keyword == "color") { // color r g b
      curColor = vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]));
    } else if (keyword == "shininess") {
      if (ss.size() == 4)
        MF.setShininess(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3])));
      else if (ss.size() == 2)
        MF.setShininess(stod(ss[1]));
      else
        assert(0);
    } else if (keyword == "transparency") {
      if (ss.size() == 4)
        MF.setTransparency(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3])));
      else if (ss.size() == 2)
        MF.setTransparency(stod(ss[1]));
      else
        assert(0);
    } else if (keyword == "ior") {
      MF.setIor(stod(ss[1]));
    } else if (keyword == "roughness") {
      MF.setRoughness(stod(ss[1]));
    } else if (keyword == "bounces") {
      bounces = stoi(ss[1]);
    } else if (keyword == "eye") {
      Eye = vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]));
    } else if (keyword == "forward") {
      Forward = vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]));
      calibration();
    } else if (keyword == "up") {
      Up = vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]));
      calibration();
    } else if (keyword == "expose") {
      expose_v = stod(ss[1]);
    }
  }

  // second pass: render
  cimg_library::CImg<unsigned char> image(width, height, 1, 4);
  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++) {
      auto sx = (2.0 * x - width) / max(width, height);
      auto sy = (height - 2.0 * y) / max(width, height);
      auto ray = Ray(Eye, sx * Right + sy * Up + Forward);
      auto color = world.getColor(ray, bounces);
      if (color.has_value()) {
        auto srgb = rgb2srgb(color.value());
        image(x, y, 0, 0) = srgb.r() * 255;
        image(x, y, 0, 1) = srgb.g() * 255;
        image(x, y, 0, 2) = srgb.b() * 255;
        image(x, y, 0, 3) = 255;
      } else {
        image(x, y, 0, 0) = 0;
        image(x, y, 0, 1) = 0;
        image(x, y, 0, 2) = 0;
        image(x, y, 0, 3) = 0;
      }
    }
  image.save_png(filename.c_str());
}