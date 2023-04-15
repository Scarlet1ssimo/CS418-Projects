#include "grouping.h"
#include "geometry.h"
#include <memory>
bool BVH::fastIntersect(const Ray &r, const double &bestT) const {
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
  // cout << "txmin: " << txmin << " txmax: " << txmax << endl;
  return txmax >= hit_eps;
}
inline optional<HitRecord>
makeRecordHelper(const Ray &r, shared_ptr<FastGroup> g, double &bestT) {
  if (!g->fastIntersect(r, bestT)) {
    return {};
  }
  optional<HitRecord> res;
  for (auto &l : g->Lchild) {
    auto tmp = makeRecord(r, l, bestT);
    if (tmp) {
      res = tmp; // move
    }
  }
  for (auto &l : g->Gchild) {
    if (auto tmp = makeRecordHelper(r, l, bestT)) {
      res = tmp;
    }
  }
  return res;
}
inline optional<HitRecord> makeRecord(const Ray &r, shared_ptr<FastGroup> g,
                                      double &bestT) {
  return makeRecordHelper(r, g, bestT);
}
pair<double, int> DivideHelper(const vector<shared_ptr<BoundableLeaf>> &Boxes,
                               bool _2D) {
  int n = Boxes.size();
  vector<BVH> Left(n), Right(n);
  Left[0].extendBy(Boxes[0]);
  for (int i = 1; i < n; i++) {
    Left[i] = Left[i - 1];
    Left[i].extendBy(Boxes[i]);
  }
  Right[n - 1].extendBy(Boxes[n - 1]);
  for (int i = n - 2; i >= 0; i--) {
    Right[i] = Right[i + 1];
    Right[i].extendBy(Boxes[i]);
  }
  double best = INF;
  int bestidx = -1;
  for (int i = 0; i < n - 1; i++) {
    double tmp = _2D ? Left[i].ndmetric() + Right[i + 1].ndmetric()
                     : Left[i].metric() + Right[i + 1].metric();
    if (tmp < best) {
      best = tmp;
      bestidx = i;
    }
  }
  return {best, bestidx};
}
void ScarletBVH::PackLchild() {
  for (auto &l : Lchild) {
    auto Next = make_shared<ScarletBVH>();
    Next->add(l);
    Gchild.push_back(Next);
  }
  Lchild.clear();
}
void ScarletBVH::ReleaseSingleGchild() {
  for (auto it = Gchild.begin(); it != Gchild.end();) {
    if ((*it)->Lchild.size() == 1 && (*it)->Gchild.size() == 0) {
      Lchild.push_back((*it)->Lchild[0]);
      it = Gchild.erase(it);
    } else {
      ++it;
    }
  }
  for (auto it = Gchild.begin(); it != Gchild.end(); it++) {
    assert((*it)->Lchild.size() != 0 || (*it)->Gchild.size() != 0);
  }
}
const int leafSize = 4;
bool ScarletBVH::Amethyst(double successRate) {
  // Decompose LChild into GChild
  // After this function, Lchild.size() == 0
  bool _2D = false;
  if (metric() < hit_eps) {
    _2D = true;
  }
  double best = INF;
  int bestidx = -1;
  shared_ptr<ScarletBVH> L = make_shared<ScarletBVH>(),
                         R = make_shared<ScarletBVH>();
  for (int i = 0; i < 3; i++) {
    if (abs(this->operator[](i) - this->operator[](i + 3)) < hit_eps)
      continue;
    sort(Lchild.begin(), Lchild.end(),
         [i](const shared_ptr<BoundableLeaf> &a,
             const shared_ptr<BoundableLeaf> &b) {
           BVH A, B;
           A.extendBy(a);
           B.extendBy(b);
           return A[i] < B[i];
         });
    auto tmp = DivideHelper(Lchild, _2D);
    if (tmp.first < best) {
      best = tmp.first;
      bestidx = tmp.second;
      L = make_shared<ScarletBVH>();
      R = make_shared<ScarletBVH>();
      for (int i = 0; i <= bestidx; i++)
        L->add(Lchild[i]);
      for (int i = bestidx + 1; i < Lchild.size(); i++)
        R->add(Lchild[i]);
    }
  }

  if (_2D) {
    if (best >= successRate * ndmetric())
      return false;
    assert(L->ndmetric() + R->ndmetric() < ndmetric());
    assert(L->ndmetric() + R->ndmetric() == best);
  } else {
    if (best >= successRate * metric())
      return false;
    assert(L->metric() + R->metric() < metric());
    assert(L->metric() + R->metric() == best);
  }
  assert(best != INF);

  Lchild.clear();

  if (!L->Amethyst(successRate)) {
    Gchild.push_back(L);
  } else {
    assert(L->Lchild.size() == 0);
    for (auto &l : L->Gchild) {
      Gchild.push_back(l);
    }
  }
  if (!R->Amethyst(successRate)) {
    Gchild.push_back(R);
  } else {
    assert(R->Lchild.size() == 0);
    for (auto &l : R->Gchild) {
      Gchild.push_back(l);
    }
  }
  return true;
}
bool ScarletBVH::Scarlet() {
  if (Lchild.size() < leafSize)
    return true;
  double mymetric = metric();
  double ma = (a + A) / 2, mb = (b + B) / 2, mc = (c + C) / 2;
  vector<shared_ptr<BoundableLeaf>> leaves;
  vector<shared_ptr<ScarletBVH>> tmp(8);
  for (int i = 0; i < 8; i++) {
    tmp[i] = make_shared<ScarletBVH>();
  }
  swap(Lchild, leaves);
  for (auto &l : leaves) {
    auto B = BVH();
    B.add(l);
    if (B.metric() > mymetric / 8) {
      Lchild.push_back(l);
    } else {
      int i = 0, j = 0, k = 0;
      if (B.A >= ma)
        i = 1;
      if (B.B >= mb)
        j = 1;
      if (B.C >= mc)
        k = 1;
      int idx = i * 4 + j * 2 + k;
      tmp[idx]->add(l);
    }
  }
  int nonempty = 0;
  for (auto &bvh : tmp) {
    if (!bvh->Lchild.empty())
      nonempty++;
  }
  if (nonempty <= 1) {
    for (auto &bvh : tmp) {
      for (auto &l : bvh->Lchild) {
        Lchild.push_back(l);
      }
    }
  } else {
    for (auto &bvh : tmp) {
      if (bvh->Lchild.size() <= 1) {
        for (auto &l : bvh->Lchild) {
          Lchild.push_back(l);
        }
      } else {
        Gchild.push_back(bvh);
      }
    }
  }
  return true;
}
void ScarletBVH::selfBalance() { settleLayer(0.6); }
void ScarletBVH::settleLayer(double successRate) {
  assert(Gchild.size() == 0);
  if (!Amethyst(successRate)) {
    if (!Scarlet()) {
      assert(false);
    }
  } else {
    assert(Lchild.size() == 0);
    double sum = 0;
    bool _2D = abs(metric()) < hit_eps;
    for (auto &l : Gchild)
      sum += _2D ? l->ndmetric() : l->metric();
    cout << "Amethyst: " << sum / (_2D ? ndmetric() : metric()) << endl;
    ReleaseSingleGchild();
  }
  for (auto &l : Gchild) {
    if (auto ll = dynamic_pointer_cast<ScarletBVH>(l))
      ll->settleLayer(min(0.6, successRate * 1));
    else {
      selfBalance();
    }
  }
}
optional<HitRecord> World::intersect(const Ray &r) const {
  double bestT = INF;
  auto res = makeRecord(r, bvh, bestT);
  for (auto &obj : objs) {
    auto tmp = makeRecord(r, obj, bestT);
    if (tmp && (!res || tmp->t < res->t)) {
      res = tmp;
    }
  }
  return res;
}

World::World() : bvh(make_shared<ScarletBVH>()) {}
optional<vec3> World::getColor(const Ray &r, int bounces, int gid) const {
  auto rec = intersect(r);
  if (rec.has_value()) {
    vec3 p = r.point_at_parameter(rec.value().t); // intersection
    vec3 outwardNormal = rec.value().obj->getNormal(p);
    auto material = rec.value().obj->material;

    if (material->roughness > 0)
      outwardNormal =
          unit_vector(outwardNormal + random_gaussian(material->roughness));

    vec3 facingNormal =
        dot(outwardNormal, r.direction()) > 0 ? -outwardNormal : outwardNormal;

    vec3 shininess = material->shininess;
    vec3 transparency = material->transparency;
    double ior = material->ior;

    vec3 specularColor = vec3(0, 0, 0);
    if (shininess.squared_length() > 0 && bounces > 0)
      specularColor =
          getColor(Ray(p, reflect(r.direction(), facingNormal), true),
                   bounces - 1, gid)
              .value_or(vec3(0, 0, 0));

    vec3 refractiveColor = vec3(0, 0, 0);
    if (transparency.squared_length() > 0 && bounces > 0) {
      bool into_obj = dot(outwardNormal, facingNormal) > 0;
      double ni_over_nt = into_obj ? 1.0 / ior : ior;
      refractiveColor =
          getColor(Ray(p,
                       refract_or_total_reflect(r.direction(), facingNormal,
                                                ni_over_nt),
                       true),
                   bounces - 1, gid)
              .value_or(vec3(0, 0, 0));
    }

    vec3 objColor = rec.value().obj->getColor(p);

    vec3 diffuseColor = vec3(0, 0, 0);
    if (gid > 0) {
      vec3 dir = facingNormal + random_in_unit_sphere();
      Ray giRay(p, dir, true); // Ray from p to random dir
      auto giColor = getColor(giRay, bounces - 1, gid - 1);

      auto intensity = dot(dir, facingNormal);
      if (giColor)
        diffuseColor += objColor * giColor.value() * max(intensity, 0.0);
    }
    for (auto &light : lights) {
      vec3 dir = light->getDir(p); // Get light to source
      Ray shadowRay(p, dir);       // Ray from p to light
      auto shadowRec = intersect(shadowRay);
      if (shadowRec.has_value() && shadowRec->t < light->getT(p))
        continue;

      auto intensity = dot(dir, facingNormal);
      auto lightColor = objColor * light->getColor(p) * max(intensity, 0.0);
      diffuseColor += lightColor;
    }
    return specularColor * shininess +
           refractiveColor * (1 - shininess) * transparency +
           diffuseColor * (1 - shininess) * (1 - transparency);
  } else {
    return {};
  }
}
