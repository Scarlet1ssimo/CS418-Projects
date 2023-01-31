import sys
from PIL import Image
inputFileName = sys.argv[1]
with open(inputFileName, "r", encoding="ascii") as f:
    for ip in f.readlines():
        keywords = ip.strip().split()
        if not keywords:
            continue
        if keywords[0] == 'png':
            width = int(keywords[1])
            height = int(keywords[2])
            filename = keywords[3]
            image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        elif keywords[0] == 'xyrgb':
            x = int(keywords[1])
            y = int(keywords[2])
            red = int(keywords[3])
            green = int(keywords[4])
            blue = int(keywords[5])
            # print(red, green, blue)
            image.im.putpixel((x, y), (red, green, blue, 255))
        elif keywords[0] == 'xyc':
            x = int(keywords[1])
            y = int(keywords[2])
            c = keywords[3]
            red = int(c[1:3], 16)
            green = int(c[3:5], 16)
            blue = int(c[5:], 16)
            # print(red, green, blue)
            image.im.putpixel((x, y), (red, green, blue, 255))
    image.save(filename)
