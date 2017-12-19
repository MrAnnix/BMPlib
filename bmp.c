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
#include <sys/stat.h>
#include <errno.h>

#include "bmp.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define NUM_ERROR_MSGS_BMP 4

static const char *error_map_bmp[NUM_ERROR_MSGS_BMP] =
  {
    "Success",
    "Cannot load the file",
    "Cannot write into the file",
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


/*---------------------------------------------------------------------------*/
/* Function definitions                                                      */
/*---------------------------------------------------------------------------*/

const char *get_error_msg_bmp(int error){
  if(error <= 0){
    if(-error > NUM_ERROR_MSGS_BMP){
      return error_map_bmp[-error];
    }
    return error_map_bmp[NUM_ERROR_MSGS_BMP];
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
    char bufferBM[2] = "\0";
    int bufferSize = 0;
    FILE *fd;

    if((fd = fopen(abs_path, "r")) == NULL){
      *error = errno;
			errno = 0;
  		return -1;
  	}
    if(fread(bufferBM, sizeof(char), 2, fd) != 2){
      fclose(fd);
      *error = errno;
			errno = 0;
      return -1;
    }
    if(fread(&bufferSize, sizeof(int), 1, fd) != 4){
      fclose(fd);
      *error = errno;
			errno = 0;
      return -1;
    }
    fclose(fd);

    if((!strcmp(bufferBM, "BM"))&&(bufferSize == info.st_size)){
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

  if(fread(image->fh.type, 2, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->fh.bfSize, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(image->fh.bfReserved, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->fh.bfOffBits, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biSize, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biWidth, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biHeight, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biPlanes, 2, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biBitCount, 2, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biCompression, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biSizeImage, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biXPelsPerMeter, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biYPelsPerMeter, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biClrUsed, 4, 1, fd) != 1){
    if(errno){
      *error = errno;
      errno = 0;
    }else{
      *error = CANNOT_LOAD;
    }
    fclose(fd);
    return -1;
  }

  if(fread(&image->ih.biClrImportant, 4, 1, fd) != 1){
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

  image->padding = (4 - (image->ih.biWidth * sizeof(PIXELS)) % 4) % 4;

  image->bitmap = (PIXELS **)malloc(sizeof(PIXELS *)*image->ih.biHeight);
  int i;
  for(i=0; i<image->ih.biHeight; i++){
    image->bitmap[i] = (PIXELS*)malloc(sizeof(PIXELS)*image->ih.biWidth);
    fread(image->bitmap[i], sizeof(PIXELS), image->ih.biWidth, fd);
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
  for(i=0; i<image->ih.biHeight; i++){
    if(image->bitmap[i]!= NULL){
      free(image->bitmap[i]);
    }
  }
  if(image->bitmap != NULL){
    free(image->bitmap);
  }
}

int save_image(BMPFILE image, char *path, int *error){
  FILE *fd;
  if((fd = fopen(path, "r")) == NULL){
    *error = errno;
    errno = 0;
    return -1;
	}

  fwrite(&image.fh, sizeof(BITMAPFILEHEADER), 1, fd);
  fwrite(&image.ih, sizeof(BITMAPINFOHEADER), 1, fd);
  fwrite(image.alignment, image.aligment_size, 1, fd );

  int i, k;
  for(i=0; i<image.ih.biHeight; ++i){
    fwrite(image.bitmap[i], sizeof(PIXELS), image.ih.biWidth, fd);
    for(k = 0; k < image.padding; k++){
      fputc(0, fd);
    }
  }
  fclose(fd);
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/
