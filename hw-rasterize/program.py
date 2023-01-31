import sys
from PIL import Image
from color import srgb_to_rgb, rgb_to_srgb
import numpy as np

curRGB = [255, 255, 255, 1]
curST = [0, 0]
points = []  # x,y,z,w,r,g,b,s,t
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
    my_shader(P)
    points.append(P)


def my_shader(p):
    # viewport
    w = p[3]
    p[0] = (p[0]/w+1)*width/2
    p[1] = (p[1]/w+1)*height/2
    # p[3] = 1
    # p[4:] *= w


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
    rasterize(points[i], points[j], points[k], text)


def callback_tri(keywords):
    tri_common(keywords)


def callback_trit(keywords):
    tri_common(keywords, True)


def point_common(keywords, text=False):
    ps = float(keywords[1])*fsaaLevel
    i = int(keywords[2])
    tl = points[i]
    tr = points[i]
    bl = points[i]
    br = points[i]
    tl[0:2] += [-ps/2, -ps/2]
    tl[7:9] = [0, 0]
    tr[0:2] += [-ps/2, ps/2]
    tr[7:9] = [0, 1]
    bl[0:2] += [ps/2, -ps/2]
    bl[7:9] = [1, 0]
    br[0:2] += [ps/2, ps/2]
    br[7:9] += [1, 1]
    rasterize(tl, tr, br, text)
    rasterize(br, bl, tl, text)


def callback_point(keywords):
    point_common(keywords)


def callback_billboard(keywords):
    point_common(keywords, True)


#
# Utility functions -- rasterization and interpolation
#

def interp(a, b, p, d):
    if hypEnabled:
        # return np.copy(p)
        A = divideWhyp(a)
        B = divideWhyp(b)
        P = divideWhyp(p)
        # p[d]=(1-s)a+sb
        # s = (P[d]-A[d])/(B[d]-A[d])
        # ans = (1-s)*A+s*B
        return divideWhyp(((B[d]-P[d])*A+(P[d]-A[d])*B)/(B[d]-A[d]))
    else:
        return np.copy(p)


def DDA(a, b, d):
    S = []
    # if hypEnabled:

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


def rasterize(p1, p2, p3, text):
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


def blendAlpha(dst, src):
    rs, gs, bs, a_s = dst
    rd, gd, bd, a_d = src
    a_ = a_s+a_d*(1-a_s)

    def blendEq(s, d): return 0 if a_ == 0 else (a_s*s+(1-a_s)*a_d*d)/a_
    r_ = blendEq(rs, rd)
    g_ = blendEq(gs, gd)
    b_ = blendEq(bs, bd)
    # print((rs, gs, bs, a_s), (rd, gd, bd, a_d), (r_, g_, b_, a_))
    return [r_, g_, b_, a_]


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
    # print(s, t)
    s, t = s-np.floor(s), t-np.floor(t)
    S, T = round(s*texwidth) % texwidth, round(t*texheight) % texheight
    # print(s, t, S, T)
    # print(data[S*texheight+T])
    # print(S, T, texwidth, texheight, S*texheight+T)
    # return texdata[S*texheight+T]
    r, g, b, a = texdata[S+T*texwidth]
    a /= 255
    # if sRGBEnabled:
    #     r, g, b, a = srgb_to_rgb((r, g, b, a))
    return r, g, b, a


def putpixel(text, x, y, z, w, r, g, b, a, s, t):
    X, Y = round(x), round(y)
    if text:
        if decalsEnabled:
            r, g, b, a = blendAlpha(get_texel(s, t), [r, g, b, a])
        else:
            r, g, b, a = get_texel(s, t)
    if X < width and Y < height:
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


def divideW(p):
    return divide(p, 3)


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
