/**BMPlib***********************************************************************

  File        bmp.h

  Resume      Library for the treat of 24 bit BMP images

  Description This library only supports 24 bit uncompressed pixel bitmap
            format files.

  See also    bmp.c

  Autor       Raúl San Martín Aniceto

  Copyright (c) 2017 Raúl San Martín Aniceto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************/
#ifndef BMP_H
#define BMP_H

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define SUCCESS_INPUT 0
#define CANNOT_LOAD -1
#define CANNOT_WRITE -2
#define NOT_SPT_FMT -3
#define UNKNOWN -4

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t  LONG;

typedef struct __attribute__((__packed__)){
  BYTE b; // Blue channel
  BYTE g; // Green channel
  BYTE r; // Red channel
}RGBTRIPLE;

typedef struct __attribute__((__packed__)){
    BYTE b; // Blue channel
    BYTE g; // Green channel
    BYTE r; // Red channel
    BYTE a; // Alpha channel
}RGBQUAD;

typedef struct __attribute__((__packed__)){
  WORD bfType; // Should be "BM"
  DWORD bfSize;  // 4 bytes with the size
  WORD bfReserved1; // 2 bytes reserved
  WORD bfReserved2; // 2 bytes reserved
  DWORD bfOffBits; // offset where the pixel array can be found
}BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct __attribute__((__packed__)){
  DWORD biSize;  // size of the DIB header from this point
  LONG  biWidth; // width in pixels
  LONG  biHeight; //height in pixels
  WORD  biPlanes; // number of planes
  WORD  biBitCount; // number of bits per pixel
  DWORD biCompression; // if compression BI_RGB = 0 ó BI_BITFIELDS = 3
  DWORD biSizeImage; // size of the array data including padding
  LONG biXPelsPerMeter; // hor resolution of the image
  LONG  biYPelsPerMeter; // ver resoilution of the image
  DWORD biClrUsed; // number of colours in the palette, 0 if it uses all
  DWORD biClrImportant; // number of important colours, 0 if all are imp
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct image{
  BITMAPFILEHEADER fh;
  BITMAPINFOHEADER ih;
  size_t aligment_size;
  BYTE *alignment; // characters between header and pixel map
  DWORD padding; // row padding within the pixel map
  RGBTRIPLE **bitmap; // BITMAP multidimensional array
}BMPFILE;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/**is_BMP*******************************************************************

  Resume       Check if the file at path is a valid BMP file

  Parameters   -path: String with the path of the file.

  Colat. Effe. Return 1 if valid, 0 if not and -1 if there is a problem. error
              is set appropiatelly.

  See also <errno> <stat> <realpath>

******************************************************************************/

int is_BMP(char *path, int *error);

/**get_error_msg_bmp***********************************************************

  Resume       Returns the associated string to an error

  Colat. Effe. Is necessary to set the error var. to 0 before call it in order
              to avoid false errors.

******************************************************************************/

const char *get_error_msg_bmp(int error);

/**load_image*******************************************************************

  Resume       Loads to memory the image in path

  Description  Loads to memory the image in path. Writes in image pointer the
              direction of the image loaded.

  Parameters   [pointer to an error variable]

  Colat. Effe. It is allocated in dinamic mem. so,
              it can and must be freed with clean_image function. Also if there
              is an error, the error var. will be set appropiatelly.

  See also     clean_image

******************************************************************************/

int load_image(BMPFILE *image, char *path, int *error);

/**clean_image*****************************************************************

  Resume       Clean from dinamic memory the image allocated by load_image

  See also     load_image

******************************************************************************/

void clean_image(BMPFILE *image);

/**save_image******************************************************************

  Resume       Save the image in the system

  Description  Save the image pointed by image to the path given
            by path. If there is an error, the error variable is set.

  See also     load_image

******************************************************************************/

int save_image(BMPFILE *image, char *path, int *error);

/**sepia*******************************************************************

  Resume       Gives to the bitmap a sepia tone.

******************************************************************************/

void sepia(BMPFILE *image);

/**grayscale*******************************************************************

  Resume       Converts the bitmap the image to bitonal.

  Description  Converts the bitmap the image to bitonal. It computes the sum of
            the tree components R+G+B. If this is less than the threshold, it
            will copy the value of the PIXELS dark into the bitmap pixel.
            If the sum is greater or equal than the threshold, it must copy the
            value of the PIXELS light.

******************************************************************************/

void bitone(BMPFILE *image, RGBTRIPLE dark, RGBTRIPLE light, int threshold);

/**grayscale*******************************************************************

  Resume       Gives the bitmap of the image a gray tone

  Description  Coverts the image to a grayscale. The channel selected to gray
            has to be selected by the rgby valiable. 'r' for the red, 'g' for
            the green, 'b' for blue and 'y' for the luminance computed as:
            r*0.2126 + g*0.7152 + b*0.072

******************************************************************************/

void grayscale(BMPFILE *image, char rgby);

/**invert*********************************************************************

  Resume       Inverts the color of the image

******************************************************************************/

void invert(BMPFILE *image);

/**mirror**********************************************************************

  Resume       Leflects the image

  Description  reflcts the image following the indications in hv. If hv value
          is 'h' it is reflected horizontally and if 'v' it is reflected
          vertically.

  Colat. Effe. If it occurs an error, the error variable is set appropiatelly.

******************************************************************************/

void mirror(BMPFILE *image, char hv, int *error);

/**rotate*********************************************************************

  Resume       Rotates 90º to the left/right the image

  Description  Rotates 90º to the left/right the image following the indication
          in motion. If motion value is 'l' it is rotated to the left and if 'r'
          it is rotated to the right.

  Colat. Effe. If it occurs an error, the error variable is set appropiatelly.

******************************************************************************/

int rotate(BMPFILE *image, char motion, int *error);

/**generate_histogram*********************************************************

  Resume       Generates the histogram of the image

  Description  Generates the histogram of the tree channels and luminance of
            image, and it is saved in path as a png file (.png is added). If it
            occurs an error, the function returns -1 and error is set
            appropiatelly. It is required to have  gnuplot installed.

  See also     man pages of gnuplot

******************************************************************************/

int generate_histogram(BMPFILE *image, char *path, int *error);

/**bmpdup*********************************************************************

  Resume       Copies the image source to dest

  Description  Returns in dest a pointer to a new image, which is a duplicate
            of the image pointed to by source. The returned pointer must be
            passed to clean_image to avoid a memory leak.

  Colat. Effe. If an error occurs, -1 is returned, a null pointer is set in
            dest and there may be set the error variable.

  See also     clean_image

******************************************************************************/

int bmpdup(BMPFILE *source, BMPFILE *dest, int *error);

/**reduce*********************************************************************

  Resume       Reduces the resolution of the image

  Description  Decimates the bitmap of the image. Reduces the samples of the
            image by a factor. For it reduces the high frecuency components
            of the signal (low-pass filter), and keeps one of every M samples.
            If it occurs an error, the function returns -1 and error is set
            appropiatelly.

  See also     https://en.wikipedia.org/wiki/Decimation_(signal_processing)

******************************************************************************/

int reduce(BMPFILE *image, int factor, int *error);

/**enlarge********************************************************************

  Resume       Enlarges the resolution of the image (Lanczos)

  Description  Upsamples the bitmap of the image. Increase the samples of the
            image by a factor. And then reduces the high frecuency components
            of the signal (Interpolation). If there is an error, the function
            returns -1 and error is set appropiatelly.

  See also     https://en.wikipedia.org/wiki/Upsampling

******************************************************************************/

int enlarge(BMPFILE *image, int factor, int *error);

/**Function*******************************************************************

  Resume       [obligatorio]

  Description  [opcional]

  Parameters   [opcional]

  Colat. Effe. [obligatorio]

  See also     [opcional]

******************************************************************************/

#endif
