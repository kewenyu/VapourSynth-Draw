# VapourSynth-Draw
A simple alternative to MaskTools2's mt_lutspa.

## Description
VapourSynth-Draw is a filter that behaves almost the same as mt_lutspa: Accept 
an expression input and take the pixel's coordinate into the calculation.

## Supported Format
8~16 bit integer YUV or GRAY clip.

## Usage and Parameters
```
draw.Draw(clip, expr[])
```
* clip: 8~16 bit integer YUV or GRAY clip.
* expr: expressions. Multiple expressions can be given to individual planes of a clip. The
following operators can be used in the expressions:
    * +, -, *, /, abs, pow, max, min
    * \>, <, >=, <=, =, not, and, or
    * ?
    * numbers: any integer/float point numbers without exponent

## Example
```python
from vapoursynth import core

src = # an 1080p YUV clip...

# Draw a white circle at the middle of a 8bit 1080p clip with radius of 100

circle = core.draw.Draw(src, 'x 960 - 2 pow y 540 - 2 pow + 100 2 pow < 255 0 ?')

# Draw a white rectangular on Y plane and set UV to 128

rect = core.draw.Draw(src, ['x 960 - abs 100 < y 540 - abs 50 < and 255 0 ?', '128'])
```
* note: due to the use of the lookup table, it will take some time to initialize. But
after that, it will affect little on processing speed.