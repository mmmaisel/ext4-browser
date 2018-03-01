/**********************************************************************\
 * ext4-browser
 * main.cpp
 *
 * Program Entry Point
 **********************************************************************
 * Copyright (C) 2017 - Max Maisel
 * Copyright 2016 ROSA and Konstantin Vlasov <konstantin.vlasov@rosalab.ru>
 *
 * The following code is a derivative work of the code from ROSA
 * ImageWriter, which is licensed GPLv3. This code therefore is also
 * licensed under the terms of the GNU Public License, verison 3.
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
#include "stdafx.h"
#include "gui/mainframe.h"

// Define a new application type, each program should derive a class from wxApp
class Ext4BrowserMain
	: public wxApp
{
public:
	// override base class virtuals
	// ----------------------------

	// this one is called on application startup and is a good place for the app
	// initialization (doing it here and not in the ctor allows to have an error
	// return: if OnInit() returns false, the application terminates)
	virtual bool OnInit() override;
};

bool Ext4BrowserMain::OnInit()
{
#ifdef __APPLE__
	// Initialize relative paths in MacOSX
	{
		char appdir[1024];
		pid_t pid;

		pid = getpid();
		proc_pidpath(pid, appdir, 1024);
		char* lastSlash = strrchr(appdir, '/');
		*lastSlash = 0;
		chdir(appdir);
	}
#endif

#if defined(WIN32) || defined(WIN64)
	HRESULT hr;
	// CoInitialize() seems to be called by wxWidgets automatically,
	// so only set security attributes
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, 0);
	if(hr != S_OK)
	{
		printf("CoInitializeSecurity failed! (Code: 0x%08lx)\n", hr);
		CoUninitialize();
		return false;
	}
#endif

	mainframe* frame = new mainframe();
	SetTopWindow(frame);
	frame->Show(true);
	return true;
}

// program entry point (main())
IMPLEMENT_APP(Ext4BrowserMain)
