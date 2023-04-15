#include "geometry.h"
#include "grouping.h"
#include "printer.h"
#include "utility.h"
#include "vec.h"
#include <bits/stdc++.h>
#include <fstream>
#include <memory>
using namespace std;

World world;

vec3 curColor = vec3(1, 1, 1);
vec3 Eye(0, 0, 0);
vec3 Forward(0, 0, -1);
vec3 Right(1, 0, 0);
vec3 Up(0, 1, 0);
void calibration() {
  Right = unit_vector(cross(Forward, Up));
  Up = unit_vector(cross(Right, Forward));
}

MatrialFactory MF;
void makeObj(shared_ptr<Leaf> objptr) {
  objptr->color = curColor;
  objptr->material = MF.curMaterial();
  world.add(objptr);
}
void makeLight(shared_ptr<Light> lightptr) {
  lightptr->color = curColor;
  world.add(lightptr);
}
void showBVH(shared_ptr<BVH> bvh, int d = 0) {
  if (d > 1)
    return;
  makeObj(make_shared<AABB>(vec3(bvh->a, bvh->b, bvh->c),
                            vec3(bvh->A, bvh->B, bvh->C)));
  for (auto &p : bvh->Gchild) {
    if (auto pp = dynamic_pointer_cast<BVH>(p)) {
      showBVH(pp, d + 1);
    }
  }
}

int main(int argc, char *argv[]) {
  // feenableexcept(FE_INVALID | FE_OVERFLOW);
  int width, height;
  string filename;
  (void)argc;
  vector<vec3> xyzs;
  vector<vec3> normals;
  vector<vec3> texcoords;
  vec3 curNormal;
  double S, T;
  int aaRays = 0, gid = 0;
  bool doNormal = false;
  bool doTexcoord = false;
  bool fisheye = false;
  bool panorama = false;
  bool dof = false;
  double focus, lens;
  int bounces = 4;

  // first pass: read scene
  ifstream fin(argv[1]);
  optional<double> expose_v;
  for (string s; getline(fin, s);) {
    trim(s);
    remove_extra_space(s);
    auto ss = split(s, " ");
    auto keyword = ss[0];
    if (keyword == "png") { // png width height filename
      width = stoi(ss[1]);
      height = stoi(ss[2]);
      filename = ss[3];
    } else if (keyword == "xyz") {
      xyzs.push_back(vec3(stod(ss[1]), stod(ss[2]), stod(ss[3])));
      if (doNormal)
        normals.push_back(curNormal);
      if (doTexcoord)
        texcoords.push_back(vec3(S, T, 0));
    } else if (keyword == "normal") {
      curNormal = vec3(stod(ss[1]), stod(ss[2]), stod(ss[3]));
      doNormal = true;
    } else if (keyword == "texcoord") {
      S = stod(ss[1]);
      T = stod(ss[2]);
      doTexcoord = true;
    } else if (keyword == "trit") {
      int idx1 = stoi(ss[1]), idx2 = stoi(ss[2]), idx3 = stoi(ss[3]);
      idx1 = idx1 < 0 ? xyzs.size() + idx1 : idx1 - 1;
      idx2 = idx2 < 0 ? xyzs.size() + idx2 : idx2 - 1;
      idx3 = idx3 < 0 ? xyzs.size() + idx3 : idx3 - 1;
      makeObj(make_shared<TriangleText>(xyzs[idx1], xyzs[idx2], xyzs[idx3],
                                        texcoords[idx1], texcoords[idx2],
                                        texcoords[idx3]));
    } else if (keyword == "trif") {
      int idx1 = stoi(ss[1]), idx2 = stoi(ss[2]), idx3 = stoi(ss[3]);
      idx1 = idx1 < 0 ? xyzs.size() + idx1 : idx1 - 1;
      idx2 = idx2 < 0 ? xyzs.size() + idx2 : idx2 - 1;
      idx3 = idx3 < 0 ? xyzs.size() + idx3 : idx3 - 1;
      if (doNormal) {
        makeObj(make_shared<TriangleInterpNormal>(
            xyzs[idx1], xyzs[idx2], xyzs[idx3], normals[idx1], normals[idx2],
            normals[idx3]));
      } else
        makeObj(
            make_shared<TriangleSimple>(xyzs[idx1], xyzs[idx2], xyzs[idx3]));
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
    } else if (keyword == "texture") {
      MF.setTexture(ss[1]);
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
    } else if (keyword == "fisheye") {
      fisheye = true;
    } else if (keyword == "panorama") {
      panorama = true;
    } else if (keyword == "aa") {
      aaRays = stoi(ss[1]);
    } else if (keyword == "gi") {
      gid = stoi(ss[1]);
    } else if (keyword == "dof") {
      dof = true;
      focus = stod(ss[1]);
      lens = stod(ss[2]);
    }
  }
  cout << "First Pass" << endl;
  world.bvh->selfBalance();
  cout << world.bvh->str() << endl;
  cout << world.objs.size() << " objects" << endl;
  cout << world.lights.size() << " lights" << endl;
  // second pass: render

  if (false) {
    panorama = false;
    dof = false;
    aaRays = 0;
    gid = 0;
    bounces = 10;
    MF.setIor(1);
    MF.setShininess(0);
    MF.setTransparency(0.9);
    curColor = vec3(1, 1, 1);
    ::showBVH(world.bvh);
    width /= 4;
    height /= 4;
  }
  Printer PT(width, height, expose_v);
  auto normalizedForward = unit_vector(Forward);
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      // cout << x << ' ' << y << endl;
      auto getColor = [&]() -> optional<vec3> {
        double xx = x, yy = y;
        if (aaRays > 0) {
          xx += random_uniform() - 0.5;
          yy += random_uniform() - 0.5;
        }
        auto sx = (2.0 * xx - width) / max(width, height);
        auto sy = (height - 2.0 * yy) / max(width, height);
        Ray ray;
        if (fisheye) {
          auto sx_ = sx / Forward.length();
          auto sy_ = sy / Forward.length();
          auto rsquare = sx_ * sx_ + sy_ * sy_;
          if (rsquare > 1)
            return {};
          ray = Ray(Eye,
                    sx_ * Right + sy_ * Up +
                        sqrt(1 - rsquare) * normalizedForward,
                    true);

        } else if (panorama) {
          auto theta = (-.5 + 2.0 * -xx / width) * M_PI;
          auto phi = (.5 - 1.0 * yy / height) * M_PI;
          ray = Ray(Eye, sin(phi) * Up +
                             cos(phi) * sin(theta) * normalizedForward +
                             cos(phi) * cos(theta) * Right);
        } else {
          ray = Ray(Eye, sx * Right + sy * Up + Forward, true);
        }
        if (dof) {
          auto interscet = ray.origin() + ray.direction() * focus;
          vec3 random_move = random_in_unit_disk();
          auto neworigin =
              ray.origin() +
              (random_move.x() * Right + random_move.y() * Up) * lens;
          ray = Ray(neworigin, interscet - neworigin, true);
        }
        return world.getColor(ray, bounces, gid);
      };
      optional<vec3> color;
      double a;
      if (aaRays == 0) {
        color = getColor();
        a = color ? 1 : 0;
      } else {
        vec3 sum(0, 0, 0);
        double suma = 0;
        for (int i = 0; i < aaRays; i++) {
          auto c = getColor();
          if (c.has_value()) {
            suma += 1;
            sum += c.value() * 1;
          }
        }
        if (suma == 0)
          color = {};
        else
          color = sum / suma;
        a = suma / aaRays;
      }
      if (color.has_value()) {
        PT.put(x, y, color.value(), a);
      } else {
        PT.put(x, y);
      }
    }
    if (width > 500 && width > 500)
      cout << x << "/" << width << endl;
  }
  PT.save(filename);
}