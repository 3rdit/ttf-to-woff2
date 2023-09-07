# ttf-to-woff2 

`ttf-to-woff2` is a C++ library in development aimed at parsing TrueType Font (TTF) files and converting them into the WOFF2 format. This library provides the tools necessary to extract an array of font metrics and mappings from TTF files - and ideally will do so in a multi-threaded fashion.

**This project currently doesn't convert to WOFF2! It is still a work in progress.**

## Features
- **TTF Parsing**: Decode and understand the structure of TTF files.
- **WOFF2 Conversion**: Take the parsed TTF information and generate WOFF2 formatted font files.
- **Font Metrics Extraction**: Retrieve crucial metrics that dictate font rendering and layout.

## Currently Supported Tables (TTF)

- `head` - Contains global information about the font.
- `hhea` - Horizontal header, contains metrics and other data for horizontal writing.
- `hmtx` - Horizontal metrics, provides widths and left side bearings of glyphs.
- `name` - Naming table, contains textual information related to the font.
- `os/2` - OS/2 and Windows-specific metrics.
- `post` - PostScript-related information.
- `loca` - Index to location, provides offsets to glyph data.
- `kern` - Kerning, specifies spacing between certain character pairs.
- `fvar` - Font variations, used in variable fonts to define axes and styles.
- `gvar` - Glyph variations, provides glyph variations for variable fonts.
- `glyf` - Glyph Data
- `cmap` - Character to glyph mapping, defines the mapping of character codes to glyph indices.
- `maxp` - Maximum profile, contains size and other metrics needed for the layout.
- `gpos` - Glyph positioning (Work in Progress).

## Planned Tables (TTF)
1. `cvt` - Control Value Table
2. `fpgm` - Font Program
4. `hdmx` - Horizontal Device Metrics
5. `LTSH` - Linear Threshold Data
6. `prep` - Control Value Program
7. `VDMX` - Vertical Device Metrics
8. `vhea` - Vertical Metrics Header
9. `vmtx` - Vertical Metrics
10. `BASE` - Baseline Data
11. `GDEF` - Glyph Definition Data
12. `GSUB` - Glyph Substitution Data
13. `JSTF` - Justification Data
14. `EBDT` - Embedded Bitmap Data (also for EBLT and EBSC)
15. `bdat` - Bitmap Data
16. `bloc` - Bitmap Data Tables
17. `COLR` - Colour Table
18. `CPAL` - Colour Palette Table
19. `SVG` - SVG Table for colour fonts
20. `sbix` - Standard Bitmap Graphics Table
21. `MATH` - Math Layout Parameters

...and there are others as well.

## License
This project is licensed under the MIT License. See LICENSE for more details.
