#include "wgdiplus.h"
#include "dlloader.h"

/*
  https://stackoverflow.com/questions/27039199/using-gdi-in-c-gdiplusstartup-function-returning-2
  https://msdn.microsoft.com/en-us/library/windows/desktop/ms534041.aspx

  GdiPlusFlat.h
  https://msdn.microsoft.com/en-us/library/windows/desktop/ms533969.aspx

  https://stackoverflow.com/questions/16368575/how-to-save-an-image-to-bmp-png-jpg-with-gdi

  Supported formats (.BMP/.PNG/.GIF/.JPEG/.TIFF):
  https://msdn.microsoft.com/en-us/library/windows/desktop/ms630810.aspx

  ! http://www.cyberforum.ru/win-api/thread324439.html
*/

static CCHAR stGDIPlus[] =
  "GdiPlus.dll\0"
  "GdiplusStartup\0"
  "GdipLoadImageFromFile\0"
  "GdipCreateHBITMAPFromBitmap\0"
  "GdipDisposeImage\0"
  "GdiplusShutdown\0";

GDPLSPTR *LoadGDIPlus(void) {
GDPLSPTR *gp;
  gp = (GDPLSPTR *) LoadDLLFile(stGDIPlus, sizeof(gp[0]));
  if (gp) {
    /* gdiplusInfo and gdiplusToken already initialized with zeros in LoadDLLFile() */
    gp->gdiplusInfo.GdiplusVersion = 1;
    if (gp->GdiplusStartup(&gp->gdiplusToken, &gp->gdiplusInfo, NULL)) {
      /* non-zero return - startup error */
      FreeDLLFile(gp, sizeof(gp[0]));
      gp = NULL;
    }
  }
  return(gp);
}

void FreeGDIPlus(GDPLSPTR *gp) {
  if (gp) {
    gp->GdiplusShutdown(gp->gdiplusToken);
    FreeDLLFile(gp, sizeof(gp[0]));
  }
}
