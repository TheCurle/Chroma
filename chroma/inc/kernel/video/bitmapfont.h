/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

// This file contains all of the bitmap fonts made by me (Curle) and taken from the public domain
// eg. http://dimensionalrift.homelinux.net/combuster/mos3/?p=viewsource&file=/modules/gfx/font8_8.asm


const unsigned char bitfont_latin[128][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
    { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
    { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
    { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
    { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
    { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
    { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
    { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
    { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
    { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
    { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
    { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
    { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (//)
    { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
    { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
    { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
    { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
    { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
    { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
    { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
    { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
    { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
    { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
    { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
    { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
    { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
    { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
    { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
    { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
    { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
    { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
    { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
    { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
    { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
    { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
    { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
    { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
    { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
    { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
    { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
    { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
    { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
    { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
    { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
    { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
    { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
    { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
    { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
    { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
    { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
    { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
    { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
    { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
    { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
    { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
    { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};


const unsigned char bitfont_extra[96][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00A0 (no break space)
    { 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00},   // U+00A1 (inverted !)
    { 0x18, 0x18, 0x7E, 0x03, 0x03, 0x7E, 0x18, 0x18},   // U+00A2 (dollarcents)
    { 0x1C, 0x36, 0x26, 0x0F, 0x06, 0x67, 0x3F, 0x00},   // U+00A3 (pound sterling)
    { 0x00, 0x00, 0x63, 0x3E, 0x36, 0x3E, 0x63, 0x00},   // U+00A4 (currency mark)
    { 0x33, 0x33, 0x1E, 0x3F, 0x0C, 0x3F, 0x0C, 0x0C},   // U+00A5 (yen)
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+00A6 (broken pipe)
    { 0x7C, 0xC6, 0x1C, 0x36, 0x36, 0x1C, 0x33, 0x1E},   // U+00A7 (paragraph)
    { 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00A8 (diaeresis)
    { 0x3C, 0x42, 0x99, 0x85, 0x85, 0x99, 0x42, 0x3C},   // U+00A9 (copyright symbol)
    { 0x3C, 0x36, 0x36, 0x7C, 0x00, 0x00, 0x00, 0x00},   // U+00AA (superscript a)
    { 0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00},   // U+00AB (<<)
    { 0x00, 0x00, 0x00, 0x3F, 0x30, 0x30, 0x00, 0x00},   // U+00AC (gun pointing left)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00AD (soft hyphen)
    { 0x3C, 0x42, 0x9D, 0xA5, 0x9D, 0xA5, 0x42, 0x3C},   // U+00AE (registered symbol)
    { 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00AF (macron)
    { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00},   // U+00B0 (degree)
    { 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x7E, 0x00},   // U+00B1 (plusminus)
    { 0x1C, 0x30, 0x18, 0x0C, 0x3C, 0x00, 0x00, 0x00},   // U+00B2 (superscript 2)
    { 0x1C, 0x30, 0x18, 0x30, 0x1C, 0x00, 0x00, 0x00},   // U+00B2 (superscript 3)
    { 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00B2 (aigu)
    { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x03},   // U+00B5 (mu)
    { 0xFE, 0xDB, 0xDB, 0xDE, 0xD8, 0xD8, 0xD8, 0x00},   // U+00B6 (pilcrow)
    { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},   // U+00B7 (central dot)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x1E},   // U+00B8 (cedille)
    { 0x08, 0x0C, 0x08, 0x1C, 0x00, 0x00, 0x00, 0x00},   // U+00B9 (superscript 1)
    { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00},   // U+00BA (superscript 0)
    { 0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00},   // U+00BB (>>)
    { 0xC3, 0x63, 0x33, 0xBD, 0xEC, 0xF6, 0xF3, 0x03},   // U+00BC (1/4)
    { 0xC3, 0x63, 0x33, 0x7B, 0xCC, 0x66, 0x33, 0xF0},   // U+00BD (1/2)
    { 0x03, 0xC4, 0x63, 0xB4, 0xDB, 0xAC, 0xE6, 0x80},   // U+00BE (3/4)
    { 0x0C, 0x00, 0x0C, 0x06, 0x03, 0x33, 0x1E, 0x00},   // U+00BF (inverted ?)
    { 0x07, 0x00, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x00},   // U+00C0 (A grave)
    { 0x70, 0x00, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x00},   // U+00C1 (A aigu)
    { 0x1C, 0x36, 0x00, 0x3E, 0x63, 0x7F, 0x63, 0x00},   // U+00C2 (A circumflex)
    { 0x6E, 0x3B, 0x00, 0x3E, 0x63, 0x7F, 0x63, 0x00},   // U+00C3 (A ~)
    { 0x63, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x63, 0x00},   // U+00C4 (A umlaut)
    { 0x0C, 0x0C, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x00},   // U+00C5 (A ring)
    { 0x7C, 0x36, 0x33, 0x7F, 0x33, 0x33, 0x73, 0x00},   // U+00C6 (AE)
    { 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x18, 0x30, 0x1E},   // U+00C7 (C cedille)
    { 0x07, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00C8 (E grave)
    { 0x38, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00C9 (E aigu)
    { 0x0C, 0x12, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00CA (E circumflex)
    { 0x36, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00CB (E umlaut)
    { 0x07, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CC (I grave)
    { 0x38, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CD (I aigu)
    { 0x0C, 0x12, 0x00, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CE (I circumflex)
    { 0x33, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CF (I umlaut)
    { 0x3F, 0x66, 0x6F, 0x6F, 0x66, 0x66, 0x3F, 0x00},   // U+00D0 (Eth)
    { 0x3F, 0x00, 0x33, 0x37, 0x3F, 0x3B, 0x33, 0x00},   // U+00D1 (N ~)
    { 0x0E, 0x00, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00},   // U+00D2 (O grave)
    { 0x70, 0x00, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00},   // U+00D3 (O aigu)
    { 0x3C, 0x66, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00},   // U+00D4 (O circumflex)
    { 0x6E, 0x3B, 0x00, 0x3E, 0x63, 0x63, 0x3E, 0x00},   // U+00D5 (O ~)
    { 0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00},   // U+00D6 (O umlaut)
    { 0x00, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x00, 0x00},   // U+00D7 (multiplicative x)
    { 0x5C, 0x36, 0x73, 0x7B, 0x6F, 0x36, 0x1D, 0x00},   // U+00D8 (O stroke)
    { 0x0E, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},   // U+00D9 (U grave)
    { 0x70, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},   // U+00DA (U aigu)
    { 0x3C, 0x66, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x00},   // U+00DB (U circumflex)
    { 0x33, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+00DC (U umlaut)
    { 0x70, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x00},   // U+00DD (Y aigu)
    { 0x0F, 0x06, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+00DE (Thorn)
    { 0x00, 0x1E, 0x33, 0x1F, 0x33, 0x1F, 0x03, 0x03},   // U+00DF (beta)
    { 0x07, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E0 (a grave)
    { 0x38, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E1 (a aigu)
    { 0x7E, 0xC3, 0x3C, 0x60, 0x7C, 0x66, 0xFC, 0x00},   // U+00E2 (a circumflex)
    { 0x6E, 0x3B, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E3 (a ~)
    { 0x33, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E4 (a umlaut)
    { 0x0C, 0x0C, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E5 (a ring)
    { 0x00, 0x00, 0xFE, 0x30, 0xFE, 0x33, 0xFE, 0x00},   // U+00E6 (ae)
    { 0x00, 0x00, 0x1E, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+00E7 (c cedille)
    { 0x07, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},   // U+00E8 (e grave)
    { 0x38, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},   // U+00E9 (e aigu)
    { 0x7E, 0xC3, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00},   // U+00EA (e circumflex)
    { 0x33, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},   // U+00EB (e umlaut)
    { 0x07, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00EC (i grave)
    { 0x1C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00ED (i augu)
    { 0x3E, 0x63, 0x1C, 0x18, 0x18, 0x18, 0x3C, 0x00},   // U+00EE (i circumflex)
    { 0x33, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00EF (i umlaut)
    { 0x1B, 0x0E, 0x1B, 0x30, 0x3E, 0x33, 0x1E, 0x00},   // U+00F0 (eth)
    { 0x00, 0x1F, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x00},   // U+00F1 (n ~)
    { 0x00, 0x07, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F2 (o grave)
    { 0x00, 0x38, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F3 (o aigu)
    { 0x1E, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F4 (o circumflex)
    { 0x6E, 0x3B, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F5 (o ~)
    { 0x00, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F6 (o umlaut)
    { 0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18, 0x00},   // U+00F7 (division)
    { 0x00, 0x60, 0x3C, 0x76, 0x7E, 0x6E, 0x3C, 0x06},   // U+00F8 (o stroke)
    { 0x00, 0x07, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00F9 (u grave)
    { 0x00, 0x38, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00FA (u aigu)
    { 0x1E, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00FB (u circumflex)
    { 0x00, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00FC (u umlaut)
    { 0x00, 0x38, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+00FD (y aigu)
    { 0x00, 0x00, 0x06, 0x3E, 0x66, 0x3E, 0x06, 0x00},   // U+00FE (thorn)
    { 0x00, 0x33, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F}    // U+00FF (y umlaut)
};

const unsigned char bitfont_greek[58][8] = {
    { 0x2D, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0390 (iota with tonos and diaeresis)
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0391 (Alpha)
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0392 (Beta)
    { 0x3F, 0x33, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00},   // U+0393 (Gamma)
    { 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x63, 0x7F, 0x00},   // U+0394 (Delta)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0395 (Epsilon)
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+0396 (Zeta)
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0397 (Eta)
    { 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x36, 0x1C, 0x00},   // U+0398 (Theta)
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0399 (Iota)
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+039A (Kappa)
    { 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x63, 0x63, 0x00},   // U+039B (Lambda)
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+039C (Mu)
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+039D (Nu)
    { 0x7F, 0x63, 0x00, 0x3E, 0x00, 0x63, 0x7F, 0x00},   // U+039E (Xi)
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+039F (Omikron)
    { 0x7F, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00},   // U+03A0 (Pi)
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+03A1 (Rho)
    { 0x00, 0x01, 0x02, 0x04, 0x4F, 0x90, 0xA0, 0x40},   // U+03A2
    { 0x7F, 0x63, 0x06, 0x0C, 0x06, 0x63, 0x7F, 0x00},   // U+03A3 (Sigma 2)
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+03A4 (Tau)
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+03A5 (Upsilon)
    { 0x18, 0x7E, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x00},   // U+03A6 (Phi)
    { 0x63, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x63, 0x00},   // U+03A7 (Chi)
    { 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x18, 0x3C, 0x00},   // U+03A8 (Psi)
    { 0x3E, 0x63, 0x63, 0x63, 0x36, 0x36, 0x77, 0x00},   // U+03A9 (Omega)
    { 0x33, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0399 (Iota with diaeresis)
    { 0x33, 0x00, 0x33, 0x33, 0x1E, 0x0C, 0x1E, 0x00},   // U+03A5 (Upsilon with diaeresis)
    { 0x70, 0x00, 0x6E, 0x3B, 0x13, 0x3B, 0x6E, 0x00},   // U+03AC (alpha aigu)
    { 0x38, 0x00, 0x1E, 0x03, 0x0E, 0x03, 0x1E, 0x00},   // U+03AD (epsilon aigu)
    { 0x38, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x30},   // U+03AE (eta aigu)
    { 0x38, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+03AF (iota aigu)
    { 0x2D, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+03B0 (upsilon with tonos and diaeresis)
    { 0x00, 0x00, 0x6E, 0x3B, 0x13, 0x3B, 0x6E, 0x00},   // U+03B1 (alpha)
    { 0x00, 0x1E, 0x33, 0x1F, 0x33, 0x1F, 0x03, 0x03},   // U+03B2 (beta)
    { 0x00, 0x00, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x00},   // U+03B3 (gamma)
    { 0x38, 0x0C, 0x18, 0x3E, 0x33, 0x33, 0x1E, 0x00},   // U+03B4 (delta)
    { 0x00, 0x00, 0x1E, 0x03, 0x0E, 0x03, 0x1E, 0x00},   // U+03B5 (epsilon)
    { 0x00, 0x3F, 0x06, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+03B6 (zeta)
    { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x30},   // U+03B7 (eta)
    { 0x00, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x1E, 0x00},   // U+03B8 (theta)
    { 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+03B9 (iota)
    { 0x00, 0x00, 0x33, 0x1B, 0x0F, 0x1B, 0x33, 0x00},   // U+03BA (kappa)
    { 0x00, 0x03, 0x06, 0x0C, 0x1C, 0x36, 0x63, 0x00},   // U+03BB (lambda)
    { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x03},   // U+03BC (mu)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+03BD (nu)
    { 0x1E, 0x03, 0x0E, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+03BE (xi)
    { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+03BF (omikron)
    { 0x00, 0x00, 0x7F, 0x36, 0x36, 0x36, 0x36, 0x00},   // U+03C0 (pi)
    { 0x00, 0x00, 0x3C, 0x66, 0x66, 0x36, 0x06, 0x06},   // U+03C1 (rho)
    { 0x00, 0x00, 0x3E, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+03C2 (sigma 1)
    { 0x00, 0x00, 0x7E, 0x1B, 0x1B, 0x1B, 0x0E, 0x00},   // U+03C3 (sigma 2)
    { 0x00, 0x00, 0x7E, 0x18, 0x18, 0x58, 0x30, 0x00},   // U+03C4 (tau)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+03C5 (upsilon)
    { 0x00, 0x00, 0x76, 0xDB, 0xDB, 0x7E, 0x18, 0x00},   // U+03C6 (phi)
    { 0x00, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+03C7 (chi)
    { 0x00, 0x00, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x00},   // U+03C8 (psi)
    { 0x00, 0x00, 0x36, 0x63, 0x6B, 0x7F, 0x36, 0x00}    // U+03C9 (omega)
};

const unsigned char bitfont_box[128][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},   // U+2500 (thin horizontal)
    { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},   // U+2501 (thick horizontal)
    { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},   // U+2502 (thin vertical)
    { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},   // U+2503 (thich vertical)
    { 0x00, 0x00, 0x00, 0x00, 0xBB, 0x00, 0x00, 0x00},   // U+2504 (thin horizontal dashed)
    { 0x00, 0x00, 0x00, 0xBB, 0xBB, 0x00, 0x00, 0x00},   // U+2505 (thick horizontal dashed)
    { 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0x08, 0x08},   // U+2506 (thin vertical dashed)
    { 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18},   // U+2507 (thich vertical dashed)
    { 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00},   // U+2508 (thin horizontal dotted)
    { 0x00, 0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x00},   // U+2509 (thick horizontal dotted)
    { 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08},   // U+250A (thin vertical dotted)
    { 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18},   // U+250B (thich vertical dotted)
    { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08, 0x08},   // U+250C (down L, right L)
    { 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x08, 0x08, 0x08},   // U+250D (down L, right H)
    { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0x18, 0x18},   // U+250E (down H, right L)
    { 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18, 0x18, 0x18},   // U+250F (down H, right H)
    { 0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08},   // U+2510 (down L, left L)
    { 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x08, 0x08, 0x08},   // U+2511 (down L, left H)
    { 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x18, 0x18},   // U+2512 (down H, left L)
    { 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x18},   // U+2513 (down H, left H)
    { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00},   // U+2514 (up L, right L)
    { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x00, 0x00, 0x00},   // U+2515 (up L, right H)
    { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x00, 0x00, 0x00},   // U+2516 (up H, right L)
    { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x00, 0x00, 0x00},   // U+2517 (up H, right H)
    { 0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00},   // U+2518 (up L, left L)
    { 0x08, 0x08, 0x08, 0x0f, 0x0f, 0x00, 0x00, 0x00},   // U+2519 (up L, left H)
    { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x00, 0x00, 0x00},   // U+251A (up H, left L)
    { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00},   // U+251B (up H, left H)
    { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0x08},   // U+251C (down L, right L, up L)
    { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x08, 0x08, 0x08},   // U+251D (down L, right H, up L)
    { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x08, 0x08, 0x08},   // U+251E (down L, right L, up H)
    { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x18, 0x18, 0x18},   // U+251F (down H, right L, up L)
    { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0x18, 0x18},   // U+2520 (down H, right L, up H)
    { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x08, 0x08, 0x08},   // U+2521 (down L, right H, up H)
    { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x18, 0x18, 0x18},   // U+2522 (down H, right H, up L)
    { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18},   // U+2523 (down H, right H, up H)
    { 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08},   // U+2524 (down L, left L, up L)
    { 0x08, 0x08, 0x08, 0x0f, 0x0f, 0x08, 0x08, 0x08},   // U+2525 (down L, left H, up L)
    { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x08, 0x08, 0x08},   // U+2526 (down L, left L, up H)
    { 0x08, 0x08, 0x08, 0x08, 0x1f, 0x18, 0x18, 0x18},   // U+2527 (down H, left L, up L)
    { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x18, 0x18},   // U+2528 (down H, left L, up H)
    { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x08, 0x08, 0x08},   // U+2529 (down L, left H, up H)
    { 0x08, 0x08, 0x08, 0x1f, 0x1f, 0x18, 0x18, 0x18},   // U+252A (down H, left H, up L)
    { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18},   // U+252B (down H, left H, up H)
    { 0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08, 0x08},   // U+252C (down L, right L, left L)
    { 0x00, 0x00, 0x00, 0x0f, 0xff, 0x08, 0x08, 0x08},   // U+252D (down L, right L, left H)
    { 0x00, 0x00, 0x00, 0xf8, 0xff, 0x08, 0x08, 0x08},   // U+252E (down L, right H, left L)
    { 0x00, 0x00, 0x00, 0xff, 0xff, 0x08, 0x08, 0x08},   // U+252F (down L, right H, left H)
    { 0x00, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18},   // U+2530 (down H, right L, left L)
    { 0x00, 0x00, 0x00, 0x1f, 0xff, 0x18, 0x18, 0x18},   // U+2531 (down H, right L, left H)
    { 0x00, 0x00, 0x00, 0xf8, 0xff, 0x18, 0x18, 0x18},   // U+2532 (down H, right H, left L)
    { 0x00, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18},   // U+2533 (down H, right H, left H)
    { 0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00, 0x00},   // U+2534 (up L, right L, left L)
    { 0x08, 0x08, 0x08, 0x0f, 0xff, 0x00, 0x00, 0x00},   // U+2535 (up L, right L, left H)
    { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x00, 0x00, 0x00},   // U+2536 (up L, right H, left L)
    { 0x08, 0x08, 0x08, 0xff, 0xff, 0x00, 0x00, 0x00},   // U+2537 (up L, right H, left H)
    { 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0x00, 0x00},   // U+2538 (up H, right L, left L)
    { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x00, 0x00, 0x00},   // U+2539 (up H, right L, left H)
    { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x00, 0x00, 0x00},   // U+253A (up H, right H, left L)
    { 0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00},   // U+253B (up H, right H, left H)
    { 0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08},   // U+253C (up L, right L, left L, down L)
    { 0x08, 0x08, 0x08, 0x0f, 0xff, 0x08, 0x08, 0x08},   // U+253D (up L, right L, left H, down L)
    { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x08, 0x08, 0x08},   // U+253E (up L, right H, left L, down L)
    { 0x08, 0x08, 0x08, 0xff, 0xff, 0x08, 0x08, 0x08},   // U+253F (up L, right H, left H, down L)
    { 0x18, 0x18, 0x18, 0x18, 0xff, 0x08, 0x08, 0x08},   // U+2540 (up H, right L, left L, down L)
    { 0x08, 0x08, 0x08, 0x08, 0xff, 0x18, 0x18, 0x18},   // U+2541 (up L, right L, left L, down H)
    { 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18},   // U+2542 (up H, right L, left L, down H)
    { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x08, 0x08, 0x08},   // U+2543 (up H, right L, left H, down L)
    { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x08, 0x08, 0x08},   // U+2544 (up H, right H, left L, down L)
    { 0x08, 0x08, 0x08, 0x1f, 0xff, 0x18, 0x18, 0x18},   // U+2545 (up L, right L, left H, down H)
    { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x18, 0x18, 0x18},   // U+2546 (up L, right H, left L, down H)
    { 0x08, 0x08, 0x08, 0xff, 0xff, 0x18, 0x18, 0x18},   // U+2547 (up L, right H, left H, down H)
    { 0x18, 0x18, 0x18, 0xff, 0xff, 0x08, 0x08, 0x08},   // U+254B (up H, right H, left H, down L)
    { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x18, 0x18, 0x18},   // U+254A (up H, right H, left L, down H)
    { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x18, 0x18, 0x18},   // U+2549 (up H, right L, left H, down H)
    { 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18},   // U+254B (up H, right H, left H, down H)
    { 0x00, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00},   // U+254C (thin horizontal broken)
    { 0x00, 0x00, 0x00, 0xE7, 0xE7, 0x00, 0x00, 0x00},   // U+254D (thick horizontal broken)
    { 0x08, 0x08, 0x08, 0x00, 0x00, 0x08, 0x08, 0x08},   // U+254E (thin vertical broken)
    { 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18},   // U+254F (thich vertical broken)
    { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00},   // U+2550 (double horizontal)
    { 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14},   // U+2551 (double vertical)
    { 0x00, 0x00, 0x00, 0xF8, 0x08, 0xF8, 0x08, 0x08},   // U+2552 (down L, right D)
    { 0x00, 0x00, 0x00, 0x00, 0xFC, 0x14, 0x14, 0x14},   // U+2553 (down D, right L)
    { 0x00, 0x00, 0x00, 0xFC, 0x04, 0xF4, 0x14, 0x14},   // U+2554 (down D, right D)
    { 0x00, 0x00, 0x00, 0x0F, 0x08, 0x0F, 0x08, 0x08},   // U+2555 (down L, left D)
    { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x14, 0x14, 0x14},   // U+2556 (down D, left L)
    { 0x00, 0x00, 0x00, 0x1F, 0x10, 0x17, 0x14, 0x14},   // U+2557 (down D, left D)
    { 0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x00, 0x00},   // U+2558 (up L, right D)
    { 0x14, 0x14, 0x14, 0x14, 0xFC, 0x00, 0x00, 0x00},   // U+2559 (up D, right L)
    { 0x14, 0x14, 0x14, 0xF4, 0x04, 0xFC, 0x00, 0x00},   // U+255A (up D, right D)
    { 0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x00, 0x00},   // U+255B (up L, left D)
    { 0x14, 0x14, 0x14, 0x14, 0x1F, 0x00, 0x00, 0x00},   // U+255C (up D, left L)
    { 0x14, 0x14, 0x14, 0x17, 0x10, 0x1F, 0x00, 0x00},   // U+255D (up D, left D)
    { 0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x08, 0x08},   // U+255E (up L, down L, right D)
    { 0x14, 0x14, 0x14, 0x14, 0xF4, 0x14, 0x14, 0x14},   // U+255F (up D, down D, right L)
    { 0x14, 0x14, 0x14, 0xF4, 0x04, 0xF4, 0x14, 0x14},   // U+2560 (up D, down D, right D)
    { 0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x08, 0x08},   // U+2561 (up L, down L, left D)
    { 0x14, 0x14, 0x14, 0x14, 0x17, 0x14, 0x14, 0x14},   // U+2562 (up D, down D, left L)
    { 0x14, 0x14, 0x14, 0x17, 0x10, 0x17, 0x14, 0x14},   // U+2563 (up D, down D, left D)
    { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x08, 0x08},   // U+2564 (left D, right D, down L)
    { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x14, 0x14, 0x14},   // U+2565 (left L, right L, down D)
    { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x14, 0x14},   // U+2566 (left D, right D, down D)
    { 0x08, 0x08, 0x08, 0xFF, 0x00, 0xFF, 0x00, 0x00},   // U+2567 (left D, right D, up L)
    { 0x14, 0x14, 0x14, 0x14, 0xFF, 0x00, 0x00, 0x00},   // U+2568 (left L, right L, up D)
    { 0x14, 0x14, 0x14, 0xF7, 0x00, 0xFF, 0x00, 0x00},   // U+2569 (left D, right D, up D)
    { 0x08, 0x08, 0x08, 0xFF, 0x08, 0xFF, 0x08, 0x08},   // U+256A (left D, right D, down L, up L)
    { 0x14, 0x14, 0x14, 0x14, 0xFF, 0x14, 0x14, 0x14},   // U+256B (left L, right L, down D, up D)
    { 0x14, 0x14, 0x14, 0xF7, 0x00, 0xF7, 0x14, 0x14},   // U+256C (left D, right D, down D, up D)
    { 0x00, 0x00, 0x00, 0x00, 0xE0, 0x10, 0x08, 0x08},   // U+256D (curve down-right)
    { 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x08},   // U+256E (curve down-left)
    { 0x08, 0x08, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00},   // U+256F (curve up-left)
    { 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x00},   // U+2570 (curve up-right)
    { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01},   // U+2571 (diagonal bottom-left to top-right)
    { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80},   // U+2572 (diagonal bottom-right to top-left)
    { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},   // U+2573 (diagonal cross)
    { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00},   // U+2574 (left L)
    { 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00},   // U+2575 (up L)
    { 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00},   // U+2576 (right L)
    { 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08},   // U+2577 (down L)
    { 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00},   // U+2578 (left H)
    { 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},   // U+2579 (up H)
    { 0x00, 0x00, 0x00, 0xF8, 0xF8, 0x00, 0x00, 0x00},   // U+257A (right H)
    { 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18},   // U+257B (down H)
    { 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x00, 0x00, 0x00},   // U+257C (right H, left L)
    { 0x08, 0x08, 0x08, 0x08, 0x18, 0x18, 0x18, 0x18},   // U+257D (up L, down H)
    { 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00},   // U+257E (right L, left H)
    { 0x18, 0x18, 0x18, 0x18, 0x08, 0x08, 0x08, 0x08}    // U+257F (up H, down L)
};

const unsigned char bitfont_block[32][8] = {
    { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00},   // U+2580 (top half)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+2581 (box 1/8)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF},   // U+2582 (box 2/8)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF},   // U+2583 (box 3/8)
    { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2584 (bottom half)
    { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2585 (box 5/8)
    { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2586 (box 6/8)
    { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2587 (box 7/8)
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2588 (solid)
    { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F},   // U+2589 (box 7/8)
    { 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F},   // U+258A (box 6/8)
    { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},   // U+258B (box 5/8)
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F},   // U+258C (left half)
    { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},   // U+258D (box 3/8)
    { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},   // U+258E (box 2/8)
    { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},   // U+258F (box 1/8)
    { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0},   // U+2590 (right half)
    { 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00},   // U+2591 (25% solid)
    { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA},   // U+2592 (50% solid)
    { 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55},   // U+2593 (75% solid)
    { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+2594 (box 1/8)
    { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // U+2595 (box 1/8)
    { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F},   // U+2596 (box bottom left)
    { 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0},   // U+2597 (box bottom right)
    { 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00},   // U+2598 (box top left)
    { 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2599 (boxes left and bottom)
    { 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0},   // U+259A (boxes top-left and bottom right)
    { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F},   // U+259B (boxes top and left)
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0},   // U+259C (boxes top and right)
    { 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00},   // U+259D (box top right)
    { 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F},   // U+259E (boxes top right and bottom left)
    { 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF},   // U+259F (boxes right and bottom)
};