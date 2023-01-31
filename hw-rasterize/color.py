def convert_color_component(value, inverse=False):
    if inverse:
        if value <= 0.0031308:
            return value * 12.92
        else:
            return 1.055 * (value ** (1 / 2.4)) - 0.055
    else:
        if value <= 0.04045:
            return value / 12.92
        else:
            return ((value + 0.055) / 1.055) ** 2.4


def srgb_to_rgb(srgb_color):
    r, g, b, a = srgb_color
    r, g, b = convert_color_component(r / 255.0), convert_color_component(
        g / 255.0), convert_color_component(b / 255.0)
    return (r * 255, g * 255, b * 255, a)


def rgb_to_srgb(rgb_color):
    r, g, b, a = rgb_color
    r, g, b = convert_color_component(r / 255.0, True), convert_color_component(
        g / 255.0, True), convert_color_component(b / 255.0, True)
    return (r * 255, g * 255, b * 255, a)


def blendAlpha(dst, src):
    rs, gs, bs, a_s = src
    rd, gd, bd, a_d = dst
    a_ = a_s+a_d*(1-a_s)

    def blendEq(s, d): return 0 if a_ == 0 else (a_s*s+(1-a_s)*a_d*d)/a_
    r_ = blendEq(rs, rd)
    g_ = blendEq(gs, gd)
    b_ = blendEq(bs, bd)
    # print((rs, gs, bs, a_s), (rd, gd, bd, a_d), (r_, g_, b_, a_))
    return [r_, g_, b_, a_]
