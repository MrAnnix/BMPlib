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
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define SUCCESS_INPUT 0
#define CANNOT_LOAD -1
#define CANNOT_WRITE -2
#define UNKNOWN -3

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct __attribute__((__packed__)){
  unsigned char b;
  unsigned char g;
  unsigned char r;
}PIXELS;

typedef struct __attribute__((__packed__)){
  char type[2]; // Should be "BM"
  int bfSize;  // 4 bytes with the size
  char bfReserved[4]; // 4 bytes reserved
  int bfOffBits; // offset where the pixel array can be found
}BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct __attribute__((__packed__)){
  int biSize;  // size of the DIB header from this point
  int  biWidth; // width in pixels
  int  biHeight; //height in pixels
  short  biPlanes; // number of planes
  short  biBitCount; // number of bits per pixel
  int biCompression; // if compression BI_RGB = 0 ó BI_BITFIELDS = 3
  int biSizeImage; // size of the array data including padding
  int biXPelsPerMeter; // hor resolution of the image
  int  biYPelsPerMeter; // ver resoilution of the image
  int biClrUsed; // number of colours in the palette, 0 if it uses all
  int biClrImportant; // number of important colours, 0 if all are imp
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct image{
  BITMAPFILEHEADER fh;
  BITMAPINFOHEADER ih;
  int aligment_size;
  char *alignment; // characters between header and pixel map
  unsigned int padding; // row padding within the pixel map
  PIXELS **bitmap; // BITMAP multidimensional array
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

  Description  [opcional]

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

  Parameters   [opcional]

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

/**Function*******************************************************************

  Resume       [obligatorio]

  Description  [opcional]

  Parameters   [opcional]

  Colat. Effe. [obligatorio]

  See also     [opcional]

******************************************************************************/

int save_image(BMPFILE image, char *path_to_store, int *error);

#endif
