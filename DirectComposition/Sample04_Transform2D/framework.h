// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

#include <dcomp.h>      // DirectComposition Header Files
#include <d3d11.h>      // Direct3D Header Files

#include <wrl.h>
using namespace Microsoft::WRL;

#include "resource.h"
