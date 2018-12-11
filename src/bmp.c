/**BMPlib***********************************************************************

  File        bmp.c

  Resume      Library for the treat of 24 bit BMP images

  Description This library only supports 24 bit uncompressed pixel bitmap
            format files.

  See also    bmp.h

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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

#include "bmp.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define NUM_ERROR_MSGS_BMP 5

#define E_PI    3.141592653589793238462643383279502884197169399375105820974
#define E_TAU   6.283185307179586476925286766559005768394338798750211641950
#define E_PI_SQ 9.869604401089358618834490999876151135313699407240790626413

static const char *error_map_bmp[NUM_ERROR_MSGS_BMP] =
  {
    "Success",
    "Cannot load the file",
    "Cannot write into the file",
    "Not supported format",
    "Unknown error"
  };

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


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
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

void RGBtoHSV(float r, float g, float b, float* h, float* s, float* v);

void HSVtoRGB(float h, float s, float v, float *r, float *g, float *b);

RGBTRIPLE **generate_bitmap(int new_height, int new_width, int *error);

RGBTRIPLE **rotate_bitmap(RGBTRIPLE **bitmap, int height, int width, char motion
          , int *error);

void free_bitmap(RGBTRIPLE **bitmap, int height);

void call_gnuplot(char *csv_template, char *path, int *error);

RGBTRIPLE **resample_bitmap(RGBTRIPLE **bitmap, int new_height, int new_width
          , int old_height, int old_width, int *error);

double sinc(double var);

double _L(double var);

double _G(double var, double sigma);

double fast_sin(double var);

double fast_exp(double x);

int max(int a, int b);

int min(int a, int b);

double d_max(int n_args, double  a, double b, ...);

double d_min(int n_args, double  a, double b, ...);

/*---------------------------------------------------------------------------*/
/* Function definitions                                                      */
/*---------------------------------------------------------------------------*/

const char *get_error_msg_bmp(int error){
  if(error <= 0){
    if(-error < NUM_ERROR_MSGS_BMP){
      return error_map_bmp[-error];
    }
    return error_map_bmp[NUM_ERROR_MSGS_BMP - 1];
  }else{
    return strerror(error);
  }
}

int is_BMP(char *path, int *error){
  char *abs_path = NULL;
  char buffer[PATH_MAX] = "\0";

  abs_path = realpath(path, buffer);
  if(errno){
    *error = errno;
    errno = 0;
    return -1;
  }

  struct stat info;
  if(lstat(abs_path, &info)){
    *error = errno;
    errno = 0;
    return -1;
  }
  if(S_ISREG(info.st_mode)){
    WORD bufferBM = 0x0000;
    DWORD bufferSize = 0;
    FILE *fd;

    if((fd = fopen(abs_path, "r")) == NULL){
      *error = errno;
			errno = 0;
  		return -1;
  	}
    if(fread(&bufferBM, sizeof(WORD), 1, fd) != 1){
      fclose(fd);
      *error = errno;
			errno = 0;
      return -1;
    }
    if(fread(&bufferSize, sizeof(DWORD), 1, fd) != 1){
      fclose(fd);
      *error = errno;
			errno = 0;
      return -1;
    }
    fclose(fd);

    if((bufferBM == 0x4D42) &&(bufferSize == info.st_size)){
      return 1;
    }
  }
  return 0;
}

int load_image(BMPFILE *image, char *path, int *error){
  char *abs_path = NULL;
  char buffer[PATH_MAX] = "\0";

  abs_path = realpath(path, buffer);
  if(errno){
    *error = errno;
    errno = 0;
    return -1;
  }

  FILE *fd;
  if((fd = fopen(abs_path, "r")) == NULL){
    *error = errno;
    errno = 0;
    return -1;
	}

  image->alignment = NULL;
  image->bitmap = NULL;

  if(fread(&image->fh.bfType, sizeof(WORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->fh.bfSize, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->fh.bfReserved1, sizeof(WORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->fh.bfReserved2, sizeof(WORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->fh.bfOffBits, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biSize, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biWidth, sizeof(LONG), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biHeight, sizeof(LONG), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biPlanes, sizeof(WORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biBitCount, sizeof(WORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(image->ih.biBitCount != 24){//Not 24bit image
    *error = NOT_SPT_FMT;
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biCompression, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(image->ih.biCompression){//Compressed image
    *error = NOT_SPT_FMT;
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biSizeImage, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biXPelsPerMeter, sizeof(LONG), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biYPelsPerMeter, sizeof(LONG), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biClrUsed, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biClrImportant, sizeof(DWORD), 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  image->aligment_size = image->fh.bfOffBits - ftell(fd);

  if(image->aligment_size == 0){
    image->alignment = NULL;
  }else{
    image->alignment = malloc(image->aligment_size);
    if(fread(image->alignment, image->aligment_size, 1, fd) != 1){
      if(errno){
        *error = errno;
        errno = 0;
      }else{
        *error = CANNOT_LOAD;
      }
      fclose(fd);
      return -1;
    }
  }

  image->padding = (4 - (image->ih.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

  image->bitmap = malloc(sizeof(RGBTRIPLE*)*image->ih.biHeight);
  int i;
  for(i=0; i<image->ih.biHeight; i++){
    image->bitmap[i] = malloc(sizeof(RGBTRIPLE)*image->ih.biWidth);
    fread(image->bitmap[i], sizeof(RGBTRIPLE), image->ih.biWidth, fd);
    fseek(fd, image->padding, SEEK_CUR);
  }

  fclose(fd);
  return 0;
}

void clean_image(BMPFILE *image){
  if(image->alignment != NULL){
    free(image->alignment);
  }

  int  i;

  if(image->bitmap != NULL){
    for(i=0; i<image->ih.biHeight; i++){
      if(image->bitmap[i]!= NULL){
        free(image->bitmap[i]);
      }
    }
    free(image->bitmap);
  }
}

int save_image(BMPFILE *image, char *path, int *error){
  FILE *fd;
  if((fd = fopen(path, "w")) == NULL){
    *error = errno;
    errno = 0;
    return -1;
	}

  fwrite(&image->fh, sizeof(BITMAPFILEHEADER), 1, fd);
  fwrite(&image->ih, sizeof(BITMAPINFOHEADER), 1, fd);
  fwrite(image->alignment, image->aligment_size, 1, fd );

  int i, k;
  for(i=0; i<image->ih.biHeight; ++i){
    fwrite(image->bitmap[i], sizeof(RGBTRIPLE), image->ih.biWidth, fd);
    for(k = 0; k < image->padding; k++){
      fputc(0, fd);
    }
  }
  fclose(fd);
  return 0;
}

void zero(BMPFILE *image, int mask){
  int i,j;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      image->bitmap[i][j].b &=  mask      & 0xFF;
      image->bitmap[i][j].g &= (mask>>8)  & 0xFF;
      image->bitmap[i][j].r &= (mask>>16) & 0xFF;
    }
  }
}

void sepia(BMPFILE *image){
  int i,j,r,g,b;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      r = image->bitmap[i][j].r*0.393 + image->bitmap[i][j].g*0.769
          + image->bitmap[i][j].b*0.189;
      g = image->bitmap[i][j].r*0.349 + image->bitmap[i][j].g*0.686
          + image->bitmap[i][j].b*0.168;
      b = image->bitmap[i][j].r*0.272 + image->bitmap[i][j].g*0.534
          + image->bitmap[i][j].b*0.131;

      (r>255) ? (image->bitmap[i][j].r = 255) : (image->bitmap[i][j].r = r);
      (g>255) ? (image->bitmap[i][j].g = 255) : (image->bitmap[i][j].g = g);
      (b>255) ? (image->bitmap[i][j].b = 255) : (image->bitmap[i][j].b = b);
    }
  }
}

void saturation(BMPFILE *image, int sat_p){
  double contrast_d = ((double)sat_p)/100.0;

  int i,j;
  float r,g,b,h,s,v;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      r = ((double)image->bitmap[i][j].r)/255.0;
      g = ((double)image->bitmap[i][j].g)/255.0;
      b = ((double)image->bitmap[i][j].b)/255.0;

      RGBtoHSV(r, g, b, &h, &s, &v);
      if((s *= contrast_d)>1.0){
        s = 1.0;
      }
      HSVtoRGB(h, s, v, &r, &g, &b);

      image->bitmap[i][j].r = (BYTE)(r*255.0);
      image->bitmap[i][j].g = (BYTE)(g*255.0);
      image->bitmap[i][j].b = (BYTE)(b*255.0);
    }
  }
}

void brightness(BMPFILE *image, int bright){
  double bright_d = ((double)bright)/100.0;

  int i,j;
  float r,g,b,h,s,v;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      r = ((double)image->bitmap[i][j].r)/255.0;
      g = ((double)image->bitmap[i][j].g)/255.0;
      b = ((double)image->bitmap[i][j].b)/255.0;

      RGBtoHSV(r, g, b, &h, &s, &v);
      if((v *= bright_d)>1.0){
        v = 1.0;
      }
      HSVtoRGB(h, s, v, &r, &g, &b);

      image->bitmap[i][j].r = (BYTE)(r*255.0);
      image->bitmap[i][j].g = (BYTE)(g*255.0);
      image->bitmap[i][j].b = (BYTE)(b*255.0);
    }
  }
}

void chroma(BMPFILE *image, int angle){
  int i,j;
  float r,g,b,h,s,v;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      r = ((double)image->bitmap[i][j].r)/255.0;
      g = ((double)image->bitmap[i][j].g)/255.0;
      b = ((double)image->bitmap[i][j].b)/255.0;

      RGBtoHSV(r, g, b, &h, &s, &v);

      h+=angle;

      int loops = ((int)h)/360;
      h = h - ((double)loops)*360.0;

      HSVtoRGB(h, s, v, &r, &g, &b);

      image->bitmap[i][j].r = (BYTE)(r*255.0);
      image->bitmap[i][j].g = (BYTE)(g*255.0);
      image->bitmap[i][j].b = (BYTE)(b*255.0);
    }
  }
}

void bitone(BMPFILE *image, RGBTRIPLE dark, RGBTRIPLE light, int threshold){
  int i,j;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      if((image->bitmap[i][j].r + image->bitmap[i][j].g + image->bitmap[i][j].b)
          < threshold){
        image->bitmap[i][j] = dark;
      }else{
        image->bitmap[i][j] = light;
      }
    }
  }
}

void grayscale(BMPFILE *image, char rgby){
  int i,j;
  BYTE result;

  switch (rgby){
    case 'r':
    for(i=0; i<image->ih.biHeight; i++){
        for(j=0; j<image->ih.biWidth; j++){
           result = image->bitmap[i][j].r;
           image->bitmap[i][j].b = result;
           image->bitmap[i][j].g = result;
        }
    }
    break;

    case 'g':
    for(i=0; i<image->ih.biHeight; i++){
        for(j=0; j<image->ih.biWidth; j++){
          result = image->bitmap[i][j].g;
          image->bitmap[i][j].r = result;
          image->bitmap[i][j].g = result;
        }
    }
    break;

    case 'b':
    for(i=0; i<image->ih.biHeight; i++){
        for(j=0; j<image->ih.biWidth; j++){
          result = image->bitmap[i][j].b;
          image->bitmap[i][j].r = result;
          image->bitmap[i][j].g = result;
        }
    }
    break;

    case 'y':
    for(i=0; i<image->ih.biHeight; i++){
        for(j=0; j<image->ih.biWidth; j++){
          result = image->bitmap[i][j].r*0.2126 + image->bitmap[i][j].g*0.7152
              + image->bitmap[i][j].b*0.0722;
          image->bitmap[i][j].r = result;
          image->bitmap[i][j].g = result;
          image->bitmap[i][j].b = result;
        }
    }
    break;
  }
}

void invert(BMPFILE *image){
  int i,j;
  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      image->bitmap[i][j].r = 255 - image->bitmap[i][j].r;
      image->bitmap[i][j].g = 255 - image->bitmap[i][j].g;
      image->bitmap[i][j].b = 255 - image->bitmap[i][j].b;
    }
  }
}

void blackandwhite(BMPFILE *image){
  unsigned int histo[256] = {0};

  int i,j,y;
  for(i=0; i<image->ih.biHeight; i++){
    for (j=0; j<image->ih.biWidth; j++){
      y = image->bitmap[i][j].r*0.2126 + image->bitmap[i][j].g*0.7152
          + image->bitmap[i][j].b*0.0722;
      histo[y]++;
    }
  }
  int total = image->ih.biWidth * image->ih.biHeight;
  //Otsu's Method
  double sum = 0;
  for(int i=0 ; i<256 ; i++){
    sum += i * histo[i];
  }

  double var_max = 0;

  double sumB = 0;
  int wB = 0;
  int wF = 0;
  int threshold = 0;

  for(i=0; i<256; ++i){
    wB += histo[i];
    wF = total - wB;
    if(wB == 0 || wF == 0){
      continue;
    }

    sumB += i*histo[i];

    double mB = sumB/wB;
    double mF = (sum - sumB)/wF;

    double between = (double)wB * (double)wF * (mB - mF) * (mB - mF);

    if(between>var_max){
      threshold = i;
      var_max = between;
    }
  }

  RGBTRIPLE light = {0xFF,0xFF,0xFF};
  RGBTRIPLE dark = {0x00,0x00,0x00};
  bitone(image, dark, light, threshold*3);
}

void mirror(BMPFILE *image, char hv, int *error){
  BMPFILE aux;

  if(bmpdup(image, &aux, error)){
    return;
  }

  int i,j;
  if(hv == 'v'){
    int w = image->ih.biWidth;

    for(i=0; i<image->ih.biHeight; i++){
      for(j=0; j<image->ih.biWidth; j++){
        memcpy(&image->bitmap[i][j], &aux.bitmap[i][w-j-1], sizeof(RGBTRIPLE));
      }
    }
  }else if(hv == 'h'){
    int h = image->ih.biHeight;

    for(i=0; i<image->ih.biHeight; i++){
      for(j=0; j<image->ih.biWidth; j++){
        memcpy(&image->bitmap[i][j], &aux.bitmap[h-1-i][j], sizeof(RGBTRIPLE));
      }
    }
  }else{
    *error = UNKNOWN;
  }

  clean_image(&aux);
}

int rotate(BMPFILE *image, char motion, int *error){
  int new_width;
  int new_height;
  int new_XPelsPerMeter = image->ih.biYPelsPerMeter;
  int new_YPelsPerMeter = image->ih.biXPelsPerMeter;

  RGBTRIPLE **new_im;

  new_width = image->ih.biHeight;
  new_height = image->ih.biWidth;

  new_im = rotate_bitmap(image->bitmap, image->ih.biHeight, image->ih.biWidth
                      , motion, error);
  if(new_im == NULL){
    return -1;
  }

  free_bitmap(image->bitmap, image->ih.biHeight);

  image->ih.biWidth = new_width;
  image->ih.biHeight = new_height;
  image->ih.biXPelsPerMeter = new_XPelsPerMeter;
  image->ih.biYPelsPerMeter = new_YPelsPerMeter;

  image->padding = (4 - (image->ih.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

  int old_biSizeImage = image->ih.biSizeImage;
  image->ih.biSizeImage = image->ih.biHeight*(image->ih.biWidth*3
        +image->padding);

  image->fh.bfSize = image->fh.bfSize + image->ih.biSizeImage - old_biSizeImage;

  image->bitmap = new_im;
  return 0;
}

int generate_histogram(BMPFILE *image, char *path, int *error){
  unsigned int histo_r[256];
  unsigned int histo_g[256];
  unsigned int histo_b[256];
  memset(histo_r, 0, sizeof(int)*255);//clean the arrays
  memset(histo_g, 0, sizeof(int)*255);
  memset(histo_b, 0, sizeof(int)*255);

  int r,g,b;
  r = 0; g = 0; b = 0;

  int i,j;
  for(i=0; i<image->ih.biHeight; i++){
    for (j=0; j<image->ih.biWidth; j++){
      r = image->bitmap[i][j].r;
      g = image->bitmap[i][j].g;
      b = image->bitmap[i][j].b;

      histo_r[r]++;
      histo_g[g]++;
      histo_b[b]++;
    }
  }

  FILE *fd;
  int temp;
  char template[]="histogram_XXXXXX.csv";
  temp = mkstemps(template, 4);

  if((fd = fdopen(temp, "w")) == NULL){
    *error = errno;
    errno = 0;
    return -1;
  }

  fprintf(fd, "Intensity\t");
  fprintf(fd, "Red\t");
  fprintf(fd, "Green\t");
  fprintf(fd, "Blue\n");

  for(i=0; i<256; i++){
    fprintf(fd, "%d",i);
    fprintf(fd, "\t");
    fprintf(fd, "%u", histo_r[i]);
    fprintf(fd, "\t");
    fprintf(fd, "%u", histo_g[i]);
    fprintf(fd, "\t");
    fprintf(fd, "%u", histo_b[i]);
    fprintf(fd, "\n");
  }
  fclose(fd);

  call_gnuplot(template, path, error);

  remove(template);

  return 0;
}

int bmpdup(BMPFILE *source, BMPFILE *dest, int *error){
  dest->fh = source->fh;
  dest->ih = source->ih;
  dest->aligment_size = source->aligment_size;
  if(source->alignment){
    dest->alignment = malloc(source->aligment_size);
    if(errno){
      *error = errno;
      errno = 0;
      return -1;
    }
    memcpy(dest->alignment, source->alignment, source->aligment_size);
  }else{
    dest->alignment = NULL;
  }
  dest->padding = source->padding;
  dest->bitmap = NULL;
  dest->bitmap = malloc(sizeof(RGBTRIPLE *)*source->ih.biHeight);
  if(errno){
    if(dest->bitmap != NULL){
      free(dest->bitmap);
    }
    *error = errno;
    errno = 0;
    return -1;
  }
  int i,j,a;
  for(a=0; a<source->ih.biHeight; a++){
    dest->bitmap[a] = NULL;
    dest->bitmap[a] = malloc(sizeof(RGBTRIPLE)*source->ih.biWidth);
    if(errno){
      for(i=0; i<a; i++){
        if(dest->bitmap[i] != NULL){
          free(dest->bitmap[i]);
        }
      }
      if(dest->bitmap != NULL){
        free(dest->bitmap);
      }
      *error = errno;
      errno = 0;
      return -1;
    }
  }

  for(i = 0; i<source->ih.biHeight; i++){
    for(j = 0; j<source->ih.biWidth; j++){
      dest->bitmap[i][j].r = source->bitmap[i][j].r;
  	  dest->bitmap[i][j].b = source->bitmap[i][j].b;
      dest->bitmap[i][j].g = source->bitmap[i][j].g;
    }
  }
  return 0;
}

int reduce(BMPFILE *image, int factor, int *error){
  int new_width = image->ih.biWidth/factor;
  int new_height = image->ih.biHeight/factor;
  int new_XPelsPerMeter = image->ih.biXPelsPerMeter/factor;
  int new_YPelsPerMeter = image->ih.biYPelsPerMeter/factor;

  RGBTRIPLE **new_im;

  new_im = resample_bitmap(image->bitmap, new_height, new_width,
                  image->ih.biHeight, image->ih.biWidth, error);

  if(new_im == NULL){
    return -1;
  }

  free_bitmap(image->bitmap, image->ih.biHeight);

  image->ih.biWidth = new_width;
  image->ih.biHeight = new_height;
  image->ih.biXPelsPerMeter = new_XPelsPerMeter;
  image->ih.biYPelsPerMeter = new_YPelsPerMeter;

  image->padding = (4 - (image->ih.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

  int old_biSizeImage = image->ih.biSizeImage;
  image->ih.biSizeImage = image->ih.biHeight * (image->ih.biWidth * 3
      + image->padding);

  image->fh.bfSize = image->fh.bfSize + image->ih.biSizeImage
      - old_biSizeImage;

  image->bitmap = new_im;
  return 0;
}

int enlarge(BMPFILE *image, int factor, int *error){
  int new_width = image->ih.biWidth*factor;
  int new_height = image->ih.biHeight*factor;
  int new_XPelsPerMeter = image->ih.biXPelsPerMeter*factor;
  int new_YPelsPerMeter = image->ih.biYPelsPerMeter*factor;

  RGBTRIPLE **new_im = NULL;

  new_im = resample_bitmap(image->bitmap, new_height, new_width,
                  image->ih.biHeight, image->ih.biWidth, error);

  if(new_im == NULL){
    return -1;
  }

  free_bitmap(image->bitmap, image->ih.biHeight);

  image->ih.biWidth = new_width;
  image->ih.biHeight = new_height;
  image->ih.biXPelsPerMeter = new_XPelsPerMeter;
  image->ih.biYPelsPerMeter = new_YPelsPerMeter;

  image->padding = (4 - (image->ih.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

  int old_biSizeImage = image->ih.biSizeImage;
  image->ih.biSizeImage = image->ih.biHeight * (image->ih.biWidth * 3
      + image->padding);

  image->fh.bfSize = image->fh.bfSize + image->ih.biSizeImage
      - old_biSizeImage;

  image->bitmap = new_im;
  return 0;
}

int crop(BMPFILE *image, unsigned char x_1, unsigned char y_1
        , unsigned char x_2, unsigned char y_2, int *error){

  if((x_1>=x_2)||(y_1>=y_2)||(x_2>100)||(y_2>100)){
    *error = UNKNOWN;
    return -1;
  }

  int x1 = (image->ih.biWidth*x_1)/100;
  int x2 = (image->ih.biWidth*x_2)/100;
  int y1 = (image->ih.biHeight*y_1)/100;
  int y2 = (image->ih.biHeight*y_2)/100;

  uint new_width = x2-x1;
  uint new_height = y2-y1;

  RGBTRIPLE** new_bitmap = NULL;
  if(!(new_bitmap = generate_bitmap(new_height, new_width, error))){
    return -1;
  }

  int i,j;
  for(i=0; i<new_height; ++i){
    for(j=0; j<new_width; ++j){
      new_bitmap[i][j] = image->bitmap[i+x1][j+y1];
    }
  }

  free_bitmap(image->bitmap, image->ih.biHeight);

  image->ih.biWidth = new_width;
  image->ih.biHeight = new_height;
  image->ih.biXPelsPerMeter
      = image->ih.biXPelsPerMeter*(image->ih.biWidth/new_width);
  image->ih.biYPelsPerMeter
      = image->ih.biYPelsPerMeter*(image->ih.biHeight/new_height);

  image->padding = (4 - (image->ih.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

  int old_biSizeImage = image->ih.biSizeImage;
  image->ih.biSizeImage = image->ih.biHeight * (image->ih.biWidth * 3
      + image->padding);

  image->fh.bfSize = image->fh.bfSize + image->ih.biSizeImage
      - old_biSizeImage;

  image->bitmap = new_bitmap;
  return 0;
}

int blur(BMPFILE *image, int radius, int *error){
  if(radius<2){
    *error = UNKNOWN;
    return -1;
  }
  int i, j, k, l, weight;

  int s_kernel = 10;
  double gaussian_kernel[10][10];

  double ii, jj;
  for(i=0; i<s_kernel; i++){
    for(j=0; j<s_kernel; j++){
      ii = (double)(i-s_kernel/2);
      jj = (double)(j-s_kernel/2);
      gaussian_kernel[i][j] = _G(ii, radius)*_G(jj, radius);
    }
  }

  double norm = 1/gaussian_kernel[0][0];
  int max_weight = 0;

  for(i=0; i<s_kernel; i++){
    for(j=0; j<s_kernel; j++){
      gaussian_kernel[i][j] *= norm;
      max_weight += gaussian_kernel[i][j];
    }
  }

	int kern_len = s_kernel/2;
	int pixel[3];

  RGBTRIPLE **new_bitmap = generate_bitmap(image->ih.biHeight, image->ih.biWidth
          , error);
  if(new_bitmap == NULL){
    return -1;
  }

  for(i=0; i<image->ih.biHeight; i++){
    for(j=0; j<image->ih.biWidth; j++){
      memset(pixel, 0, 3*sizeof(int));
			weight = max_weight;
      for(k=0; k<s_kernel; k++){
        for(l=0; l<s_kernel; l++){
          int h = i-kern_len+k;
          int w = j-kern_len+l;
          int gaussian_term = (int) gaussian_kernel[k][l];
          if((h>=0)&&(w>=0)&&(h<image->ih.biHeight)&&(w<image->ih.biWidth)){
            pixel[0] += gaussian_term * image->bitmap[h][w].r;
            pixel[1] += gaussian_term * image->bitmap[h][w].g;
            pixel[2] += gaussian_term * image->bitmap[h][w].b;
          }else{
            weight -= gaussian_term;
          }
        }
      }
      new_bitmap[i][j].r = pixel[0]/weight;
      new_bitmap[i][j].g = pixel[1]/weight;
      new_bitmap[i][j].b = pixel[2]/weight;
		}
	}

  free_bitmap(image->bitmap, image->ih.biHeight);
  image->bitmap = new_bitmap;

  return 0;
}

/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/

void RGBtoHSV(float r, float g, float b, float* h, float* s, float* v){
	float min, max, delta;
	min = d_min(3, r, g, b);
	max = d_max(3, r, g, b);
  delta = max - min;
  //V
	*v = max;
  //S
	if(max != 0){
		*s = delta / max;
	}else{
    *s = 0;
		*h = -1;
		return;
	}
  //H
	if(r == max){
		*h = (g - b)/delta;
	}else if(g == max){
		*h = 2 + (b - r)/delta;
	}else{
		*h = 4 + (r - g)/delta;
  }
  *h *= 60;
	if(*h < 0){
		*h += 360;
  }
}

void HSVtoRGB(float h, float s, float v, float *r, float *g, float *b){
	int i;
	float f, p, q, t;
	if(s == 0) {
		*r = *g = *b = v;
		return;
	}
	h /= 60;
	i = h;
	f = h - i;
	p = v*(1 - s);
	q = v*(1 - s*f);
	t = v*(1 - s*(1 - f));
	switch(i){
    case 0:
			*r = v;
			*g = t;
			*b = p;
		  break;

		case 1:
			*r = q;
			*g = v;
			*b = p;
      break;

		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;

		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;

		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;

		default:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}

RGBTRIPLE **generate_bitmap(int new_height, int new_width, int *error){
  RGBTRIPLE **new_bitmap;

  if((new_bitmap = malloc(new_height * sizeof(RGBTRIPLE*))) == NULL){
    *error = errno;
    errno = 0;
    return NULL;
  }

  int i,a;
  for(i=0; i<new_height; i++){
    new_bitmap[i] = NULL;
    new_bitmap[i] = malloc(new_width * sizeof(RGBTRIPLE));
    if(new_bitmap[i] == NULL){
      for(a=0; a<i; a++){
        if(new_bitmap[a] != NULL){
          free(new_bitmap[a]);
        }
      }
      if(new_bitmap != NULL){
        free(new_bitmap);
      }
      *error = errno;
      errno = 0;
      return NULL;
    }
    memset(new_bitmap[i], 0, new_width * sizeof(RGBTRIPLE));
  }
  return new_bitmap;
}

void free_bitmap(RGBTRIPLE **bitmap, int height){
  int i;
  for(i=0; i<height; i++){
    if(bitmap[i] != NULL){
      free(bitmap[i]);
    }
  }
  if(bitmap != NULL){
    free(bitmap);
  }
}

RGBTRIPLE **rotate_bitmap(RGBTRIPLE **bitmap, int height, int width, char motion
        , int *error){
  int new_width = height;
  int new_height = width;

  RGBTRIPLE **new_bitmap = generate_bitmap(new_height, new_width, error);
  if(new_bitmap == NULL){
    return NULL;
  }

  int i,j;
  for (i = 0; i < new_height; i++) {
    for (j = 0; j < new_width; j++) {
        if(motion == 'r')
          new_bitmap[i][j] = bitmap[j][width-1-i];
        else
          new_bitmap[i][j] = bitmap[height-1-j][i];
    }
  }
  return new_bitmap;
}

void call_gnuplot(char *csv_template, char *path, int *error){
  int scpt;
  char script[]="script_XXXXXX.gp";
  scpt = mkstemps(script, 3);

  FILE *fd;

  if((fd = fdopen(scpt, "w")) == NULL){
    *error = errno;
    errno = 0;
    return;
  }

  fprintf(fd, "set xrange [0:255]\n");
  fprintf(fd, "plot for [COL=2:4] inputfile using COL title columnheader with");
  fprintf(fd, " lines\n");
  fprintf(fd, "set terminal png\n");
  fprintf(fd, "set output outputfile\n");
  fprintf(fd, "replot\n");

  fclose(fd);

  char f_argument[33] = "inputfile='";
  strcat(f_argument, csv_template);
  strcat(f_argument,"'");
  char s_argument[PATH_MAX+20] = "outputfile='";
  strcat(s_argument, path);
  strcat(s_argument,".png'");

  int status = 0;
  pid_t fk = fork();
  if(fk == 0){
    execl("/usr/bin/gnuplot", "gnuplot", "-e", f_argument, "-e", s_argument
            , script, NULL);
  }else{
    wait(&status);
  }
  if(status){
    *error = UNKNOWN;
  }
  remove(script);

  if(errno){
    *error = errno;
    errno = 0;
  }
}

RGBTRIPLE **resample_bitmap(RGBTRIPLE **bitmap, int new_height, int new_width
        , int old_height, int old_width, int *error){

  RGBTRIPLE **new_bitmap = generate_bitmap(new_height, new_width, error);
  if(new_bitmap == NULL){
    return NULL;
  }

  double row_ratio = (double)old_height / (double)new_height;
  double col_ratio = (double)old_width / (double)new_width;

  int x,y,i,j;

  for(i=0; i<new_height; i++){
    double old_i = i*row_ratio;
    double floor_i = (int)old_i;
    for(j=0; j<new_width; j++){
      double old_j = j*col_ratio;
      double floor_j = (int)old_j;

      double r = 0.0;
      double g = 0.0;
      double b = 0.0;
      double weight = 0.0;

      for(x=floor_i-2+1; x<=floor_i+2; x++){
        for(y=floor_j-2+1; y<=floor_j+2; y++){
          if((x<old_height)&&(y<old_width)&&(x>=0)&&(y>=0)){
            double lanc_term = _L(old_i-x) * _L(old_j-y);
            r += bitmap[x][y].r * lanc_term;
            g += bitmap[x][y].g * lanc_term;
            b += bitmap[x][y].b * lanc_term;
            weight += lanc_term;
          }
        }
      }
      r /= weight;
      g /= weight;
      b /= weight;

      if((r < 0.0)||(r > 255.0)){
        r = (r > 255.0) ? 255.0 : 0;
      }
      if((g < 0.0)||(g > 255.0)){
        g = (g > 255.0) ? 255.0 : 0;
      }
      if((b < 0.0)||(b > 255.0)){
        b = (b > 255.0) ? 255.0 : 0;
      }

      new_bitmap[i][j].r = r;
      new_bitmap[i][j].g = g;
      new_bitmap[i][j].b = b;
    }
  }
  return new_bitmap;
}

double fast_sin(double var){
  int loops = var/(E_TAU);
  var = var - loops*E_TAU;

  if(var>E_PI){
    var = E_PI - var;
  }else if((-var)>E_PI){
    var = -E_PI - var;
  }

  if(var<0){
    var = -var;
    return -(16.0*var*(E_PI - var))/(5.0*E_PI_SQ - 4.0*var*(E_PI - var));
  }else{
    return (16.0*var*(E_PI - var))/(5.0*E_PI_SQ - 4.0*var*(E_PI - var));
  }
}

double sinc(double var){
  if(var == 0.0){
    return 1.0;
  }
  return fast_sin(E_PI*var)/(E_PI*var);
}

double _L(double var){//Lanczos kernel window size = 2
  if((var < 2)&&(var > -2)){
    return sinc(var)*sinc(var/2.0);
  }
  return 0;
}

double _G(double var, double sigma){//Gaussian function
  return (1/(2*E_PI*sigma*sigma))*fast_exp(-(var*var)/(2*sigma*sigma));
}

int max(int a, int b){
  if(a > b){
    return a;
  }
  return b;
}

int min(int a, int b){
  if(a < b){
    return a;
  }
  return b;
}

double d_max(int n_args, double  a, double b, ...){
  n_args -= 2;
  double maximum, aux;

  if(a > b){
    maximum = a;
  }else{
    maximum = b;
  }

  va_list numbers;
  va_start (numbers, b);

  int i;
  for(i=0; i<n_args; i++){
    aux = va_arg (numbers, double);
    if(aux > maximum){
      maximum = aux;
    }
  }

  va_end (numbers);
  return maximum;
}

double d_min(int n_args, double  a, double b, ...){
  n_args -= 2;
  double minimum, aux;

  if(a < b){
    minimum = a;
  }else{
    minimum = b;
  }

  va_list numbers;
  va_start (numbers, b);

  int i;
  for(i=0; i<n_args; i++){
    aux = va_arg (numbers, double);
    if(aux < minimum){
      minimum = aux;
    }
  }

  va_end (numbers);
  return minimum;
}

double fast_exp(double x){
  volatile union{
    float f;
    unsigned int i;
  }cvt;

  float t = x * 1.442695041;
  float fi = (float)((int)t);
  float f = t - fi;
  int i = (int)fi;
  cvt.f = (0.3371894346 * f + 0.657636276) * f + 1.00172476;
  cvt.i += (i << 23);
  return (double)cvt.f;
}
