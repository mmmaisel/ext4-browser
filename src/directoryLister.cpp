/**********************************************************************\
 * ext4-browser
 * directoryLister.cpp
 *
 * wxDirTraverser directory lister
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
#include "stdafx.h"
#include "directoryLister.h"

wxDirTraverseResult DirectoryLister::OnFile(const wxString& filename)
{
	// Replace all windows \ path separators with ext4 /
#if defined(_WIN32) || defined(_WIN64)
	wxString _filename = filename;
	_filename.Replace(L"\\", L"/", true);
	mpFileList->push_back(ls_ent(_filename, false));
#else
	mpFileList->push_back(ls_ent(filename, false));
#endif
	return wxDIR_CONTINUE;
}

wxDirTraverseResult DirectoryLister::OnDir(const wxString& dirname)
{
	// Replace all windows \ path separators with ext4 /
#if defined(_WIN32) || defined(_WIN64)
	wxString _dirname = dirname;
	_dirname.Replace(L"\\", L"/", true);
	mpFileList->push_back(ls_ent(_dirname, true));
#else
	mpFileList->push_back(ls_ent(dirname, true));
#endif
	return wxDIR_CONTINUE;
}

wxDirTraverseResult DirectoryLister::OnOpenError(const wxString& dirname)
{
	return wxDIR_CONTINUE;
}
