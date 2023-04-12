import numpy as np
from typing import List, Tuple, Optional
# import numpy.typing as npt
curColor = np.array([0, 0, 0])
cutST = np.array([0, 0])
vec3Type = type(curColor)
vec2Type = type(cutST)


class Ray:
    origin: vec3Type
    direction: vec3Type

    def __init__(self, origin, direction):
        self.origin = origin
        self.direction = direction

    def getT(self, t):
        return self.origin + t * self.direction


class Intersectable:
    def __init__(self):
        raise NotImplementedError

    def intersect(self, ray: Ray) -> (Tuple[Optional[float], Optional[Leaf]]):
        raise NotImplementedError


class Leaf(Intersectable):
    color: vec3Type
    shininess: vec3Type
    transparency: vec3Type
    ior: float
    roughness: float
    stCoord: vec2Type

    def __init__(self, color, shininess, transparency, ior, roughness, stCoord):
        self.color = color
        self.shininess = shininess
        self.transparency = transparency
        self.ior = ior
        self.roughness = roughness
        self.stCoord = stCoord

    def getNormal(self, point) -> (vec3Type):
        raise NotImplementedError

    def getColor(self, ray: Ray, d) -> (vec3Type):
        raise NotImplementedError


class BVH(Intersectable):
    def intersect(self, ray: Ray, d) -> (Tuple[Optional[float], Optional[Leaf]]):
        raise NotImplementedError


class Sphere(Leaf):
    center: vec3Type
    radius: float

    def __init__(self, center, radius, *args):
        super().__init__(*args)
        self.center = center
        self.radius = radius

    def getNormal(self, point) -> (vec3Type):
        return (point - self.center) / self.radius

    def getColor(self, ray: Ray, d) -> (Tuple[Optional[float], Optional[Leaf]]):
        raise NotImplementedError


class Light:
    color: vec3Type

    def getDir(self, point) -> (vec3Type):
        raise NotImplementedError

    def getColor(self) -> (vec3Type):
        return self.color


class Sun(Light):
    lightDir: vec3Type

    def __init__(self, lightDir, color):
        self.lightDir = lightDir
        self.color = color

    def getDir(self, point) -> (vec3Type):
        return self.lightDir


class Bulb(Light):
    coord: vec3Type

    def __init__(self, coord, color):
        self.coord = coord
        self.color = color

    def getDir(self, point) -> (vec3Type):
        direction = self.coord - point
        return direction / np.linalg.norm(direction)
