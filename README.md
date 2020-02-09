# wocarlib
WOCARLib - WOCAR OCR Engine helper library

Build with GCC 3.2 (mingw special 20020817-1) from DEV-CPP 4.9.8.0 (2003).
Pure C and Win32API application, no GMO required
(like msvcrt.dll, .NET Framework, Visual Studio Redistributable and so on).
Tested under Windows 98, Windows XP, Windows 10, so should work anywhere.

Software homepage (you can download binary "tstwocar.exe" file here):

http://systools.losthost.org/?misc#wocarlib

WOCARLib licensed under the Apache License, Version 2.0 (the "License") - see LICENSE file.

Please note that WOCAR Engine itself is not a free and since 2002 acquired by SimpleOCR:

> Q: What does it mean "Wocar is free for any non commercial purpose"?
> A: Basically, it means you can use it at home freely and give it away to your friends. If you use it at work, I'll appreciate if your company can support Wocar (in this case, please contact me). In any case, you can't sell it and you can't make money with it, directly or indirectly.
> [WOCAR FAQ](https://web.archive.org/web/20040412193825/http://ccambien.free.fr/wocar/faq.html)
> Wocar is now SimpleOCR. For downloading SimpleOCR or if your are interested in SimpleOCR engine, the only OCR API that's Royalty Free, visit [SimpleOCR](https://www.simpleocr.com/).

WOCARLib requires OCRDLL.DLL and DLLTWAIN.DLL files from WOCAR to work.
You can also use ENGLISH.WDC, FRENCH.WDC and DUTCH.WDC (last one only in v2.6) to improve OCR result for selected languages.

**It's strongly recommended to read all the details in the "ocrdll.h" file.**

Please see a "tstwocar.c" file to understand how to work with this library and WOCAR Engine.

You can also build "ocrdll.a" file for static linking with the following command (GCC):
`dlltool -k -d ocrdll.def -l ocrdll.a`

Known WOCAR Engine library versions supported by WOCARLib:

- v2.4 [1997-10-30 15:41:29 GMT] named as v2.5 in documentation
  [Download](https://web.archive.org/web/20070324035310/http://ccambien.free.fr/wocar25.zip)

- v2.6 [2002-01-07 18:04:19 GMT] recommended version
  [Download](https://web.archive.org/web/20020228023446/http://www.simpleocr.com/Download/SimpleOCR.exe)
  Notable changes:
  - fixed float point zero division for completely black images and zero DPI
  - added DUTCH language support (but without DUTCH.WDC file)
  - smaller .DLL file size

- v2.6 [2002-03-19 01:17:34 GMT] debug version, big and unoptimized
  [Download](https://web.archive.org/web/20020402130839/http://www.simpleocr.com/Download/SimpleOCR.exe)
  Notable changes:
  - DUTCH.WDC file added, you can take it from this version and add to one above
  - larger .DLL file size, debug build, slow
  - latest version with OCRDLL.DLL and DLLTWAIN.DLL - API and .DLL files changed after

WOCAR manual in [Windows help file](https://web.archive.org/web/20051023121456/http://ccambien.free.fr/wocar/dlldoc25.zip).
Note that .HLP format support was dropped since Windows Vista without [KB917607](https://support.microsoft.com/en-us/help/917607/feature-not-included-help-not-supported-error-opening-help-windows).

WOCAR manual in [HTML file](https://web.archive.org/web/20041022005911/http://ccambien.free.fr:80/wocar/dlldoc.html) (broken markup).

Original WOCAR Engine copyright:

> WOCAR Engine v 2.5
> (c) 1997-1998 Cyril Cambien - All rights reserved.
> cambien(a)francenet(.)fr