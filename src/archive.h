/**********************************************************************\
 * ext4-browser
 * archive.h
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
#pragma once
#include "backendTask.h"

struct ls_ent
{
	inline ls_ent(wxString _name, bool _isDir, size_t _size = 0)
		: name(_name), isDir(_isDir), size(_size) {}
	wxString name;
	bool isDir;
	size_t size;

	inline bool operator< (const ls_ent& rhs) const
	{
		return name.compare(rhs.name) < 0;
	}
};

class Archive
{
	public:
		Archive() : mpInterface(0), mState(0) {}
		virtual ~Archive() {}

		enum eError
		{
			NOERROR,
			ERR_CANCELED,
			ERR_ALLOC,
			ERR_ALREADY_OPEN,
			ERR_MALLOC,
			ERR_FILE_NOT_FOUND,
			ERR_MKFS,
			ERR_MBR_SCAN,
			ERR_REG,
			ERR_UREG,
			ERR_MOUNT,
			ERR_UMOUNT,
			ERR_NO_PARTITION,
			ERR_RECOVER,
			ERR_JOURNAL_START,
			ERR_JOURNAL_STOP,
			ERR_CACHE_WRITEBACK,
			ERR_FOPEN,
			ERR_DIR_OPEN,
			ERR_DIR_CLOSE,
			ERR_EMPTY_PATH,
			ERR_MKDIR,
			ERR_READ,
			ERR_WRITE,
			ERR_MOVE,
			ERR_DELETE
		};

		enum eFileExists
		{
			SKIP			= (1 << 0),
			OVERWRITE		= (1 << 1),
			MERGE			= (1 << 2),
			SKIP_DIR		= (1 << 3),
			SKIP_ALL		= (1 << 8),
			OVERWRITE_ALL	= (1 << 9),
			MERGE_ALL		= (1 << 10),
			SKIP_DIR_ALL	= (1 << 11),

			ANSWER_SKIP			= SKIP,
			ANSWER_SKIP_ALL		= SKIP | SKIP_ALL,
			ANSWER_OVERWRITE	= OVERWRITE,
			ANSWER_OVERWRITE_ALL= OVERWRITE | OVERWRITE_ALL,
			ANSWER_SKIP_DIR		= SKIP_DIR,
			ANSWER_SKIP_DIR_ALL	= SKIP_DIR | SKIP_DIR_ALL,
			ANSWER_MERGE		= MERGE,
			ANSWER_MERGE_ALL	= MERGE | MERGE_ALL,

			// Errors:
			ANSWER_CONTINUE	= (1 << 16),
			ANSWER_ABORT	= (1 << 17)
		};

		virtual wxString GetRootName() const = 0;
		virtual int GetPartitionCount() const = 0;
		virtual bool IsValidPath(wxString path) const = 0;
		virtual bool IsRootPath(wxString path) const = 0;

		virtual int ListDirectory
			(wxString path, std::list<ls_ent>& ls, bool recursive) = 0;
		virtual int ExtractFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) = 0;
		virtual int AddFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) = 0;
		virtual int CopyFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) = 0;
		virtual int MoveFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) = 0;
		virtual int DeleteFiles(std::list<ls_ent>& ls) = 0;
		virtual int MakeDirectory(wxString path, bool recursive) = 0;

		inline void SetInterface(IBackendTask* pInterface)
			{ mpInterface = pInterface; }

	protected:
		IBackendTask* mpInterface;

		wxString mSkipDirname;
		USHORT mState;

		void RunFileExistsStateMachine
			(const wxString& srcpath, const wxString& dstpath, bool exists, bool isDir);
};

