import sys
from PIL import Image
from color import srgb_to_rgb, rgb_to_srgb, blendAlpha
import numpy as np

curRGB = [255, 255, 255, 1]
curST = [0, 0]
points = []  # x,y,z,w,r,g,b,a,s,t
clipplanes = []  # p1 p2 p3 p4 _ _ _ _ _
depthEnabled = False
sRGBEnabled = False
hypEnabled = False
cullEnabled = False
decalsEnabled = False
fsaaLevel = 1
channels = 4  # r,g,b,a
np.set_printoptions(precision=2)
texdata = None


def build_bitmap():
    global bitmap
    global depth
    bitmap = np.zeros((width, height, channels), dtype=float)
    if depthEnabled:
        depth = np.ones((width, height), dtype=float)


def callback_png(keywords):
    global image
    global filename
    global width
    global height
    global bitmap
    width = int(keywords[1])
    height = int(keywords[2])
    filename = keywords[3]
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    build_bitmap()


def callback_depth(keywords):
    global depth
    global depthEnabled
    depth = np.ones((width, height), dtype=float)
    depthEnabled = True


def callback_sRGB(keywords):
    global sRGBEnabled
    sRGBEnabled = True


def callback_hyp(keywords):
    global hypEnabled
    hypEnabled = True


def callback_cull(keywords):
    global cullEnabled
    cullEnabled = True


def callback_decals(keywords):
    global decalsEnabled
    decalsEnabled = True


def callback_frustum(keywords):
    global clipplanes
    clipplanes += [np.array([1, 0, 0, 1, 0, 0, 0, 0, 0, 0]),
                   np.array([-1, 0, 0, 1, 0, 0, 0, 0, 0, 0]),
                   np.array([0, 1, 0, 1, 0, 0, 0, 0, 0, 0]),
                   np.array([0, -1, 0, 1, 0, 0, 0, 0, 0, 0]),
                   np.array([0, 0, 1, 1, 0, 0, 0, 0, 0, 0]),
                   np.array([0, 0, -1, 1, 0, 0, 0, 0, 0, 0]),
                   ]


def callback_clipplane(keywords):
    P = np.array(
        list(map(float, keywords[1:]))+[0, 0, 0, 0, 0, 0], dtype=float)
    clipplanes.append(P)


def callback_fsaa(keywords):
    global fsaaLevel
    global width
    global height

    fsaaLevel = int(keywords[1])
    width *= fsaaLevel
    height *= fsaaLevel
    build_bitmap()


def callback_xyzw(keywords):
    RGB_eff = tuple(curRGB)
    if sRGBEnabled:
        RGB_eff = srgb_to_rgb(RGB_eff)
    P = np.array(
        list(map(float, keywords[1:]))+list(RGB_eff)+list(curST), dtype=float)
    if not clipplanes:
        my_shader(P)
    points.append(P)


def my_shader(p, reverse=False):
    # viewport
    w = p[3]
    if reverse:
        p[0] = (p[0]/(width/2)-1)*w
        p[1] = (p[1]/(height/2)-1)*w
    else:
        p[0] = (p[0]/w+1)*width/2
        p[1] = (p[1]/w+1)*height/2


def callback_texcoord(keywords):
    global curST
    curST = list(map(float, keywords[1:]))


def callback_texture(keywords):
    global texturefile
    global texdata
    texturefile = keywords[1]
    texdata = None


def callback_rgb(keywords):
    global curRGB
    curRGB = list(map(float, keywords[1:]))+[1]


def callback_rgba(keywords):
    global curRGB
    curRGB = list(map(float, keywords[1:]))


def tri_common(keywords, text=False):
    i, j, k = map(lambda s: int(s) if int(s) < 0 else int(s)-1, keywords[1:])
    if cullEnabled and np.cross(points[i][:3]-points[j][:3], points[j][:3]-points[k][:3])[2] > 0:
        return
    rasterizeTri(points[i], points[j], points[k], text)


def callback_tri(keywords):
    tri_common(keywords)


def callback_trit(keywords):
    tri_common(keywords, True)


def point_common(keywords, text=False):
    ps = float(keywords[1])*fsaaLevel
    i = int(keywords[2])
    tl = np.copy(points[i])
    tr = np.copy(points[i])
    bl = np.copy(points[i])
    br = np.copy(points[i])
    tl[0:2] += [-ps/2, -ps/2]
    tr[0:2] += [-ps/2, ps/2]
    bl[0:2] += [ps/2, -ps/2]
    br[0:2] += [ps/2, ps/2]
    tl[8:10] = [0, 0]
    tr[8:10] = [0, 1]
    bl[8:10] = [1, 0]
    br[8:10] = [1, 1]
    rasterizeTri(tl, tr, br, text)
    rasterizeTri(br, bl, tl, text)


def callback_point(keywords):
    point_common(keywords)


def callback_billboard(keywords):
    point_common(keywords, True)


#
# Utility functions -- rasterization and interpolation
#

def interp(a, b, p, d):
    if hypEnabled:
        A = divideWhyp(a)
        B = divideWhyp(b)
        P = divideWhyp(p)
        return divideWhyp(((B[d]-P[d])*A+(P[d]-A[d])*B)/(B[d]-A[d]))
    else:
        return np.copy(p)


def DDA(a, b, d):
    S = []

    if a[d] == b[d]:
        return S
    if a[d] > b[d]:
        a, b = b, a
    s = divide(b-a, d)
    e = np.ceil(a[d])-a[d]
    p = a+e*s
    while p[d] < b[d]:
        S.append(interp(a, b, p, d))
        p += s
    return S


def rasterizeTriRaw(p1, p2, p3, text):
    # x:0 y:1 z:2 w:3
    if p1[1] > p2[1]:
        p1, p2 = p2, p1
    if p1[1] > p3[1]:
        p1, p3 = p3, p1
    if p2[1] > p3[1]:
        p2, p3 = p3, p2
    #   p1
    #      p2
    # p3
    SLong = DDA(p1, p3, 1)
    SComb = DDA(p1, p2, 1)+DDA(p2, p3, 1)
    assert len(SLong) == len(SComb)
    for e1, e2 in zip(SLong, SComb):
        # - inside
        # - on the left
        # - on perfect top
        S = DDA(e1, e2, 0)
        for raster in S:
            putpixel(text, *raster)


def trydouble(p1, p2, p3, cp):
    def intersect(p, q, dp, dq):
        e = (dq*p-dp*q)/(dq-dp)
        return interp(p, q, e, 2)
    d1, d2, d3 = np.dot(cp, p1), np.dot(cp, p2), np.dot(cp, p3)
    if d1 >= 0 and d2 >= 0 and d3 >= 0:
        return [(p1, p2, p3)]
    if d1*d2 < 0:
        e = intersect(p1, p2, d1, d2)
        return [(p1, e, p3), (p2, e, p3)]
    if d3*d2 < 0:
        e = intersect(p3, p2, d3, d2)
        return [(p3, e, p1), (p2, e, p1)]
    if d1*d3 < 0:
        e = intersect(p1, p3, d1, d3)
        return [(p1, e, p2), (p3, e, p2)]
    return []


def discard(p1, p2, p3, cp):
    d1, d2, d3 = np.dot(cp, p1), np.dot(cp, p2), np.dot(cp, p3)
    if d1 >= 0 and d2 >= 0 and d3 >= 0:
        return True


def clipone(p1, p2, p3, cp):
    S = trydouble(p1, p2, p3, cp)
    nS = []
    for i in S:
        nS += trydouble(*i, cp)
    nS = filter(lambda tri: discard(*tri, cp), nS)
    return nS


def rasterizeTri(p1, p2, p3, text):
    def clipping(S, cp):
        nS = []
        for i in S:
            nS += clipone(*i, cp)
        return nS

    if clipplanes:
        S = [(p1, p2, p3)]
        for cp in clipplanes:
            S = clipping(S, cp)
        for tri in S:
            p1, p2, p3 = tri
            p1 = p1.copy()
            p2 = p2.copy()
            p3 = p3.copy()
            my_shader(p1)
            my_shader(p2)
            my_shader(p3)
            rasterizeTriRaw(p1, p2, p3, text)
    else:
        rasterizeTriRaw(p1, p2, p3, text)


def blend_forXY(X, Y, rs, gs, bs, a_s):
    bitmap[X, Y] = blendAlpha(bitmap[X, Y], [rs, gs, bs, a_s])


def get_texel(s, t):
    global texdata
    global texheight
    global texwidth
    if texdata is None:
        texture = Image.open(texturefile)
        texwidth, texheight = texture.size
        texdata = texture.getdata()
    s, t = s-np.floor(s), t-np.floor(t)
    S, T = round(s*texwidth) % texwidth, round(t*texheight) % texheight
    r, g, b, a = texdata[S+T*texwidth]
    a /= 255
    # if sRGBEnabled:
    #     r, g, b, a = srgb_to_rgb((r, g, b, a))
    return r, g, b, a


def putpixel(text, x, y, z, w, r, g, b, a, s, t):
    X, Y = round(x), round(y)
    if text:
        if decalsEnabled:
            r, g, b, a = blendAlpha([r, g, b, a], get_texel(s, t))
        else:
            r, g, b, a = get_texel(s, t)
    if 0 <= X < width and 0 <= Y < height:
        if depthEnabled:
            if -1 <= z/w <= depth[X, Y]:
                blend_forXY(X, Y, r, g, b, a)
                depth[X, Y] = z/w
        else:
            blend_forXY(X, Y, r, g, b, a)


def draw():
    for i in range(0, width//fsaaLevel):
        for j in range(0, height//fsaaLevel):
            X = i*fsaaLevel
            Y = j*fsaaLevel
            multisamples = bitmap[X:X+fsaaLevel,
                                  Y:Y+fsaaLevel]
            # print(multisamples.mean(axis=(0, 1)))
            # r, g, b, a = multisamples.mean(axis=(0, 1))
            sum_a = multisamples[:, :, 3].sum()
            mixed_color = np.zeros((3,), dtype=float)
            if sum_a == 0:
                mixed_color = np.zeros((3,), dtype=float)
            else:
                for ii in range(fsaaLevel):
                    for jj in range(fsaaLevel):
                        mixed_color += multisamples[ii,
                                                    jj, :3]*multisamples[ii, jj, 3]/sum_a
            r, g, b = mixed_color
            a = sum_a/fsaaLevel**2
            if sRGBEnabled:
                r, g, b, a = rgb_to_srgb((r, g, b, a))
            a *= 255
            finalcolor = tuple(
                map(round, (r, g, b, a)))

            image.putpixel((i, j), finalcolor)

#
# Utility functions -- Transformation
#


def divide(p, d):
    return p/p[d]


def divideWhyp(p):
    w = p[3]
    new = np.copy(p)
    new[2:] /= w
    new[3] = 1/w
    return new


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
