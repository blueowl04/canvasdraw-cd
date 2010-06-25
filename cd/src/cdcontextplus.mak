PROJNAME = cd
LIBNAME = cdcontextplus
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE


ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRCDIR = gdiplus
  SRC = cdwemfp.cpp cdwimgp.cpp cdwinp.cpp cdwnativep.cpp cdwprnp.cpp cdwdbufp.cpp cdwclpp.cpp cdwgdiplus.c

  INCLUDES = . gdiplus drv
  LIBS = gdiplus
  CHECK_GDIPLUS = Yes
else
  SRC = xrender/cdxrender.c xrender/cdxrplus.c

  LIBS = Xrender Xft
  USE_X11 = Yes
  CHECK_XRENDER = Yes

  INCLUDES = . sim drv freetype2 x11
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  BUILD_DYLIB=Yes
endif

USE_CD = YES
CD = ..
