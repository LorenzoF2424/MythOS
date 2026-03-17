from PIL import Image, ImageFont, ImageDraw

# =================  =================
font_path = "Mx437_DOS-V_TWN16.ttf"    # 
output_file = "fontBitmap.h"  # 
font_size = 16                # 
num_chars = 256               # 
width = 8                     # 
height = 16                   # 
# ==================================================


font = ImageFont.truetype(font_path, font_size)

font_array = []

for code in range(num_chars):
    img = Image.new("1", (width, height), 0)
    draw = ImageDraw.Draw(img)
    draw.text((0, 0), chr(code), font=font, fill=1)

    char_bytes = []
    for y in range(height):
        byte = 0
        for x in range(width):
            pixel = img.getpixel((x, y))
            byte = (byte << 1) | (1 if pixel else 0)
        char_bytes.append(byte)
    font_array.append(char_bytes)

with open(output_file, "w") as f:
    f.write("// font array generated automatically\n")
    f.write(f"unsigned char fontBitmap[{num_chars}][{height}] = {{\n")
    for char in font_array:
        line = ", ".join(f"0x{b:02X}" for b in char)
        f.write(f"    {{ {line} }},\n")
    f.write("};\n")

print(f"File C generated: {output_file}")