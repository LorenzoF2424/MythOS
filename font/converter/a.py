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

# Mappa per i commenti (così il .h è leggibile)
cp437_special = [
    "Null", "☺", "☻", "♥", "♦", "♣", "♠", "•", 
    "◘", "○", "◙", "♂", "♀", "♪", "♫", "☼",
    "►", "◄", "↕", "‼", "¶", "§", "▬", "↨", 
    "↑", "↓", "→", "←", "∟", "↔", "▲", "▼"
]

for code in range(num_chars):
    img = Image.new("1", (width, height), 0)
    draw = ImageDraw.Draw(img)
    
    # LA MAGIA È QUI: Scegliamo il carattere GIUSTO da far disegnare a PIL
    if code < 32 or code == 127:
        # Per le prime 32 faccine/simboli, i font DOS usano la mappatura diretta
        char_to_draw = chr(code)
    else:
        # Per il resto (accentate, cornici, ecc.), diciamo a Python di pescare 
        # l'equivalente Unicode esatto della tabella CP437!
        char_to_draw = bytes([code]).decode('cp437')

    draw.text((0, 0), char_to_draw, font=font, fill=1)

    char_bytes = []
    for y in range(height):
        byte = 0
        for x in range(width):
            pixel = img.getpixel((x, y))
            byte = (byte << 1) | (1 if pixel else 0)
        char_bytes.append(byte)
    font_array.append(char_bytes)

with open(output_file, "w", encoding="utf-8") as f:
    f.write("#ifndef FONT_H\n#define FONT_H\n#include <stdint.h>\n\n")
    f.write("// font array generated automatically\n")
    f.write(f"inline const unsigned char fontBitmap[{num_chars}][{height}] = {{\n")
    
    for code, char_bytes in enumerate(font_array):
        line = ",".join(f"0x{b:02X}" for b in char_bytes)
        
        # Generiamo i commenti perfetti a fine riga
        if code < 32:
            display_char = cp437_special[code]
        elif code == 127:
            display_char = "⌂"
        elif code == 255:
            display_char = "nbsp"
        else:
            display_char = bytes([code]).decode('cp437')
            
        f.write(f"    {{{line}}}, // {code} ({display_char})\n")
        
    f.write("};\n#endif\n")

print(f"File C generated: {output_file}")