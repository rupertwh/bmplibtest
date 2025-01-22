# bmplibtest

Testing framework for [bmplib](https://github.com/rupertwh/bmplib).



## Test definitions:

Use the `-f` command line option to specify a file which contains the
definitions of all tests to be run. (see `--help`)

The test parser is very simplisitic, no quoting, escaping or even nesting.
Leading and trailing white space around commands, options, and their values is
ignored.

Every test consists of a series of commands. Tests are separated by blank lines.

Commands may be (but don't have to be) separated from each other with white
space or semicolons.

Lines starting with '#' are comments. (Comment lines do not count as separators
between tests.)

Example, defining two tests ("Load 8-bit indexed" and "Test HDR 64-bit"):
```

# simple test; load a BMP file and compare against a reference PNG
name{Load 8-bit indexed}
loadbmp{sample,g/pal8.bmp}; loadpng{ref,ref_8bit_252c.png}
compare{}

#
# more complicated test
#
name          { Test HDR 64-bit }
#
# Load PNG and save as 64bit BMP with exposure +2
#
loadpng       { ref, almdudler.png }
convertformat { float, 0 }
convertgamma  { srgb, linear }
exposure      { fstops: 2 }
savebmp       { hdr-64bit.bmp, 64bit: yes }
#
# Reset exposure and convert to int8 and add alpha channel
#
exposure      { fstops: -2 }
convertformat { int, 8 }
addalpha      { }
#
# Load the 64bit BMP that we saved above
#
loadbmp       { tmp, hdr-64bit.bmp, format: float, conv64: linear }
#
# Reset exposure and convert to int8, then compare with previous,
# allowing slight rounding differences with 'fuzz: 1'
#
exposure      { fstops: -2 }
convertformat { int, 8 }
compare       { fuzz: 1 }
```


### Available commands

#### loadbmp
Load a BMP image onto the image stack.

`loadbmp{<dir>,<file>,...}`

##### Mandatory (positional) arguments:
- `<dir>` must be one of the lables "sample", "ref", or "tmp". The actual path
  name is defined via command line options or environment variables
  (see `--help`)
- `<file>` the file name. May include subdirectories, e.g. "g/test.bmp".
##### Optional (named) arguments:
- `line:whole|line` Whether to read the whole image at once, or line-by-line.
- `rgb:rgb|index` Whether to load indexed images as RGB or index + palette.
- `undef:alpha|leave` Whether to make undefined pixels in RLE images
  transparent. (adds an alpha channel to the loaded image)
- `conv64:srgb|linear` How to convert (or not) 64bit gamma.
- `format:int|float|s2.13` Specify number format for loaded image.
- `insane:yes` Load images larger than bmplib's insanity limit.

-------------------------------------------------------------------------------
#### savebmp
Save the topmost image on the stack to a BMP file.

`savebmp{<file>,...}`

##### Mandatory (positional) arguments:
- `<file>` the file name. May include subdirectories, e.g. "abc/test.bmp".
##### Optional (named) arguments:
- `format:int|float|s2.13` Number format used to supply image data to bmplib.
  Note: the image will be converted to the specified format before saving, and
  the converted image will be left on the stack. Use duplicate{}/delete{} to
  preserve the original. When specifying `format:int`, `bufferbits` must be
  set, as well.
- `bufferbits` number of bits when specifying `format:int`. Will otherwise be
  ignored.
- `rle:auto|rle8|none` Which RLE compression to use. `auto` selects one from
  RLE4, RLE8, RLE24, or Huffman-1D, depending on the supplied image data
  (and the `allow`-option, see below).
- `allow:huff|2bit|rle24` allow writing the respective compressions / pixel
  formats. Option may be repeated to allow more than one of the posibilites.
- `outbits:r<n>g<n>b<n>a<n>` Specify the channel bit-depths for the BMP file.
  E.g. `outbits: r10g10b10a0` to write a 32-bit BMP with RGBA channels
  10-10-10 - 0.
- `64bit:yes` write a 64bit BMP file.


-------------------------------------------------------------------------------
#### loadpng
Load a PNG image onto the image stack.

`loadpng{<dir>,<file>}`

-------------------------------------------------------------------------------
#### compare
Compare the 2 images on top of the stack. Test fails if images are not
identical.

`compare{fuzz:<n>}`
##### Optional (named) arguments:
- `fuzz:<n>` Allow a difference of `n` between pixel values.

-------------------------------------------------------------------------------
#### delete
Remove the top image from the stack.

`delete{}`

-------------------------------------------------------------------------------
#### swap
Swap the top two images on the stack.

`swap{}`

-------------------------------------------------------------------------------
#### duplicate
Duplicate the top image on the stack.

`duplicate{}`

-------------------------------------------------------------------------------
#### addalpha
Add an alpha channel (full opacity) to the top image on the stack.

`addalpha{}`

-------------------------------------------------------------------------------
#### convertgamma
Convert the top image on the stack to the specified gamma curve.

`convertgamma{<from>,<to>}`
##### Mandatory (positional) arguments:
- `<from>`, `to` may be one of `srgb` or `linear`

-------------------------------------------------------------------------------
#### convertformat
Convert the top image on the stack to the specified number format.

`convertformat{<format>,<bits>}`
- `<format>` May be one of `int`, `float`, `s2.13`.
- `<bits>` Only needed for `int` format, otherwise ignored. Must be one of 8,
  16, 32.

-------------------------------------------------------------------------------
#### flatten
Convert the top image on the stack from indexed to RGB.

`flatten{}`

-------------------------------------------------------------------------------
#### exposure
Change the exposure (brightness) for the top image on the stack.

`exposure{fstops:<f>}`
- `fstops:<f>` Positive or negative floating point number.
