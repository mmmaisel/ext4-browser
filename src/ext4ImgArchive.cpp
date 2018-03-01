/**********************************************************************\
 * ext4-browser
 * ext4ImgArchive.cpp
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
#include "stdafx.h"
#include "ext4ImgArchive.h"

const int   Ext4ImgFileArchive::DEVNAME_LEN = 8;
const char* Ext4ImgFileArchive::DEVNAME = "loop0/px";
const int   Ext4ImgFileArchive::ROOTNAME_LEN = 7;
const char* Ext4ImgFileArchive::ROOTNAME = "/loop0/";
const int   Ext4ImgFileArchive::MNTNAME_LEN = 10;
const char* Ext4ImgFileArchive::MNTNAME = "/loop0/px/";
const int   Ext4ImgFileArchive::LOST_FOUND_LEN = 21;
const char* Ext4ImgFileArchive::LOST_FOUND = "/loop0/px/lost+found/";

Ext4ImgFileArchive::Ext4ImgFileArchive()
	: mSize_MiB(0)
	, mType(0)
	, mpExt4Dev(0)
	, m_bdevs({0})
{
}

Ext4ImgFileArchive::~Ext4ImgFileArchive()
{
	UMount();
	if(mpExt4Dev)
	{
		blockdev_destroy(mpExt4Dev);
	}
}

int Ext4ImgFileArchive::Open(wxString& path)
{
	int res = 0;
	if(mpExt4Dev)
		return ERR_ALREADY_OPEN;

	mPath = path;
	mFriendlyName = wxFileName(mPath).GetFullName();
	mpExt4Dev = blockdev_create(mPath, BLOCKDEV_DEFAULT_BSIZE, 0);
	res = ext4_mbr_scan(&mpExt4Dev->bd, &m_bdevs);
	if(res != EOK)
		return ERR_MBR_SCAN;
	mSize_MiB = mpExt4Dev->bd.part_size;

	return NOERROR;
}

int Ext4ImgFileArchive::OpenPhysical(PhysicalDevice* pDev)
{
	int res = 0;
	if(mpExt4Dev)
		return ERR_ALREADY_OPEN;

	mPath = pDev->GetSystemName();
	mFriendlyName = pDev->GetFriendlyName();
	mpExt4Dev = blockdev_create(mPath, pDev->GetSectorSize(), pDev);
	res = ext4_mbr_scan(&mpExt4Dev->bd, &m_bdevs);
	if(res != EOK)
		return ERR_MBR_SCAN;
	mSize_MiB = mpExt4Dev->bd.part_size;

	return NOERROR;
}

int Ext4ImgFileArchive::Create(wxString& path, int64_t size_MiB, BYTE type)
{
	int res = 0;
	ext4_blockdev* bd = 0;
	ext4_blockdev* bd_root = 0;
	ext4_mbr_parts partitions = {0};
	ext4_fs extfs = {0};
	ext4_mkfs_info fsinfo = {0};

	if(mpExt4Dev)
		return ERR_ALREADY_OPEN;

	mPath = path;
	mFriendlyName = wxFileName(mPath).GetFullName();
	mSize_MiB = size_MiB;
	mType = type;
	mpExt4Dev = blockdev_create(mPath, BLOCKDEV_DEFAULT_BSIZE, 0);

	{
		// Create image file
		wxFile imgfile;
		BYTE data = 0;

		imgfile.Open(mPath, wxFile::write);
		if(!imgfile.IsOpened())
			return ERR_FILE_NOT_FOUND;

		imgfile.Seek(mSize_MiB * 1024 * 1024 - 1);
		imgfile.Write(&data, 1);
		imgfile.Close();
	}

#ifdef _DEBUG
	ext4_dmask_set(DEBUG_ALL);
#endif

	srand(time(0));
	bd_root = &mpExt4Dev->bd;

	partitions.division[0] = 100;
	partitions.division[1] = 0;
	partitions.division[2] = 0;
	partitions.division[3] = 0;

	ext4_mbr_write(bd_root, &partitions, rand());
	ext4_mbr_scan(bd_root, &m_bdevs);
	bd = &m_bdevs.partitions[0];

	for(BYTE i = 0; i < UUID_SIZE; ++i)
		fsinfo.uuid[i] = rand();
	fsinfo.journal = true;

	res = ext4_mkfs(&extfs, bd, &fsinfo, mType);
	if(res != EOK)
		return ERR_MKFS;

	return NOERROR;
}

uint8_t Ext4ImgFileArchive::Partitions() const
{
	uint8_t partitions = 0;

	for(BYTE i = 0; i < 4; ++i)
	{
		if(m_bdevs.partitions[i].part_size != 0)
			partitions |= (1 << i);
	}
	return partitions;
}

int Ext4ImgFileArchive::Mount()
{
	bool extPartitionFound = false;
	int res = 0;
	// +1: \0
	char devname[DEVNAME_LEN+1];
	char mntname[MNTNAME_LEN+1];

	strcpy(devname, DEVNAME);
	strcpy(mntname, MNTNAME);

	for(BYTE i = 0; i < 4; ++i)
	{
		if(m_bdevs.partitions[i].part_size != 0)
		{
			extPartitionFound = true;
			devname[DEVNAME_LEN-1] = '0' + i;
			mntname[MNTNAME_LEN-2] = '0' + i;
			res = ext4_device_register(&m_bdevs.partitions[i], devname);
			if(res != EOK)
				return ERR_REG;
			res = ext4_mount(devname, mntname, false);
			if(res != EOK)
				return ERR_MOUNT;
			res = ext4_recover(mntname);
			if(res != EOK)
				return ERR_RECOVER;
			res = ext4_journal_start(mntname);
			if(res != EOK)
				return ERR_JOURNAL_START;
		}
	}

	if(!extPartitionFound)
		return ERR_NO_PARTITION;
	return NOERROR;
}

int Ext4ImgFileArchive::UMount()
{
	int res = 0;
	// +1: \0
	char devname[DEVNAME_LEN+1];
	char mntname[MNTNAME_LEN+1];

	strcpy(devname, DEVNAME);
	strcpy(mntname, MNTNAME);

	for(BYTE i = 0; i < 4; ++i)
	{
		if(m_bdevs.partitions[i].part_size != 0)
		{
			devname[DEVNAME_LEN-1] = '0' + i;
			mntname[MNTNAME_LEN-2] = '0' + i;
			res = ext4_cache_write_back(mntname, 1);
			if(res == ENODEV)
				continue;
			if(res != EOK)
				return ERR_CACHE_WRITEBACK;
			res = ext4_journal_stop(mntname);
			if(res != EOK)
				return ERR_JOURNAL_STOP;
			res = ext4_umount(mntname);
			if(res != EOK)
				return ERR_UMOUNT;
			res = ext4_device_unregister(devname);
			if(res != EOK)
				return ERR_UREG;
		}
	}
	return NOERROR;
}

wxString Ext4ImgFileArchive::GetRootName() const
{
	return wxString(ROOTNAME);
}

int Ext4ImgFileArchive::GetPartitionCount() const
{
	int count = 0;

	for(BYTE i = 0; i < 4; ++i)
	{
		if(m_bdevs.partitions[i].part_size != 0)
			++count;
	}
	return count;
}

bool Ext4ImgFileArchive::IsValidPath(wxString path) const
{
	uint8_t partitions = Partitions();

	for(uint8_t i = 0; i < 4; ++i)
	{
		if(partitions & (1 << i))
		{
			wxString prefix = wxString::Format("%sp%d/", ROOTNAME, i);
			if(path.StartsWith(prefix))
				return true;
		}
	}
	return false;
}

bool Ext4ImgFileArchive::IsRootPath(wxString path) const
{
	uint8_t partitions = Partitions();

	for(uint8_t i = 0; i < 4; ++i)
	{
		if(partitions & (1 << i))
		{
			wxString partitionRoot = wxString::Format("%sp%d/", ROOTNAME, i);
			if(path == partitionRoot)
				return true;
		}
	}
	return false;
}

int Ext4ImgFileArchive::ListDirectory(wxString path, std::list<ls_ent>& ls, bool recursive)
{
	int res = 0;
	const ext4_direntry* direntry = 0;
	std::stack<ext4_dir> dirStack;
	dirStack.push({0});

	if(!IsValidPath(path))
		return ERR_EMPTY_PATH;

	res = ext4_dir_open(&dirStack.top(), path.ToUTF8().data());
	if(res != EOK)
	{
		ext4_file tmpfile = {0};
		res = ext4_fopen(&tmpfile, path.ToUTF8().data(), "r");
		if(res == EOK)
		{
			ls.push_back(ls_ent(path, false, tmpfile.fsize));
			ext4_fclose(&tmpfile);
			return NOERROR;	// Root is file -> finish here
		}
		else
		{
			return ERR_FILE_NOT_FOUND;
		}
	}

	direntry = ext4_dir_entry_next(&dirStack.top());

	while(dirStack.size())
	{
		direntry = ext4_dir_entry_next(&dirStack.top());
		if(!direntry)
		{
			res = ext4_dir_close(&dirStack.top());
			if(res != EOK)
				return ERR_DIR_CLOSE;
			dirStack.pop();

			// step out of directory, adjust paths
			path.RemoveLast();
			size_t pos = path.find_last_of(L'/');
			path = path.substr(0, pos+1); // keep trailing '/'
		}
		else
		{
			wxString fullpath;
			wxString name = wxString::FromUTF8((const char*)direntry->name);
			ext4_dir tmpdir = {0};
			ext4_file tmpfile = {0};
			fullpath = path + name;

			if(name == L".")
				; // Ignore self
			else if(name == L"..")
				ls.push_back(ls_ent(path, true)); // Add current dir
			else if(ext4_dir_open(&tmpdir, fullpath.ToUTF8().data()) == EOK)
			{
				ls.push_back(ls_ent(fullpath, true));
				if(recursive)
				{
					// step into directory, adjust paths
					dirStack.push(tmpdir);
					path.append(name);
					path.append(L'/');
				}
				else
				{
					ext4_dir_close(&tmpdir);
				}
			}
			else if(ext4_fopen(&tmpfile, fullpath.ToUTF8().data(), "r") == EOK)
			{
				ls.push_back(ls_ent(fullpath, false, tmpfile.fsize));
				ext4_fclose(&tmpfile);
			}
			else
			{
				return ERR_FOPEN;
			}
		}
	}
	return NOERROR;
}

int Ext4ImgFileArchive::ExtractFiles
	(wxString path, std::list<ls_ent>& ls, int nPrefixLen)
{
	int retval = NOERROR;
	int res = 0;
	wxString dstpath;
	ext4_file srcfile = {0};
	wxFile dstfile;
	BYTE* buffer = 0;
	size_t src_count = 0;
	size_t dst_count = 0;

	buffer = new BYTE[COPY_BUFFER_SIZE];
	if(!buffer)
	{
		retval = ERR_ALLOC;
		goto exit_error;
	}

	// Reset state variables
	mState = 0;
	mSkipDirname.clear();
	mpInterface->ResetCanceled();

	for(const auto& file : ls)
	{
		if(mpInterface->IsCanceled())
		{
			retval = ERR_CANCELED;
			goto exit_error;
		}
		dstpath = file.name;
		dstpath.Remove(0, nPrefixLen);
		dstpath.insert(0, path);

		RunFileExistsStateMachine
			(file.name, dstpath, wxFileName::Exists(dstpath), file.isDir);

		// Only set if file is directory
		if(mState & MERGE)
		{
			if(!wxFileName::Mkdir(dstpath, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
			{
				if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
					continue;
				else
				{
					retval = ERR_MKDIR;
					goto exit_error;
				}
			}
		}
		// Only set if file is normal file
		else if(mState & OVERWRITE)
		{
			res = ext4_fopen(&srcfile, file.name.ToUTF8().data(), "rb");
			if(res != EOK)
			{
				if(mpInterface->OnError(file.name) == ANSWER_CONTINUE)
					continue;
				else
				{
					retval = ERR_FOPEN;
					goto exit_error;
				}
			}

			dstfile.Open(dstpath, wxFile::write);
			if(!dstfile.IsOpened())
			{
				// Must use temporary wxFileName, otherwise
				// it creates a directory with filename
				wxFileName dirname = dstpath;
				if(!dirname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						continue;
					else
					{
						retval = ERR_MKDIR;
						goto exit_error;
					}
				}

				dstfile.Open(dstpath, wxFile::write);
				if(!dstfile.IsOpened())
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						continue;
					else
					{
						retval = ERR_FOPEN;
						goto exit_error;
					}
				}
			}

			do
			{
				res = ext4_fread(&srcfile, buffer, COPY_BUFFER_SIZE, &src_count);
				if(res != EOK)
				{
					if(mpInterface->OnError(file.name) == ANSWER_CONTINUE)
						break;
					else
					{
						retval = ERR_READ;
						dstfile.Close();
						ext4_fclose(&srcfile);
						goto exit_error;
					}
				}

				// If the image is read completely and image size is a multiple of
				// COPY_BUFFER_SIZE, src_count can get 0.
				// Write operation with size 0 cause problems on OSX
				// so break out of loop here if there is no remainder.
				if(src_count == 0)
					break;

				dst_count = dstfile.Write(buffer, src_count);

				if(dst_count != src_count)
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						break;
					else
					{
						retval = ERR_WRITE;
						dstfile.Close();
						ext4_fclose(&srcfile);
						goto exit_error;
					}
				}

				if(mpInterface->IsCanceled())
				{
					retval = ERR_CANCELED;
					dstfile.Close();
					ext4_fclose(&srcfile);
					goto exit_error;
				}
			}
			while(src_count >= COPY_BUFFER_SIZE);

			dstfile.Close();
			ext4_fclose(&srcfile);
		}

		if(mpInterface)
			mpInterface->OnProgress(1);
	}

exit_error:
	if(buffer)
		delete[] buffer;

	return retval;
}

int Ext4ImgFileArchive::AddFiles
	(wxString path, std::list<ls_ent>& ls, int nPrefixLen)
{
	int retval = NOERROR;
	int res = 0;
	wxString dstpath;
	ext4_file dstfile = {0};
	wxFile srcfile;
	BYTE* buffer = 0;
	ssize_t src_count = 0;
	size_t dst_count = 0;
	uint32_t dummy;

	buffer = new BYTE[COPY_BUFFER_SIZE];
	if(!buffer)
	{
		retval = ERR_ALLOC;
		goto exit_error;
	}

	// Reset state variables
	mState = 0;
	mSkipDirname.clear();
	mpInterface->ResetCanceled();

	for(const auto& file : ls)
	{
		if(mpInterface->IsCanceled())
		{
			retval = ERR_CANCELED;
			goto exit_error;
		}
		dstpath = file.name;
		dstpath.Remove(0, nPrefixLen);
		dstpath.insert(0, path);

		RunFileExistsStateMachine
			(file.name, dstpath, ext4_mode_get(dstpath, &dummy) == EOK, file.isDir);

		// Only set if file is directory
		if(mState & MERGE)
		{
			if(MakeDirectory(dstpath, true) != NOERROR)
			{
				if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
					continue;
				else
				{
					retval = ERR_MKDIR;
					goto exit_error;
				}
			}
		}
		// Only set if file is normal file
		else if(mState & OVERWRITE)
		{
			srcfile.Open(file.name, wxFile::read);
			if(!srcfile.IsOpened())
			{
				if(mpInterface->OnError(file.name) == ANSWER_CONTINUE)
					continue;
				else
				{
					retval = ERR_FOPEN;
					goto exit_error;
				}
			}

			res = ext4_fopen(&dstfile, dstpath.ToUTF8().data(), "w+b");
			if(res != EOK)
			{
				wxString dirname = dstpath;
				if(dirname.Last() != '/')
				{
					dirname.erase(dirname.begin()+dirname.find_last_of('/')+1,
						dirname.end());
				}

				res = MakeDirectory(dirname, true);
				if(res != NOERROR)
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						continue;
					else
					{
						retval = ERR_MKDIR;
						goto exit_error;
					}
				}

				res = ext4_fopen(&dstfile, dstpath.ToUTF8().data(), "w+b");
				if(res != EOK)
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						continue;
					else
					{
						retval = ERR_FOPEN;
						goto exit_error;
					}
				}
			}

			do
			{
				src_count = srcfile.Read(buffer, COPY_BUFFER_SIZE);
				if(src_count == wxInvalidOffset)
				{
					if(mpInterface->OnError(file.name) == ANSWER_CONTINUE)
						break;
					else
					{
						retval = ERR_READ;
						ext4_fclose(&dstfile);
						srcfile.Close();
						goto exit_error;
					}
				}

				res = ext4_fwrite(&dstfile, buffer, src_count, &dst_count);
				if(res != EOK || dst_count != static_cast<size_t>(src_count))
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						break;
					else
					{
						retval = ERR_WRITE;
						ext4_fclose(&dstfile);
						srcfile.Close();
						goto exit_error;
					}
				}

				if(mpInterface->IsCanceled())
				{
					retval = ERR_CANCELED;
					ext4_fclose(&dstfile);
					srcfile.Close();
					goto exit_error;
				}
			}
			while(src_count >= COPY_BUFFER_SIZE);

			ext4_fclose(&dstfile);
			srcfile.Close();
		}

		if(mpInterface)
			mpInterface->OnProgress(1);
	}

exit_error:
	if(buffer)
		delete[] buffer;

	return retval;
}

int Ext4ImgFileArchive::CopyFiles
	(wxString path, std::list<ls_ent>& ls, int nPrefixLen)
{
	int retval = NOERROR;
	int res = 0;
	wxString dstpath;
	ext4_file dstfile = {0};
	ext4_file srcfile = {0};
	BYTE* buffer = 0;
	size_t src_count = 0;
	size_t dst_count = 0;
	uint32_t dummy;

	buffer = new BYTE[COPY_BUFFER_SIZE];
	if(!buffer)
	{
		retval = ERR_ALLOC;
		goto exit_error;
	}

	// Reset state variables
	mState = 0;
	mSkipDirname.clear();
	mpInterface->ResetCanceled();

	for(const auto& file : ls)
	{
		if(mpInterface->IsCanceled())
		{
			retval = ERR_CANCELED;
			goto exit_error;
		}
		dstpath = file.name;
		dstpath.Remove(0, nPrefixLen);
		dstpath.insert(0, path);

		// Don't copy to self, this would corrupt both files
		if(file.name == dstpath)
			continue;

		RunFileExistsStateMachine
			(file.name, dstpath, ext4_mode_get(dstpath, &dummy) == EOK, file.isDir);

		// Only set if file is directory
		if(mState & MERGE)
		{
			if(MakeDirectory(dstpath, true) != NOERROR)
			{
				if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
					continue;
				else
				{
					retval = ERR_MKDIR;
					goto exit_error;
				}
			}
		}
		// Only set if file is normal file
		else if(mState & OVERWRITE)
		{
			res = ext4_fopen(&srcfile, file.name.ToUTF8().data(), "rb");
			if(res != EOK)
			{
				if(mpInterface->OnError(file.name) == ANSWER_CONTINUE)
					continue;
				else
				{
					retval = ERR_FOPEN;
					goto exit_error;
				}
			}

			res = ext4_fopen(&dstfile, dstpath.ToUTF8().data(), "w+b");
			if(res != EOK)
			{
				wxString dirname = dstpath;
				if(dirname.Last() != '/')
				{
					dirname.erase(dirname.begin()+dirname.find_last_of('/')+1,
						dirname.end());
				}

				res = MakeDirectory(dirname, true);
				if(res != NOERROR)
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						continue;
					else
					{
						retval = ERR_MKDIR;
						goto exit_error;
					}
				}

				res = ext4_fopen(&dstfile, dstpath.ToUTF8().data(), "w+b");
				if(res != EOK)
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						continue;
					else
					{
						retval = ERR_FOPEN;
						goto exit_error;
					}
				}
			}

			do
			{
				res = ext4_fread(&srcfile, buffer, COPY_BUFFER_SIZE, &src_count);
				if(res != EOK)
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						break;
					else
					{
						retval = ERR_READ;
						ext4_fclose(&dstfile);
						ext4_fclose(&srcfile);
						goto exit_error;
					}
				}

				res = ext4_fwrite(&dstfile, buffer, src_count, &dst_count);
				if(res != EOK || dst_count != static_cast<size_t>(src_count))
				{
					if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
						break;
					else
					{
						retval = ERR_WRITE;
						ext4_fclose(&dstfile);
						ext4_fclose(&srcfile);
						goto exit_error;
					}
				}

				if(mpInterface->IsCanceled())
				{
					retval = ERR_CANCELED;
					ext4_fclose(&dstfile);
					ext4_fclose(&srcfile);
					goto exit_error;
				}
			}
			while(src_count >= COPY_BUFFER_SIZE);

			ext4_fclose(&dstfile);
			ext4_fclose(&srcfile);
		}

		if(mpInterface)
			mpInterface->OnProgress(1);
	}

exit_error:
	if(buffer)
		delete[] buffer;

	return retval;
}

int Ext4ImgFileArchive::MoveFiles
	(wxString path, std::list<ls_ent>& ls, int nPrefixLen)
{
	int res;
	wxString dstpath;
	uint32_t dummy;

	// Reset state variables
	mState = 0;
	mSkipDirname.clear();
	mpInterface->ResetCanceled();

	for(const auto& file : ls)
	{
		if(mpInterface->IsCanceled())
			return ERR_CANCELED;

		dstpath = file.name;
		dstpath.Remove(0, nPrefixLen);
		dstpath.insert(0, path);

		RunFileExistsStateMachine
			(file.name, dstpath, ext4_mode_get(dstpath, &dummy) == EOK, file.isDir);

		// Only set if file is directory
		if(mState & MERGE)
		{
			// Dont rename lost+found dir
			wxUniChar ch = file.name[MNTNAME_LEN-2]; // part number
			file.name[MNTNAME_LEN-2] = 'x';
			if(file.name == LOST_FOUND)
				continue;
			file.name[MNTNAME_LEN-2] = ch;

			file.name[file.name.length()-1] = 0;
			res = ext4_dir_mv(file.name.ToUTF8().data(), dstpath.ToUTF8().data());
			file.name[file.name.length()-1] = '/';
		}
		// Only set if file is normal file
		else if(mState & OVERWRITE)
		{
			res = ext4_frename(file.name.ToUTF8().data(), dstpath.ToUTF8().data());
		}

		if(mpInterface)
			mpInterface->OnProgress(1);

		if(res != EOK)
		{
			if(mpInterface->OnError(dstpath) == ANSWER_CONTINUE)
				continue;
			else
				return ERR_MOVE;
		}
	}

	return NOERROR;
}

int Ext4ImgFileArchive::DeleteFiles(std::list<ls_ent>& ls)
{
	int res;
	mpInterface->ResetCanceled();

	for(auto& file : ls)
	{
		if(mpInterface->IsCanceled())
			return ERR_CANCELED;

		if(file.isDir)
		{
			// Dont delete lost+found dir
			wxUniChar ch = file.name[MNTNAME_LEN-2]; // part number
			file.name[MNTNAME_LEN-2] = 'x';
			if(file.name == LOST_FOUND)
				continue;
			file.name[MNTNAME_LEN-2] = ch;

			file.name[file.name.length()-1] = 0;
			res = ext4_dir_rm(file.name.ToUTF8().data());
			file.name[file.name.length()-1] = '/';
		}
		else
			res = ext4_fremove(file.name.ToUTF8().data());

		if(mpInterface)
			mpInterface->OnProgress(1);

		if(res != EOK)
		{
			if(mpInterface->OnError(file.name) == ANSWER_CONTINUE)
				continue;
			else
				return ERR_DELETE;
		}
	}
	return NOERROR;
}

int Ext4ImgFileArchive::MakeDirectory(wxString path, bool recursive)
{
	if(recursive)
	{
		int pos = MNTNAME_LEN;
		while((pos = path.find_first_of('/', pos)) != wxNOT_FOUND)
		{
			path[pos] = 0;
			if(ext4_dir_mk(path.ToUTF8().data()) != EOK)
				return ERR_MKDIR;
			path[pos] = '/';
			++pos;
		}
	}
	else
	{
		if(ext4_dir_mk(path.ToUTF8().data()) != EOK)
			return ERR_MKDIR;
	}
	return NOERROR;
}
