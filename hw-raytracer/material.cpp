#include "material.h"
#include "CImg.h"
static inline vec3 srgb2rgb(vec3 srgb) {
  return mapeach(srgb, [](double x) {
    return x <= 0.04045 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
  });
}
Texture::Texture(const string &s) {
  try {
    cimg_library::CImg<unsigned char> image(s.c_str());
    w = image.width();
    h = image.height();
    cerr << "texture " << s << ":" << w << 'x' << h << endl;
    data.resize(h * w);
    for (int i = 0; i < h; i++)
      for (int j = 0; j < w; j++)
        data[i * w + j] =
            srgb2rgb(vec3(image(j, i, 0, 0) / 255.0, image(j, i, 0, 1) / 255.0,
                          image(j, i, 0, 2) / 255.0));
    valid = true;
  } catch (cimg_library::CImgIOException) {
    valid = false;
  }
}
