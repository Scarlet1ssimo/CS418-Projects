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
