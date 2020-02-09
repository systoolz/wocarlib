#include <setjmp.h>
#ifdef __HAVE_BUILTIN_SETJMP__
#undef setjmp
#define setjmp __builtin_setjmp
#undef longjmp
#define longjmp __builtin_longjmp
#endif

#include <stdio.h>

#ifdef TINYFILE
#include "tinyfile.h"
#endif

#include "wocarlib.h"
#include "wgdiplus.h"

/*
void SaveToBMP(TCHAR *filename, IMG *r, WOCARPTR *wp) {
BITMAPFILEHEADER fh;
BITMAPINFO *bi;
unsigned char *p;
HANDLE fl;
DWORD dw;
int sz;
  if (filename && r && wp) {
    sz = wp->GetImgBitmapSize(r);
    bi = wp->GetImgBitmapInfo(r);
    p = wp->GetImgBitmap(r);
    if (sz && bi && p) {
      fl = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
      if (fl != INVALID_HANDLE_VALUE) {
        ZeroMemory(&fh, sizeof(fh));
        fh.bfType = 0x4D42;
        fh.bfOffBits = sizeof(fh) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 2);
        fh.bfSize = fh.bfOffBits + sz;
        WriteFile(fl, &fh, sizeof(fh), &dw, NULL);
        WriteFile(fl, bi, fh.bfOffBits - sizeof(fh), &dw, NULL);
        WriteFile(fl, p, sz, &dw, NULL);
        CloseHandle(fl);
      }
    }
  }
}
*/

HBITMAP LoadImageWithWOCAR(WOCARPTR *wp, TCHAR *filename) {
#ifdef UNICODE
CCHAR buf[MAX_PATH + 1];
#endif
unsigned char *p;
BITMAPINFO *bi;
HBITMAP hbmp;
HDC dc;
IMG *r;
  hbmp = 0;
  /* sanity check */
  if (wp && filename) {
    #ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, filename, -1, buf, MAX_PATH, NULL, NULL);
    buf[MAX_PATH] = 0;
    filename = (TCHAR *) buf;
    #endif
    /* load image */
    r = wp->LoadImg((CCHAR *) filename);
    if (r) {
      /* get image info */
      bi = wp->GetImgBitmapInfo(r);
      p = wp->GetImgBitmap(r);
      /* convert to HBITMAP */
      if (bi && p) {
        /* do not use CreateBitmap() here since it's expected top-bottom pixel data */
        dc = GetDC(0);
        if (dc) {
          hbmp = CreateDIBitmap(dc, &bi->bmiHeader, CBM_INIT, p, bi, DIB_RGB_COLORS);
          ReleaseDC(0, dc);
        }
      }
      wp->FreeImg(r);
    }
  }
  return(hbmp);
}

HBITMAP LoadImageWithGDIPlus(TCHAR *filename) {
HBITMAP hbmp;
void *bitmap;
GDPLSPTR *gp;
#ifndef UNICODE
WCHAR buf[MAX_PATH + 1];
#endif
  hbmp = 0;
  /* sanity check */
  if (filename) {
    #ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, filename, -1, buf, MAX_PATH);
    buf[MAX_PATH] = 0;
    filename = (TCHAR *) buf;
    #endif
    /* GDI+ available */
    gp = LoadGDIPlus();
    if (gp) {
      /* image loaded */
      if (!gp->GdipLoadImageFromFile((WCHAR *) filename, &bitmap)) {
        /* create HBITMAP */
        gp->GdipCreateHBITMAPFromBitmap(bitmap, &hbmp, 0);
        /* delete GDI+ bitmap */
        gp->GdipDisposeImage(bitmap);
      }
      FreeGDIPlus(gp);
    }
  }
  return(hbmp);
}

/*
  https://docs.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
  https://docs.microsoft.com/en-us/windows/win32/gdi/storing-an-image

  Image loading sequence goes as follows:
  1) Load image from file:
    - with WinAPI as simply Windows .BMP (uncompressed)
    - with GDI+ as compressed .BMP, .PNG, .JPG, .TIF, etc.
    - with WOCAR as .TIF if none of above work and convert IMG to HBITMAP
  2) Resize image.
  3) Convert to monochrome.
  4) Convert HBITMAP to IMG type with DIBToIMG().
  5) Invert image with InvertImg().
*/
IMG *LoadMonoImage(WOCARPTR *wp, TCHAR *filename, BYTE zooma, BYTE zoomb, BYTE invert) {
BITMAPINFOHEADER bi;
HBITMAP hbmp, htmp;
HGLOBAL hg;
BITMAP bd;
LONG w, h;
BYTE *p;
HDC dc;
IMG *r;
  r = NULL;
  /* sanity check */
  if (wp && filename) {
    /* try to load as simple .BMP file with WinAPI */
    hbmp = LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    /* try to load with GDI+ if available (.BMP, .GIF / .PNG / .JPG, .TIF) */
    if (!hbmp) { hbmp = LoadImageWithGDIPlus(filename); }
    /* try to load as .TIF image with WOCAR */
    if (!hbmp) { hbmp = LoadImageWithWOCAR(wp, filename); }
    /* at least got something */
    if (hbmp) {
      /* get bitmap info */
      ZeroMemory(&bd, sizeof(bd));
      if (GetObject(hbmp, sizeof(bd), &bd) == sizeof(bd)) {
        /* zoom factor must be at least 1 */
        zooma = zooma ? zooma : 1;
        zoomb = zoomb ? zoomb : 1;
        /* resize image if needed */
        if (zooma != zoomb) {
          w = (bd.bmWidth * zooma) / zoomb;
          h = (bd.bmHeight * zooma) / zoomb;
          /* valid new size */
          if (w && h) {
            htmp = CopyImage(hbmp, IMAGE_BITMAP, w, h, 0);
            /* done */
            if (htmp) {
              DeleteObject(hbmp);
              hbmp = htmp;
              bd.bmWidth = w;
              bd.bmHeight = h;
            }
          }
        }
        /* convert to monochrome */
        if (bd.bmBitsPixel != 1) {
          htmp = CopyImage(hbmp, IMAGE_BITMAP, bd.bmWidth, bd.bmHeight, LR_MONOCHROME);
          /* done */
          if (htmp) {
            DeleteObject(hbmp);
            hbmp = htmp;
            bd.bmBitsPixel = 1;
          }
        }
        /* already monochrome or converted to one */
        if (bd.bmBitsPixel == 1) {
          /* get dc for GetDIBits() */
          dc = GetDC(0);
          if (dc) {
            /*
              Important notes according to MSDN:
              - biSize must be initalized or function fails
                since it's the only way for it to tell the difference
                between BITMAPINFOHEADER and BITMAPCOREHEADER
              - if lpvBits is NULL and biBitCount is 0 only
                BITMAPINFOHEADER will be filled without color table
            */
            ZeroMemory(&bi, sizeof(bi));
            bi.biSize = sizeof(bi);
            if (
              (GetDIBits(dc, hbmp, 0, 0, NULL, (BITMAPINFO *) &bi, DIB_RGB_COLORS)) &&
              (bi.biSizeImage) && (bi.biBitCount == 1) && (!bi.biCompression)
            ) {
              /* header size: info + palette
                 do not use biClrUsed / biClrImportant instead of 2
                 since it's zero on Windows 9x */
              bd.bmWidthBytes = sizeof(bi) + (sizeof(RGBQUAD) * 2);
              /* allocate global memory block required by DIBToIMG() */
              hg = GlobalAlloc(GHND, bd.bmWidthBytes + bi.biSizeImage);
              if (hg) {
                p = (BYTE *) GlobalLock(hg);
                if (p) {
                  /* PATCH: set correct DPI to avoid error with WOCAR v2.4 */
                  if (bi.biXPelsPerMeter < 20) { bi.biXPelsPerMeter = 20; }
                  if (bi.biYPelsPerMeter < 20) { bi.biYPelsPerMeter = 20; }
                  /* init BITMAPINFOHEADER structure */
                  CopyMemory(p, &bi, sizeof(bi));
                  /* copy bitmap data */
                  GetDIBits(dc, hbmp, 0, bi.biHeight, &p[bd.bmWidthBytes], (BITMAPINFO *) p, DIB_RGB_COLORS);
                  GlobalUnlock(p);
                  /* finally */
                  r = wp->DIBToIMG(hg);
                  /* invert if required */
                  if (r && invert) {
                    wp->InvertImg(r);
                  }
                }
                /* release global handle */
                GlobalFree(hg);
              }
            }
            /* release device context */
            ReleaseDC(0, dc);
          }
        }
      }
      /* delete bitmap */
      DeleteObject(hbmp);
    }
  }
  return(r);
}

static jmp_buf jmpbuf;

void MyErrorHandler(char *mod, char *fmt, va_list args) {
char s[1025];
  if (mod) { printf("%s: ", mod); }
  wvsprintfA(s, fmt, args);
  printf("%s.\n", s);
  /* try to recover from fatal error */
  longjmp(jmpbuf, 1);
}

void MyWarningHandler(char *mod, int code, char *fmt, va_list args) {
char s[1025];
  switch (code) {
    case 1:
      printf("**** ");
      break;
    case 2:
      printf("*** ");
      break;
    case 3:
      printf("** ");
      break;
    default:
      printf("* ");
      break;
  }
  if (mod) { printf("%s: ", mod); }
  wvsprintfA(s, fmt, args);
  printf("%s.\n", s);
}

static char bFmt;

void MyOutputHandler(int event, int param) {
  if (event == OT_TEXT) {
    printf("%c", LOBYTE(param));
  } else {
    if (bFmt) {
      /* FIXME: rework to HTML output? autosave OT_BITM to files? */
      printf("[%X|%X]", event, param);
    }
    if (event == OT_ENDL) { printf("\n"); }
    if (event == OT_ENDZ) { printf("\n\n"); }
  }
}

DWORD ParseInt(char *s) {
DWORD r;
  r = 0;
  if (s) {
    for (; *s; s++) {
      if ((*s < '0') || (*s > '9')) {
        break;
      }
      r *= 10;
      r += *s - '0';
    }
  }
  return(r);
}

int main(int argc, char *argv[]) {
WOCARPTR *wp;
IMG *p;
BYTE d, m, v;
int i;
  printf("WOCAR Engine test application v1.0\n(c) SysTools 2020\nhttp://systools.losthost.org/?misc\n\n");
  if (argc < 2) {
    printf(
      "Usage: tstwocar <filename.ext> [/m#] [/d#] [/r] [/i]\n\n"
      "Where:\n"
      "  filename.ext - input image file name\n"
      "    supported input image formats:\n"
      "    - .BMP (uncompressed, any color)\n"
      "    - .TIF (only black/white)\n"
      "    - any .BMP / .PNG / .GIF / .JPG / .TIFF (GDI+ required)\n"
      "  /m# - zoon in image by # times (optional, default 1)\n"
      "  /d# - zoon out image by # times (optional, default 1)\n"
      "  /r - output as text with rich tags (optional, default text only)\n"
      "  /i - invert image colors - black text must be on white background (optional)\n"
      "\n"
      "Example: /m3 /d2 will multiply image width and height by 1.5 (3/2).\n"
      "I.e. image 400x200 with /m3 /d2 will be ((400*3)/2)x((200*3)/2) = 600x300.\n"
      "For images or screenshots with tiny text use at least /m3 (zoom by 3).\n"
      "\n"
    );
    return(1);
  }
  /* parse commandline */
  m = 0;
  d = 0;
  v = 0;
  bFmt = 0;
  for (i = 2; i < argc; i++) {
    if ((argv[i][0] == '/') && argv[i][1]) {
      if (argv[i][2]) {
        if ((argv[i][1] | 0x20) == 'm') { m = ParseInt(&argv[i][2]); continue; }
        if ((argv[i][1] | 0x20) == 'd') { d = ParseInt(&argv[i][2]); continue; }
      } else {
        if ((argv[i][1] | 0x20) == 'r') { bFmt = 1; continue; }
        if ((argv[i][1] | 0x20) == 'i') { v = 1; continue; }
      }
    }
    printf("Error: invalid or unknown argument: %s\n\n", argv[i]);
    return(2);
  }
  /* load library */
  wp = LoadWOCAR(WOCAR_DEBUG_NONE); /* FIXME: allow to set state from commandline? */
  if (wp) {
    wp->OCRSetErrorHandler(MyErrorHandler);
    wp->OCRSetWarningHandler(MyWarningHandler);
    printf("+ WOCAR Engine initialized\n");
    p = NULL;
    if (!setjmp(jmpbuf)) {
      p = LoadMonoImage(wp, argv[1], m, d, v);
      printf("+ image file loaded\n");
    } else {
      printf("- error loading input image file\n");
    }
    if (p) {
      if (!setjmp(jmpbuf)) {
        wp->SetLanguage(ENGLISH, "."); /* FIXME: allow to set language from commandline? */
        printf("+ English language and dictionary loaded\n");
      } else {
        printf("- error loading English dictionary\n");
      }
      wp->SetOutputMode(OM_RICHTEXT);
      wp->OCRSetOutputHandler(MyOutputHandler);
      printf("\n");
      if (!setjmp(jmpbuf)) {
        if (!wp->OCR(p, 1)) {
          printf("+ OCR completed\n");
        } else {
          printf("- error in OCR\n");
        }
      } else {
        printf("- fatal error in OCR\n");
      }
      wp->FreeImg(p);
    }
    FreeWOCAR(wp);
  } else {
    printf("- error loading OCRDLL.DLL file\n");
  }
  printf("* exiting\n");
  return(0);
}
