/**********************************************************************\
 * ext4-browser
 * backend.h
 *
 * Backend
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

#include "ext4ImgArchive.h"

//class backendEvent;
wxDECLARE_EVENT(BACKEND_CMD_RESULT, wxThreadEvent);
wxDECLARE_EVENT(BACKEND_CMD_PROGRESS, wxThreadEvent);
wxDECLARE_EVENT(BACKEND_QUESTION, wxThreadEvent);

// define an event table entry
// (wxThreadEvent does not support custom types by default)
#define EVT_BACKEND_THREAD(id, type, func) \
 	wx__DECLARE_EVT1(type, id, wxThreadEventHandler(func))

// backend runs task in another thread to keep
// gui responsive (eg. for progress/cancel)
class backend
	: public IBackendTask
{
	public:
		backend(wxFrame* pParent);
		backend(const backend&) = delete;
		backend(backend&&) = delete;
		~backend();

		enum commandID
		{
			CMD_NOP = wxID_HIGHEST, // Spurious wakeup
			CMD_SHUTDOWN,
			CMD_CREATE_IMAGE,
			CMD_OPEN_IMAGE,
			CMD_OPEN_DISK,
			CMD_CLOSE,
			CMD_WRITE_IMAGE,
			CMD_ADD,
			CMD_MAKE_DIR,
			CMD_EXTRACT,
			CMD_COPY,
			CMD_MOVE,
			CMD_DELETE
		};

		void WriteImage(PhysicalDevice* device, wxFile* imgFile);
		void AddFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen);
		void ExtractFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen);
		void CopyFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen);
		void MoveFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen);
		void DeleteFiles(std::list<ls_ent>* ls);

		inline void SetImageFile(Ext4ImgFileArchive* pImg)
		{
			mpArchiveFile = pImg;
			if(mpArchiveFile)
				mpArchiveFile->SetInterface(this);
		}

		virtual void OnProgress(int n) override;
		virtual int  OnFileExists(const wxString& filename, bool isFolder) override;
		virtual int  OnError(const wxString& filename) override;

	private:
		wxFrame* mpParent;
		std::thread* mpThread;
		std::mutex mMutex;
		std::condition_variable mSignal;

		int mCmd;
		void* mpCmdData;
		int mProgress;
		Ext4ImgFileArchive* mpArchiveFile;

		struct writeImageCtx
		{
			writeImageCtx() : device(0), imgFile(0) {}
			writeImageCtx(const writeImageCtx&) = delete;
			writeImageCtx(writeImageCtx&&) = delete;
			~writeImageCtx()
			{
				if(device) delete device;
				if(imgFile) delete imgFile;
			}
			PhysicalDevice* device;
			wxFile* imgFile;
		};

		struct fileOpCtx
		{
			fileOpCtx() : dstpath(0), ls(0), prefixLen(0) {}
			fileOpCtx(const fileOpCtx&) = delete;
			fileOpCtx(fileOpCtx&&) = delete;
			~fileOpCtx()
			{
				if(dstpath) delete dstpath;
				if(ls) delete ls;
			}
			wxString* dstpath;
			std::list<ls_ent>* ls;
			int prefixLen;
		};

		void Run();

		void DoWriteImage(writeImageCtx* ctx);
		void DoAddFiles(fileOpCtx* ctx);
		void DoExtractFiles(fileOpCtx* ctx);
		void DoCopyFiles(fileOpCtx* ctx);
		void DoMoveFiles(fileOpCtx* ctx);
		void DoDeleteFiles(std::list<ls_ent>* ls);
};
