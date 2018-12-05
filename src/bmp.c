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

RGBTRIPLE **generate_bitmap(int new_height, int new_width, int *error);

RGBTRIPLE **rotate_bitmap(RGBTRIPLE **bitmap, int height, int width, char motion,
          int *error);

void free_bitmap(RGBTRIPLE **bitmap, int height);

void call_gnuplot(char *csv_template, char *path, int *error);

RGBTRIPLE **resample_bitmap(RGBTRIPLE **bitmap, int new_height, int new_width,
          int old_height, int old_width, int *error);

double sinc(double var);

double _L(double var);

double fast_sin(double var);

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

  image->bitmap = malloc(sizeof(RGBTRIPLE *)*image->ih.biHeight);
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
  }else{
    int h = image->ih.biHeight;

    for(i=0; i<image->ih.biHeight; i++){
      for(j=0; j<image->ih.biWidth; j++){
        memcpy(&image->bitmap[i][j], &aux.bitmap[h-1-i][j], sizeof(RGBTRIPLE));
      }
    }
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
  image->ih.biSizeImage = image->ih.biHeight*(image->ih.biWidth*3+image->padding);

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

/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/

RGBTRIPLE **generate_bitmap(int new_height, int new_width, int *error){
  RGBTRIPLE **new_bitmap;

  if((new_bitmap = malloc(new_height * sizeof(RGBTRIPLE *))) == NULL){
    *error = errno;
    errno = 0;
    return NULL;
  }

  int i,a;
  for(i=0; i<new_height; i++){
    new_bitmap[i] = NULL;
    new_bitmap[i] = malloc(new_width * sizeof(RGBTRIPLE *));
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
    memset(new_bitmap[i], 0, new_width * sizeof(RGBTRIPLE *));
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

RGBTRIPLE **rotate_bitmap(RGBTRIPLE **bitmap, int height, int width, char motion,
          int *error){
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
  fprintf(fd, "plot for [COL=2:4] inputfile using COL title columnheader with lines\n");
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
    execl("/usr/bin/gnuplot", "gnuplot", "-e", f_argument, "-e", s_argument, script, NULL);
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

RGBTRIPLE **resample_bitmap(RGBTRIPLE **bitmap, int new_height, int new_width,
          int old_height, int old_width, int *error){

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
  var = var%E_TAU;

  if(var>E_PI){
    var = E_PI - var;
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
  if(fabs(var) < 2){
    return sinc(var)*sinc(var/2.0);
  }

  return 0;
}
