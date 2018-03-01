/**********************************************************************\
 * ext4-browser
 * elevation.cpp
 *
 * Helper functions for process elevation (admin).
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
#include "elevation.h"

bool IsSelfElevated()
{
#if defined(WIN32) || defined(WIN64)
	// From https://stackoverflow.com/questions/8046097/
	// how-to-check-if-a-process-has-the-administrative-rights
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
	{
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if(GetTokenInformation( hToken, TokenElevation, &Elevation,
			sizeof(Elevation), &cbSize))
		{
			fRet = Elevation.TokenIsElevated;
		}
	}
	if(hToken)
		CloseHandle(hToken);
	return fRet;
#elif defined(__APPLE__)
	uid_t uid = getuid();
	uid_t euid = geteuid();
	if ((uid == 0) || (euid == 0))
		return true;
	return false;
#elif defined(__linux)
	uid_t uid = getuid();
	if (uid == 0)
		return true;
	return false;
#endif
}

bool StartElevated(wxString cmd)
{
#if defined(WIN32) || defined(WIN64)
	if((int)ShellExecute(NULL, L"runas", cmd.ToStdWstring().c_str(),
		NULL, NULL, SW_NORMAL) > 32)
	{
		return true;
	}
#elif defined(__APPLE__)
	char* const args[1] = {0};
	AuthorizationItem authItem = { kSMRightModifySystemDaemons, 0, NULL, 0 };
	AuthorizationRights authRights = { 1, &authItem };
	AuthorizationFlags flags = kAuthorizationFlagDefaults
		| kAuthorizationFlagInteractionAllowed | kAuthorizationFlagPreAuthorize
		| kAuthorizationFlagExtendRights;

	AuthorizationRef authRef = NULL;

	if(AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment,
		flags, &authRef) != errAuthorizationSuccess)
	{
		return false;
	}

	if(AuthorizationExecuteWithPrivileges(authRef, cmd.ToStdString().c_str(),
		kAuthorizationFlagDefaults, args, NULL) == errAuthorizationSuccess)
	{
		return true;
	}
#elif defined(__linux)
	wxMessageBox("Not supported not Linux yet");
#endif
	return false;
}
