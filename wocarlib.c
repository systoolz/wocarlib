#include "wocarlib.h"
#include "dlloader.h"

static CCHAR stWOCAR[] =
  "OCRDLL.DLL\0"
  "AddDIB\0"
  "AddImage\0"
  "CreateMultipleImg\0"
  "DIBToIMG\0"
  "DelImage\0"
  "DeskewImg\0"
  "EraseBlackBordersImg\0"
  "ExtractImgArea\0"
  "FreeImg\0"
  "FreeMultipleImg\0"
  "GetImage\0"
  "GetImgBitmap\0"
  "GetImgBitmapInfo\0"
  "GetImgBitmapSize\0"
  "GetImgRes\0"
  "GetImgSize\0"
  "GetNbImages\0"
  "HalfSizeImg\0"
  "InsertImage\0"
  "InvertImg\0"
  "LoadImg\0"
  "LoadMultipleImg\0"
  "OCR\0"
  "OCROnArea\0"
  "OCROnArea2\0"
  "OCRSetErrorHandler\0"
  "OCRSetOutputHandler\0"
  "OCRSetProgressHandler\0"
  "OCRSetWarningHandler\0"
  "ReplaceImage\0"
  "ResizeImg\0"
  "RotateImg\0"
  "SaveImg\0"
  "SaveMultipleImg\0"
  "ScanAndAddImage\0"
  "ScanAutoBright\0"
  "ScanAvailable\0"
  "ScanBrightness\0"
  "ScanEnd\0"
  "ScanImg\0"
  "ScanInit\0"
  "ScanResolution\0"
  "ScanSelect\0"
  "ScanShowUI\0"
  "ScanThreshold\0"
  "SetLanguage\0"
  "SetOutputMode\0"
  "SetOutputWindow\0"
  "ShrinkImg\0";

static DWORD hacklist[][3] = {
  /* v2.4 [1997-10-30 15:41:29 GMT]
     named as v2.5 in documentation */
  {0x3458AAA9, 0x020B6C, 0x096040},
  /* v2.6 [2002-01-07 18:04:19 GMT]
     recommended version */
  {0x3C39E323, 0x01FF0C, 0x094800},
  /* v2.6 [2002-03-19 01:17:34 GMT]
     debug version, big and unoptimized */
  {0x3C9691AE, 0x0354E0, 0x11D800},
};

void MemPatch(BYTE *p, BYTE b) {
DWORD dw;
  if (VirtualProtect(p, 1, PAGE_EXECUTE_READWRITE, &dw)) {
    *p = b;
    VirtualProtect(p, 1, dw, &dw);
  }
}

WOCARPTR *LoadWOCAR(int state) {
WOCARPTR *wp;
BYTE *p;
DWORD i;
  wp = (WOCARPTR *) LoadDLLFile(stWOCAR, sizeof(wp[0]));
  if (wp) {
    /* hacks - undocumented and unsupported */
    if (state) {
      p = (BYTE *) wp->hLibrary;
      /* test against known library versions */
      for (i = 0; i < (sizeof(hacklist) / sizeof(hacklist[0])); i++) {
        /* check library version through DateTimeStamp */
        if (*((DWORD *)&p[*((LONG *)&p[0x3C]) + 8]) == hacklist[i][0]) {
          /* suppress debug console output */
          if ((state == WOCAR_DEBUG_NONE) && (p[hacklist[i][1]] == 0x55)) {
            MemPatch(&p[hacklist[i][1]], 0xC3);
          }
          /* allow excessive debug console output and intermediate
             "area%d-%d.tif" image files creation */
          if ((state == WOCAR_DEBUG_MORE) && (p[hacklist[i][2]] == 0x00)) {
            MemPatch(&p[hacklist[i][2]], 0x01);
          }
          /* only one match */
          break;
        }
      }
    }
  }
  return(wp);
}

void FreeWOCAR(WOCARPTR *wp) {
  FreeDLLFile(wp, sizeof(wp[0]));
}
