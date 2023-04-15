#include "printer.h"
#include "vec.h"
inline vec3 rgb2srgb(vec3 rgb, optional<double> expose_v) {
  if (expose_v.has_value()) {
    auto qwq = [&](double x) { return 1 - exp(-x * expose_v.value()); };
    for (int i = 0; i < 3; i++)
      rgb[i] = qwq(rgb[i]);
  }
  return mapeach(rgb, [](double x) {
    return x <= 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
  });
}
Printer::Printer(int width, int height, optional<double> expose_v)
    : image(width, height, 1, 4), expose_v(expose_v) {}
void Printer::put(int x, int y, vec3 color, double a) {
  auto srgb = rgb2srgb(color, expose_v);
  srgb.mapeach([](double x) { return min(max(0.0, x), 1.0); });
  image(x, y, 0, 0) = srgb.r() * 255;
  image(x, y, 0, 1) = srgb.g() * 255;
  image(x, y, 0, 2) = srgb.b() * 255;
  image(x, y, 0, 3) = a * 255;
}
void Printer::put(int x, int y) {
  image(x, y, 0, 0) = 0;
  image(x, y, 0, 1) = 0;
  image(x, y, 0, 2) = 0;
  image(x, y, 0, 3) = 0;
}
void Printer::save(const string &filename) { image.save_png(filename.c_str()); }