from PIL import Image
import os
import os.path
im=Image.open("logo.png")
width, height = im.size
if os.path.exists("rawlogo.bin"):
    os.remove("rawlogo.bin")

rawLogoFile=open("rawlogo.bin","xb")

w=width.to_bytes(4,"little")
h=height.to_bytes(4,"little")

rawLogoFile.write(w)
rawLogoFile.write(h)

for y in range(height):
    for x in range(width):
        rawLogoFile.write(bytes(im.getpixel((x,y))[0:3]))

rawLogoFile.close()