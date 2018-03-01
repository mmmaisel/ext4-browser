/**********************************************************************\
 * ext4-browser
 * ext4ImgArchive.h
 *
 * ext4 image file archive implementation
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

#include "blockdev.h"
#include "archive.h"

class Ext4ImgFileArchive
	: public Archive
{
	public:
		Ext4ImgFileArchive();
		Ext4ImgFileArchive(const Ext4ImgFileArchive&) = delete;
		Ext4ImgFileArchive(Ext4ImgFileArchive&&) = delete;
		virtual ~Ext4ImgFileArchive();

		int Open(wxString& path);
		int OpenPhysical(PhysicalDevice* pDev);
		int Create(wxString& path, int64_t size_MiB, BYTE type);

		static const int   DEVNAME_LEN;
		static const char* DEVNAME;
		static const int   ROOTNAME_LEN;
		static const char* ROOTNAME;
		static const int   MNTNAME_LEN;
		static const char* MNTNAME;
		static const int   LOST_FOUND_LEN;
		static const char* LOST_FOUND;

		inline wxString GetPath() const {return mPath;}
		inline wxString GetName() const {return mFriendlyName;}

		uint8_t Partitions() const;

		inline int64_t GetSize() const
			{return mpExt4Dev->bd.part_size;}
		inline int64_t GetPartitionSize(int i) const
			{return m_bdevs.partitions[i].part_size;}

		int Mount();
		int UMount();

		virtual wxString GetRootName() const override;
		virtual int GetPartitionCount() const override;
		virtual bool IsValidPath(wxString path) const override;
		virtual bool IsRootPath(wxString path) const override;

		virtual int ListDirectory
			(wxString path, std::list<ls_ent>& ls, bool recursive) override;
		virtual int ExtractFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) override;
		virtual int AddFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) override;
		virtual int CopyFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) override;
		virtual int MoveFiles
			(wxString path, std::list<ls_ent>& ls, int nPrefixLen) override;
		virtual int DeleteFiles(std::list<ls_ent>& ls) override;
		virtual int MakeDirectory(wxString path, bool recursive) override;

	private:
		wxString mPath;
		wxString mFriendlyName;
		int64_t mSize_MiB;
		BYTE mType;

		blockdev* mpExt4Dev;
		ext4_mbr_bdevs m_bdevs;

		static const int COPY_BUFFER_SIZE = 1024*1024;
};
