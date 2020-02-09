#ifndef __WGDIPLUS_H
#define __WGDIPLUS_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)
typedef struct {
  UINT32 GdiplusVersion;
  LPVOID DebugEventCallback;
  BOOL   SuppressBackgroundThread;
  BOOL   SuppressExternalCodecs;
} GdiplusStartupInput;
#pragma pack(pop)

#define GdiplusStartupOutput VOID

typedef DWORD (WINAPI *GDIPLUSSTARTUPPROC)(ULONG_PTR*, GdiplusStartupInput*, GdiplusStartupOutput*);
typedef DWORD (WINAPI *GDIPLOADIMAGEFROMFILEPROC)(WCHAR*, void**);
typedef DWORD (WINAPI *GDIPCREATEHBITMAPFROMBITMAPPROC)(void*, HBITMAP*, DWORD);
typedef DWORD (WINAPI *GDIPDISPOSEIMAGEPROC)(void *);
typedef VOID  (WINAPI *GDIPLUSSHUTDOWNPROC)(ULONG_PTR);

#pragma pack(push, 1)
typedef struct {
  HINSTANCE hLibrary;
  GDIPLUSSTARTUPPROC GdiplusStartup;
  GDIPLOADIMAGEFROMFILEPROC GdipLoadImageFromFile;
  GDIPCREATEHBITMAPFROMBITMAPPROC GdipCreateHBITMAPFromBitmap;
  GDIPDISPOSEIMAGEPROC GdipDisposeImage;
  GDIPLUSSHUTDOWNPROC GdiplusShutdown;
  ULONG_PTR gdiplusToken;
  GdiplusStartupInput gdiplusInfo;
} GDPLSPTR;
#pragma pack(pop)

GDPLSPTR *LoadGDIPlus(void);
void FreeGDIPlus(GDPLSPTR *gp);

#ifdef __cplusplus
}
#endif

#endif
