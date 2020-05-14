# Python >= 3.7.1

# This script converts bitmap image *.png into two c-array: one 
# containing bitmap data (*_bitmap) and other containing transparency data (*_mask).
# First two bytes in arrays are width and height in pixels.
# The rest is bitmap data. Data is stored in row-major order.
# Each row element (byte) represents a *column* of 8 pixels.

from PIL import Image
import os
import sys
from typing import *

Pixel = Tuple[int,int,int,int]
Pixels = List[Pixel]
PixelTestFun = Callable[[Pixel], bool]

def toBitmapArray(pixels: Pixels, width: int, isPixelSet: PixelTestFun) -> List[int]:
    buffer: List[int] = []
    height = len(pixels) // width
    bHeight = (height + 7) // 8
    
    for byteY in range(0, bHeight):
        for x in range(0, width):
            byte: int = 0
            for bitY in range(0, 8):
                idx = (byteY*8 + bitY) * width + x
                if idx < len(pixels) and isPixelSet(pixels[idx]):
                    byte |= (1<<bitY)
            buffer.append(byte)

    return buffer

# width, height, data...
def toSpriteFormat(width: int, height: int, bitmap: List[int]) -> List[int]:
    if width > 255:
        raise Exception(f"width is too big: {width}")
    if height > 255:
        raise Exception(f"height is too big: {height}")
    return [width, height] + bitmap

def toHexStr(data: List[int]) -> str:
    buff = ""
    for i in range(0, len(data)):
        buff += "0x{:02x}".format(data[i])
        if i != len(data) - 1:
            buff += ", "
            if i != 0 and (i+1)%16 == 0:
                buff += "\n"
    return buff
    #return ", ".join("0x{:02x}".format(c) for c in data)

#---

def toBitmapStr(arrName: str, pixels: Pixels, width: int, isPixelSet: PixelTestFun) -> str:
    height = len(pixels) // width
    return f"static const uint8_t {arrName}[] PROGMEM = {{\n{toHexStr(toSpriteFormat(width, height, toBitmapArray(pixels, width, isPixelSet)))}\n}};\n"

isNotWhite: PixelTestFun = lambda px: not ( (px[0] > 127 and px[1] > 127 and px[2] > 127) or (len(px) >=4 and px[3] == 0))
isNotTransparent: PixelTestFun = lambda px: len(px) < 4 or px[3] != 0

#---

#imgPath = ' '
imgPath = sys.argv[1]
im = Image.open(imgPath)
width, height = im.size
pixels = list(im.getdata())

basename = os.path.basename(imgPath)

#print(f"w = {width}, h = {height}, p = {pixels[4180]}")

arrName = os.path.splitext(basename.replace('-', '_'))[0]
arrDataName = f"{arrName}_bitmap"
arrMaskName = f"{arrName}_mask"

print(toBitmapStr(arrDataName, pixels, width, isNotWhite))
print(toBitmapStr(arrMaskName, pixels, width, isNotTransparent))
#print(f"const BitmapData {arrName}_data = {{{arrDataName}+2, {arrMaskName}+2}};")