/**********************************************************************\
 * ext4-browser
 * archive.cpp
 *
 * Archive interface
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
#include "archive.h"

void Archive::RunFileExistsStateMachine
	(const wxString& srcpath, const wxString& dstpath, bool exists, bool isDir)
{
	// Clear all non-persistent bits
	mState &= ~(MERGE | SKIP | OVERWRITE);

	if(mState & SKIP_DIR)
	{
		if(srcpath.StartsWith(mSkipDirname))
			mState |= SKIP;
		else
			mState &= ~SKIP_DIR;
	}

	if(exists && !(mState & SKIP))
	{
		if(isDir)
		{
			if(mState & MERGE_ALL)
				mState |= MERGE;
			else if(mState & SKIP_DIR_ALL)
				mState |= SKIP_DIR;
			else
				mState |= mpInterface->OnFileExists(dstpath, isDir);
		}
		else
		{
			if(mState & OVERWRITE_ALL)
				mState |= OVERWRITE;
			else if(mState & SKIP_ALL)
				mState |= SKIP;
			else
				mState |= mpInterface->OnFileExists(dstpath, isDir);
		}
	}
	else if(!(mState & SKIP))
	{
		if(isDir)
			mState |= MERGE;
		else
			mState |= OVERWRITE;
	}

	// Update skipDirname
	if(isDir && (mState & SKIP_DIR))
	{
		mSkipDirname = srcpath;
	}
}
