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


def rgb1_to_srgb255(rgb_color):
    r, g, b, a = rgb_color
    r, g, b = convert_color_component(r, True), convert_color_component(
        g, True), convert_color_component(b, True)
    return (r * 255, g * 255, b * 255, a)
