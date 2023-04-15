#pragma once
#define cimg_use_png
#define cimg_display 0
#include "CImg.h"
#include "vec.h"
struct Printer {
  optional<double> expose_v;
  cimg_library::CImg<unsigned char> image;
  Printer(int width, int height, optional<double> expose_v);
  void put(int x, int y, vec3 color, double a);
  void put(int x, int y);
  void save(const string &filename);
};
