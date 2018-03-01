/**********************************************************************\
 * ext4-browser
 * stdafx.h
 *
 * precompiled header file
 **********************************************************************
 * Copyright (C) 2017 - Max Maisel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
\**********************************************************************/
#pragma once

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

// C header files
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
	// reduces the size of the Win32 header files
	// by excluding some of the less frequently used APIs
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <tchar.h>
	#include <shellapi.h>

	#include <initguid.h>
	#include <wbemidl.h>
	#include <winioctl.h>

	#if defined(NOERROR)
	// Rename nasty define
	#define WINDOWS_NOERROR NOERROR
	#undef NOERROR
	#endif
#elif defined(__linux) || defined(__APPLE__)
	typedef uint8_t  BYTE;
	typedef uint16_t WORD;
	typedef uint32_t DWORD;
	typedef uint64_t QWORD;

	typedef uint16_t USHORT;
	typedef uint32_t ULONG;

	typedef uint64_t ULONG64;
#else
	#error Compiling to unsupportet OS!
#endif

#ifdef __APPLE__
    #include <libproc.h>
	#include <CoreFoundation/CoreFoundation.h>
	#include <IOKit/IOKitLib.h>
	#include <IOKit/IOKitKeys.h>
	#include <IOKit/IOBSD.h>
	#include <IOKit/usb/IOUSBLib.h>
	#include <IOKit/storage/IOMedia.h>
    #include <Security/Security.h>
    #include <ServiceManagement/ServiceManagement.h>
#endif

#ifdef __linux
#warning Linux support is for testing of generic features only! \
	Use native tools for image writing and ext4.
#endif
// wxWidgets header files:
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>
#include <wx/treelist.h>
#include <wx/file.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

// ext4 header files
extern "C"
{
	#include "ext4.h"
	#include "ext4_mbr.h"
	#include "ext4_mkfs.h"
}

// C++ header files:
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <stack>
#include <thread>
