#ifndef __OCRDLL_H
#define __OCRDLL_H

/*
  WOCAR Engine v 2.x
  (c) 1997-1998 Cyril Cambien - All rights reserved.

  ocrdll.h unofficial header file

  /!\ WARNING /!\

  1) THIS IS UNOFFICIAL HEADER FILE. IT IS PROVIDED "AS IS" WITHOUT ANY EXPRESS
     OR IMPLIED WARRANTIES. USE AT YOUR OWN RISK.
  2) Be aware that almost all the functions doesn't have sanity checks. For example
     "GetImage(set, index)" literally returns result of "set->list[index]" without
     any data validation for invalid or NULL "set" pointer or "index" out of bounds.
  3) [v2.4] You get a fatal error if "biXPelsPerMeter" or "biYPelsPerMeter" image DPI less than 20.
     Change these values manually if you're using screenshots instead of real scanned images.
     From DIBToIMG() internal code:
       biXPelsPerMeter = 39 * (0.5 - (-0.0254 * DIB->biXPelsPerMeter));
       biYPelsPerMeter = 39 * (0.5 - (-0.0254 * DIB->biYPelsPerMeter));
       39 * (0.5 - (-0.0254 * ##)) => (0.5/0.0254) = 19.685039 =>
       39 * (0.5 - (-0.0254 * 19)) = 38.3214 < 39 - error
       39 * (0.5 - (-0.0254 * 20)) = 39.312 >= 39 - ok
       biXPelsPerMeter and biYPelsPerMeter always divided by 39 after this initialization.
       And since anything below 20 (as ##) will give 0 as result you'll get divide by 0 error.
  4) [v2.4] You get another fatal error for float point (0 / 0) if try to OCR completely
     black image or image where it's not enough white space. Remember: text on OCR image MUST BE
     black and background MUST BE white. It's mandatory for this OCR engine to work properly.
  5) WOCAR v2.x still has a lot of debugging fprintf()'s inside - don't be surprised with unexpected
     output if you're building console application. Though it's limited mostly to erroneous cases.
  6) All things marked as "undoc" was undocumented and probably valid only for WOCAR v2.x.
     It's strongly recommended to avoid any use if it unless you're really have to.
  7) Some original documentation typos and errors fixed, like SaveMultipleImg() actually return "int"
     instead of "void" and also fixed OT_* numeration compared to VobSubResync version of this file.
  8) To improve OCR results use *.WDC dictionary and try to zoom in OCR images at least x3 times.
  9) Any comments, questions and suggestions about this file accepted here:
     https://github.com/systoolz

  Known OCRDLL.DLL versions:

  v2.4 [1997-10-30 15:41:29 GMT] named as v2.5 in documentation
  TWAIN_RegisterApp(2, 4, 13, 1, "2.4", "Cyril Cambien", "WOCAR", "WOCAR for 95/NT")
  https://web.archive.org/web/20070324035310/http://ccambien.free.fr/wocar25.zip

  v2.6 [2002-01-07 18:04:19 GMT] recommended version
  TWAIN_RegisterApp(2, 6, 13, 1, "2.6", "Cyril Cambien", "WOCAR", "WOCAR for 95/NT")
  https://web.archive.org/web/20020228023446/http://www.simpleocr.com/Download/SimpleOCR.exe
  Notable changes:
  - fixed float point zero division for completely black images and zero DPI (see 3 and 4 above)
  - added DUTCH language support (but without DUTCH.WDC file)
  - smaller file size

  v2.6 [2002-03-19 01:17:34 GMT] debug version, big and unoptimized
  TWAIN_RegisterApp(2, 6, 13, 1, "2.6", "Cyril Cambien", "WOCAR", "WOCAR for 95/NT")
  https://web.archive.org/web/20020402130839/http://www.simpleocr.com/Download/SimpleOCR.exe
  Notable changes:
  - DUTCH.WDC file added, you can take it from this version and add to one above
  - larger .DLL file size, debug build, slow
  - latest version with OCRDLL.DLL and DLLTWAIN.DLL - API and .DLL files changed after
*/

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* undoc: undocumented internal structures */
#pragma pack(push, 1)
typedef struct {       /* memory allocated structure */
  BITMAPINFO *head;    /* memory allocated block */
  unsigned char *data; /* memory allocated block (bitmap data) */
  int size;            /* size of one image row in bytes */
  unsigned char *last; /* start of the last line of the pixels = data[img->size * (height - 1)] */
} IMG; /* undoc */

typedef struct { /* memory allocated structure */
  int count;     /* used elements in list[] */
  int alloc;     /* allocated elements in list[] */
  IMG  **list;   /* memory allocated block */
} SETOFIMG; /* undoc */
#pragma pack(pop)

/* SetLanguage():language */
enum {
  NONE,
  ENGLISH, /* file "ENGLISH.WDC" required */
  FRENCH,  /* file "FRENCH.WDC" required */
  DUTCH    /* file "DUTCH.WDC" required [v2.6+] */
};

/* SetOutputMode():mode */
enum {
  OM_TEXT,
  OM_RICHTEXT,
  OM_WINDOW
};

/* OCROutputHandler():event */
enum {
  OT_TEXT,
  OT_PROP,
  OT_ITAL,
  OT_ZZZZ, /* [3] undoc - unused ? */
  OT_UNDS,
  OT_SIZE,
  OT_HILT,
  OT_ENDL,
  OT_ENDZ,
  OT_BITM
};

typedef void (*OCRErrorHandler)(char *mod, char *fmt, va_list args);
typedef void (*OCROutputHandler)(int event, int param);
typedef int (*OCRProgressHandler)(int percent);
typedef void (*OCRWarningHandler)(char *mod, int code, char *fmt, va_list args);

/* This function is similar to AddImage, except that, instead of an IMG object,
   this function expects an image in the DIB format. */
int AddDIB(SETOFIMG *set, HGLOBAL hDib);

/* Add an image to a set of image. The new image will be added at the last position. */
int AddImage(SETOFIMG *set, IMG *image);

/* Creates an empty set of images. */
SETOFIMG *CreateMultipleImg(void);

/* Converts a memory block containing a Device Independent Bitmap (DIB) to an IMG object. */
IMG *DIBToIMG(HGLOBAL hDib);

/* Deletes and frees an image in a set. */
void DelImage(SETOFIMG *set, int index);

/* When a document has not been properly scanned, the resulting image can be skewed.
   This function analyses a skewed image and rotates it in order to fix the problem. */
int DeskewImg(IMG *img);

/* Sometimes a scanned image has black borders. It happens frequently when the scanned document
   is smaller than the scanning area. This function detects and removes these black borders. */
int EraseBlackBordersImg(IMG *img);

/* Extracts a rectangular area from an image. */
IMG *ExtractImgArea(IMG *img, int x, int y, int w, int h);

/* This function frees an existing image. */
void FreeImg(IMG *img);

/* Frees a set of images. All the images in the set are also freed. */
void FreeMultipleImg(SETOFIMG *set);

/* Gets a pointer to a given image in a set of images. */
IMG *GetImage(const SETOFIMG *set, int index);
/* RET set->list[index] */

/* This function gets a pointer to the bitmap corresponding to the image.
   The bitmap is organized like a Device Independent Bitmap (DIB). */
unsigned char *GetImgBitmap(const IMG *img);
/* RET img->data */

/* This function gets a pointer to the BITMAPINFO structure corresponding to the image. */
LPBITMAPINFO GetImgBitmapInfo(const IMG *img);
/* RET GetNbImages(img) - why this just not img->head ? */

/* This function returns the size in bytes of the bitmap corresponding to the image. */
int GetImgBitmapSize(const IMG *img);
/* RET img->size * img->head->bmiHeader.biHeight */

/* This function allows you to get the horizontal and vertical resolution of an image. */
void GetImgRes(const IMG *img, int *pw, int *ph);
/* RET img->head->bmiHeader.biXPelsPerMeter/39, img->head->bmiHeader.biYPelsPerMeter/39 */

/* This function allows you to get the size of an image, given in pixels. */
void GetImgSize(const IMG *img, int *pw, int *ph);
/* RET img->head->bmiHeader.biWidth, img->head->bmiHeader.biHeight */

/* Returns the number of images that a set of images contains. */
int GetNbImages(const SETOFIMG *set);
/* RET set->count */

/* Shrinks a bilevel image at 50% of the original size and returns the result in a grayscale image. */
IMG *HalfSizeImg(IMG *img);

/* Inserts an image in a set of image at a given position. */
int InsertImage(SETOFIMG *set, int index, IMG *image);

/* Inverts an image (black pixels becomes white and white pixels becomes black). */
int InvertImg(IMG *img);
/* RET always 0 for some reason */

/* Loads an image from a TIFF file. */
IMG *LoadImg(const char *filename);

/* Loads a set of images from a TIFF file. */
SETOFIMG *LoadMultipleImg(const char *filename);

/* Recognizes the text located in an image. */
int OCR(const IMG *img, int noisy);

/* Recognizes the text located in an image that contains a unique text area.
   This function doesn't do any layout analysis on the area.
   The image containing the area is usually extracted from a page with ExtractImgArea. */
int OCROnArea(const IMG *img, int noisy);
/* RET OCROnArea2(img, noisy, 0, 100) */

/* This function is similar to OCROnArea but allows you to give starting and ending values
   for the progress percentage.
   It is useful when you want to have to display a progress bar when processing several areas. */
int OCROnArea2(const IMG *img, int noisy, int startprogress, int endprogress);

/* undoc: undocumented function */
OCRErrorHandler OCRSetErrorHandler(OCRErrorHandler handler); /* undoc */
/*
void DefaultErrorHandler(char *mod, char *fmt, va_list args) {
  if (mod) { fprintf(stderr, "%s: ", mod); }
  vfprintf(stderr, fmt, args);
  fprintf(stderr, ".\n");
  exit(1);
}
*/

/* When the output mode is OM_TEXT or OM_RICHTEXT, a user defined function of
   type OCROutputHandler will be called by the OCR engine for each "OCR event". */
OCROutputHandler OCRSetOutputHandler(OCROutputHandler handler);
/*
// hWnd - from SetOutputWindow(), default = 0
void DefaultOutputHandler(int event, int param) {
  if (event == OT_TEXT) {
    if (param == '\n') {
      SendMessage(hWnd, WM_CHAR, '\n', 0);
    } else {
      SendMessage(hWnd, WM_CHAR, LOBYTE(param), 0);
    }
  }
}
*/

/* When the OCR engine processes a document, a user defined function of type OCRProgressHandler,
   is called several times. */
OCRProgressHandler OCRSetProgressHandler(OCRProgressHandler handler);
/*
int DefaultProgressHandler(int percent) {
  return(0);
}
*/

/* undoc: undocumented function */
OCRWarningHandler OCRSetWarningHandler(OCRWarningHandler handler); /* undoc */
/*
void DefaultWarningHandler(char *mod, int code, char *fmt, va_list args) {
  switch (code) {
    case 1:
      fprintf(stderr, "**** ");
      break;
    case 2:
      fprintf(stderr, "*** ");
      break;
    case 3:
      fprintf(stderr, "** ");
      break;
    default:
      fprintf(stderr, "* ");
      break;
  }
  if (mod) { fprintf(stderr, "%s: ", mod); }
  vfprintf(stderr, fmt, args);
  fprintf(stderr, ".\n");
}
*/

/* Replaces an image in a set of image at a given position. */
int ReplaceImage(SETOFIMG *set, int index, IMG *image);
/* RET always 0 for some reason */

/* Resizes an image. */
IMG *ResizeImg(IMG *img, int nw, int nh);

/* Rotates an image. */
int RotateImg(IMG *img, int angle);

/* Saves an image to a TIFF file. */
int SaveImg(const char *filename, const IMG *img);

/* Saves an image from a TIFF file. */
int SaveMultipleImg(const char *filename, SETOFIMG *set);

/* Acquires a new image and adds it to a previously created image set.
   The scanning session should have been initialized with ScanInit. */
int ScanAndAddImage(SETOFIMG * set);

/* Selects "Autobright" mode and lets the scanner determines an optimal brightness level. (Recommanded) */
void ScanAutoBright(int automode);

/* Detects if a scanner is connected to the computer. */
int ScanAvailable(void);

/* Changes scanning brightness. */
void ScanBrightness(int brightness);

/* Terminate a scanning section. */
void ScanEnd(void);

/* Acquires a new image. The scanning session should have been initialized with ScanInit. */
IMG *ScanImg(void);

/* Initializes the image acquisition process. */
int ScanInit(HWND hWnd);

/* Sets the scanning resolution. This function should be called before ScanInit. */
void ScanResolution(int resolution);

/* Lets the user select a given scanner if several scanners are connected to the computer. */
void ScanSelect(HWND hWnd);

/* Indicates if Wocar should use the scanner user interface or not.
   This function should be called before ScanInit. */
void ScanShowUI(int mode);

/* Sets the scanning threshold. This function should be called before ScanInit. */
void ScanThreshold(int threshold); /* undoc */

/* Selects the language used in the text you want to process. */
void SetLanguage(int language, const char *dictDir);

/* Selects the output mode for the OCR engine. */
void SetOutputMode(int mode);

/* When the output mode OT_WINDOW has been selected with the SetOutputMode function,
   this function allows you to indicate in which window the text will be sent. */
void SetOutputWindow(HWND hWnd);

/* Shrinks a bilevel image and returns the result in a grayscale image. */
IMG *ShrinkImg(IMG *img, int nw, int nh);

/* function types for dynamic library linking */

typedef int (*ADDDIBPROC)(SETOFIMG *set, HGLOBAL hDib);
typedef int (*ADDIMAGEPROC)(SETOFIMG *set, IMG *image);
typedef SETOFIMG *(*CREATEMULTIPLEIMGPROC)(void);
typedef IMG *(*DIBTOIMGPROC)(HGLOBAL hDib);
typedef void (*DELIMAGEPROC)(SETOFIMG *set, int index);
typedef int (*DESKEWIMGPROC)(IMG *img);
typedef int (*ERASEBLACKBORDERSIMGPROC)(IMG *img);
typedef IMG *(*EXTRACTIMGAREAPROC)(IMG *img, int x, int y, int w, int h);
typedef void (*FREEIMGPROC)(IMG *img);
typedef void (*FREEMULTIPLEIMGPROC)(SETOFIMG *set);
typedef IMG *(*GETIMAGEPROC)(const SETOFIMG *set, int index);
typedef unsigned char *(*GETIMGBITMAPPROC)(const IMG *img);
typedef LPBITMAPINFO (*GETIMGBITMAPINFOPROC)(const IMG *img);
typedef int (*GETIMGBITMAPSIZEPROC)(const IMG *img);
typedef void (*GETIMGRESPROC)(const IMG *img, int *pw, int *ph);
typedef void (*GETIMGSIZEPROC)(const IMG *img, int *pw, int *ph);
typedef int (*GETNBIMAGESPROC)(const SETOFIMG *set);
typedef IMG *(*HALFSIZEIMGPROC)(IMG *img);
typedef int (*INSERTIMAGEPROC)(SETOFIMG *set, int index, IMG *image);
typedef int (*INVERTIMGPROC)(IMG *img);
typedef IMG *(*LOADIMGPROC)(const char *filename);
typedef SETOFIMG *(*LOADMULTIPLEIMGPROC)(const char *filename);
typedef int (*OCRPROC)(const IMG *img, int noisy);
typedef int (*OCRONAREAPROC)(const IMG *img, int noisy);
typedef int (*OCRONAREA2PROC)(const IMG *img, int noisy, int startprogress, int endprogress);
typedef OCRErrorHandler (*OCRSETERRORHANDLERPROC)(OCRErrorHandler handler);
typedef OCROutputHandler (*OCRSETOUTPUTHANDLERPROC)(OCROutputHandler handler);
typedef OCRProgressHandler (*OCRSETPROGRESSHANDLERPROC)(OCRProgressHandler handler);
typedef OCRWarningHandler (*OCRSETWARNINGHANDLERPROC)(OCRWarningHandler handler);
typedef int (*REPLACEIMAGEPROC)(SETOFIMG *set, int index, IMG *image);
typedef IMG *(*RESIZEIMGPROC)(IMG *img, int nw, int nh);
typedef int (*ROTATEIMGPROC)(IMG *img, int angle);
typedef int (*SAVEIMGPROC)(const char *filename, const IMG *img);
typedef int (*SAVEMULTIPLEIMGPROC)(const char *filename, SETOFIMG *set);
typedef int (*SCANANDADDIMAGEPROC)(SETOFIMG * set);
typedef void (*SCANAUTOBRIGHTPROC)(int automode);
typedef int (*SCANAVAILABLEPROC)(void);
typedef void (*SCANBRIGHTNESSPROC)(int brightness);
typedef void (*SCANENDPROC)(void);
typedef IMG *(*SCANIMGPROC)(void);
typedef int (*SCANINITPROC)(HWND hWnd);
typedef void (*SCANRESOLUTIONPROC)(int resolution);
typedef void (*SCANSELECTPROC)(HWND hWnd);
typedef void (*SCANSHOWUIPROC)(int mode);
typedef void (*SCANTHRESHOLDPROC)(int threshold);
typedef void (*SETLANGUAGEPROC)(int language, const char *dictDir);
typedef void (*SETOUTPUTMODEPROC)(int mode);
typedef void (*SETOUTPUTWINDOWPROC)(HWND hWnd);
typedef IMG *(*SHRINKIMGPROC)(IMG *img, int nw, int nh);

#ifdef __cplusplus
}
#endif

#endif
