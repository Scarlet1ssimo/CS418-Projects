import sys
from PIL import Image
import numpy as np
import numpy.typing as npt
from color import rgb1_to_srgb255
from typing import List, Tuple, Optional
from geometry import *

np.set_printoptions(precision=2)
eye = np.ones(3)
forward = np.array([0, 0, -1])
right = np.array([1, 0, 0])
up = np.array([0, 1, 0])
curColor = np.array([0, 0, 0])
vec3Type = type(curColor)


class Scene:
    geometry: List[BVH] = []
    # direction and Color
    lights: List[Light] = []

    def addObj(self, obj):
        self.geometry.append(obj)

    def addLight(self, light):
        self.lights.append(light)

    def hit(self, ray: Ray):
        bestT = 1e9
        firstObj = None
        for i in self.geometry:
            t, leaf = i.intersect(ray)
            if t is not None and t < bestT:
                bestT = t
                firstObj = leaf
        if firstObj is None:
            return None, None
        return bestT, firstObj

    def getColor(self, ray: Ray, d):
        t, obj = self.hit(ray)
        if obj is None:
            return np.array([0, 0, 0, 0])
        intersection = ray.getT(t)
        sumColor = np.array([0, 0, 0, 0])
        objColor = obj.getColor(ray, d-1)
        # reflectionColor
        # transparentColor
        for i in self.lights:
            lightDir = i.getDir(intersection)
            lightColor = i.getColor()
            sumColor += objColor * lightColor * \
                np.dot(lightDir, obj.getNormal(intersection))
        return sumColor


scene = Scene()


def callback_png(keywords):
    global image
    global filename
    global width
    global height
    width = int(keywords[1])
    height = int(keywords[2])
    filename = keywords[3]
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))


def callback_sphere(keywords):
    scene.addObj(
        Sphere(np.array(map(float, keywords[1:4])), float(keywords[4])))


def callback_sun(keywords):
    scene.addLight(Sun(np.array(map(float, keywords[1:4])), curColor))


def callback_color(keywords):
    global curColor
    curColor = np.array(map(float, keywords[1:]))


def draw():
    for x in range(width):
        for y in range(height):
            sx = (2*x-width) / max(width, height)
            sy = (height-2*y) / max(width, height)
            ray = sx * right + sy * up + forward
            ray = Ray(eye, ray / np.linalg.norm(ray))
            color = scene.getColor(ray, 4)
            image.putpixel((x, y), rgb1_to_srgb255(color))


if __name__ == "__main__":
    inputFileName = sys.argv[1]
    with open(inputFileName, "r", encoding="ascii") as f:
        for ip in f.readlines():
            keywords = ip.strip().split()
            if not keywords:
                continue
            callback_name = "callback_"+keywords[0]
            if callback_name in locals():
                func = locals()[callback_name]
                func(keywords)
        draw()
        image.save(filename)
