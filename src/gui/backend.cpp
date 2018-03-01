/**********************************************************************\
 * ext4-browser
 * backend.cpp
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
#include "stdafx.h"
#include "backend.h"

wxDEFINE_EVENT(BACKEND_CMD_RESULT, wxThreadEvent);
wxDEFINE_EVENT(BACKEND_CMD_PROGRESS, wxThreadEvent);
wxDEFINE_EVENT(BACKEND_QUESTION, wxThreadEvent);

backend::backend(wxFrame* pParent)
	: mpParent(pParent)
	, mCmd(CMD_NOP)
	, mpCmdData(0)
	, mProgress(0)
	, mpArchiveFile(0)
{
	mpThread = new std::thread(&backend::Run, this);
}

backend::~backend()
{
	std::unique_lock<std::mutex> lk(mMutex);
	mCmd = CMD_SHUTDOWN;
	mpCmdData = 0;
	lk.unlock();
	mSignal.notify_one();
	mpThread->join();
	delete mpThread;
}

void backend::WriteImage(PhysicalDevice* device, wxFile* imgFile)
{
	writeImageCtx* pCtx = new writeImageCtx;
	pCtx->device = device;
	pCtx->imgFile = imgFile;

	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCmd = CMD_WRITE_IMAGE;
		mpCmdData = pCtx;
		lk.unlock();
		mSignal.notify_one();
	}
}

void backend::AddFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen)
{
	fileOpCtx* pCtx = new fileOpCtx;
	pCtx->dstpath = dstpath;
	pCtx->ls = ls;
	pCtx->prefixLen = prefixLen;

	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCmd = CMD_ADD;
		mpCmdData = pCtx;
		lk.unlock();
		mSignal.notify_one();
	}
}

void backend::ExtractFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen)
{
	fileOpCtx* pCtx = new fileOpCtx;
	pCtx->dstpath = dstpath;
	pCtx->ls = ls;
	pCtx->prefixLen = prefixLen;

	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCmd = CMD_EXTRACT;
		mpCmdData = pCtx;
		lk.unlock();
		mSignal.notify_one();
	}
}

void backend::CopyFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen)
{
	fileOpCtx* pCtx = new fileOpCtx;
	pCtx->dstpath = dstpath;
	pCtx->ls = ls;
	pCtx->prefixLen = prefixLen;

	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCmd = CMD_COPY;
		mpCmdData = pCtx;
		lk.unlock();
		mSignal.notify_one();
	}
}

void backend::MoveFiles(wxString* dstpath, std::list<ls_ent>* ls, int prefixLen)
{
	fileOpCtx* pCtx = new fileOpCtx;
	pCtx->dstpath = dstpath;
	pCtx->ls = ls;
	pCtx->prefixLen = prefixLen;

	{
		std::unique_lock<std::mutex> lk(mMutex);
		mCmd = CMD_MOVE;
		mpCmdData = pCtx;
		lk.unlock();
		mSignal.notify_one();
	}
}

void backend::DeleteFiles(std::list<ls_ent>* ls)
{
	std::unique_lock<std::mutex> lk(mMutex);
	mCmd = CMD_DELETE;
	mpCmdData = ls;
	lk.unlock();
	mSignal.notify_one();
}

void backend::OnProgress(int n)
{
	wxThreadEvent event(BACKEND_CMD_PROGRESS);
	mProgress += n;
	event.SetId(mCmd);
	event.SetInt(mProgress);
	wxPostEvent(mpParent, event);
}

int backend::OnFileExists(const wxString& filename, bool isFolder)
{
	wxThreadEvent event(BACKEND_QUESTION);
	event.SetId(QUESTION_FILE_EXISTS);
	event.SetString(filename);
	event.SetInt(isFolder);
	wxPostEvent(mpParent, event);

	return WaitForAnswer();
}

int backend::OnError(const wxString& filename)
{
	wxThreadEvent event(BACKEND_QUESTION);
	event.SetId(QUESTION_ERROR);
	event.SetString(filename);
	wxPostEvent(mpParent, event);

	return WaitForAnswer();
}

void backend::Run()
{
	std::unique_lock<std::mutex> lk(mMutex);
	for(;;)
	{
		mSignal.wait(lk);

		switch(mCmd)
		{
			case CMD_SHUTDOWN:
				return;

			case CMD_CREATE_IMAGE:
				//DoCreateAndOpenImage(reinterpret_cast<Ext4ImgFileArchive*>(mpCmdData));
				break;

			case CMD_OPEN_IMAGE:
				break;

			case CMD_OPEN_DISK:
				break;

			case CMD_CLOSE:
				break;

			case CMD_WRITE_IMAGE:
				DoWriteImage(reinterpret_cast<writeImageCtx*>(mpCmdData));
				break;

			case CMD_ADD:
				DoAddFiles(reinterpret_cast<fileOpCtx*>(mpCmdData));
				break;

			case CMD_MAKE_DIR:
				break;

			case CMD_EXTRACT:
				DoExtractFiles(reinterpret_cast<fileOpCtx*>(mpCmdData));
				break;

			case CMD_COPY:
				DoCopyFiles(reinterpret_cast<fileOpCtx*>(mpCmdData));
				break;

			case CMD_MOVE:
				DoMoveFiles(reinterpret_cast<fileOpCtx*>(mpCmdData));
				break;

			case CMD_DELETE:
				DoDeleteFiles(reinterpret_cast<std::list<ls_ent>*>(mpCmdData));
				break;

			case CMD_NOP:
			default:
				break;
		}
		mCmd = CMD_NOP;
	}
}

void backend::DoWriteImage(writeImageCtx* ctx)
{
	wxThreadEvent event(BACKEND_CMD_RESULT);
	event.SetId(mCmd);
	mProgress = 0;

	int result = ctx->device->WriteImage(*ctx->imgFile, this);

	event.SetInt(result);
	wxPostEvent(mpParent, event);

	delete ctx;
}

void backend::DoAddFiles(fileOpCtx* ctx)
{
	wxThreadEvent event(BACKEND_CMD_RESULT);
	event.SetId(mCmd);
	mProgress = 0;
	int result = mpArchiveFile->AddFiles
		(*ctx->dstpath, *ctx->ls, ctx->prefixLen);
	event.SetInt(result);
	wxPostEvent(mpParent, event);

	delete ctx;
}

void backend::DoExtractFiles(fileOpCtx* ctx)
{
	wxThreadEvent event(BACKEND_CMD_RESULT);
	event.SetId(mCmd);
	mProgress = 0;
	int result = mpArchiveFile->ExtractFiles
		(*ctx->dstpath, *ctx->ls, ctx->prefixLen);
	event.SetInt(result);
	wxPostEvent(mpParent, event);

	delete ctx;
}

void backend::DoCopyFiles(fileOpCtx* ctx)
{
	wxThreadEvent event(BACKEND_CMD_RESULT);
	event.SetId(mCmd);
	mProgress = 0;
	int result = mpArchiveFile->CopyFiles
		(*ctx->dstpath, *ctx->ls, ctx->prefixLen);
	event.SetInt(result);
	wxPostEvent(mpParent, event);

	delete ctx;
}

void backend::DoMoveFiles(fileOpCtx* ctx)
{
	wxThreadEvent event(BACKEND_CMD_RESULT);
	event.SetId(mCmd);
	mProgress = 0;
	int result = mpArchiveFile->MoveFiles
		(*ctx->dstpath, *ctx->ls, ctx->prefixLen);
	event.SetInt(result);
	wxPostEvent(mpParent, event);

	delete ctx;
}

void backend::DoDeleteFiles(std::list<ls_ent>* ls)
{
	wxThreadEvent event(BACKEND_CMD_RESULT);
	event.SetId(mCmd);
	mProgress = 0;
	int result = mpArchiveFile->DeleteFiles(*ls);
	event.SetInt(result);
	wxPostEvent(mpParent, event);

	delete ls;
}
