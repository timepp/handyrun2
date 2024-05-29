#pragma once

#include "compilerconf.h"

MSVC_NO_WARNING_AREA_BEGIN

// system
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>

// ATL/WTL
#include <atlbase.h>
#include <atlwin.h>
#include <WTL/atlapp.h>
#include <WTL/atlmisc.h>
#include <WTL/atldlgs.h>
#include <WTL/atlctrls.h>
#include <WTL/atlctrlx.h>
#include <WTL/atlframe.h>
#include <WTL/atlgdi.h>

// STL
#include <vector>
#include <string>

// others
#include <tplib/auto_release.h>
#include <tplib/format_shim.h>

MSVC_NO_WARNING_AREA_END
MSVC_TUNE_WARNINGS
