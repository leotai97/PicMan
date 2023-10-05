
#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _USE_MATH_DEFINES
// Windows Header Files

#include <windows.h>
#include <windowsx.h>

#include <bcrypt.h> // encyrpt decrpyt

// C RunTime Header Files

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <comdef.h>
#include <time.h>
#include <stdio.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <Gdipluspixelformats.h>
#include <atlbase.h>
#include <shlwapi.h>
#include <commctrl.h> 
#include <math.h>
#include <dwmapi.h> // dark mode

// odbc

#include <sql.h>      
#include <sqlext.h>

#include <shellapi.h>

using namespace Gdiplus;

#pragma comment (lib,"Gdiplus.lib")

// STL

#include<stdexcept>
#include<string>
#include<sstream>
#include<map>
#include<stack>
#include<vector>
#include<algorithm>
#include<cmath>
#include<thread>
#include<mutex>
#include <fstream>
#include <iostream>
#include<functional>
#include<chrono>
#include<iterator>
#include <random>
