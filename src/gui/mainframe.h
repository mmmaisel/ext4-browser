/**********************************************************************\
 * ext4-browser
 * mainframe.h
 *
 * Mainframe
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
#include "physicalDevice.h"
#include "backend.h"
#include "fileTree.h"
#include "directoryLister.h"

class mainframe
	: public wxFrame
{
	public:
		mainframe();
		mainframe(const mainframe&) = delete;
		mainframe(mainframe&&) = delete;
		~mainframe();

		enum eIDs
		{
			// Home ribbon page.
			ID_MAIN_MENU = wxID_HIGHEST + 1,
			ID_FILE_TREE_CTRL,
			ID_CREATE_IMAGE,
			ID_OPEN_IMAGE,
			ID_CLOSE_IMAGE,
			ID_OPEN_DISK,
			ID_WRITE_IMAGE,
			ID_ADD_FILE,
			ID_ADD_FOLDER,
			ID_NEW_FOLDER,
			ID_EXTRACT_FILE,
			ID_COPY_FILE,
			ID_CUT_FILE,
			ID_PASTE_FILE,
			ID_RENAME_FILE,
			ID_DELETE_FILE,
			ID_ACCEL_DELETE,
			ID_ACCEL_REFRESH,
			ID_ACCEL_COPY,
			ID_ACCEL_CUT,
			ID_ACCEL_PASTE
		};

	private:
		void OnClose(wxCloseEvent& event);

		// Ribbon events.
		void OnAbout(wxRibbonBarEvent& evt);
		void OnCreateImage(wxRibbonButtonBarEvent& evt);
		void OnOpenImage(wxRibbonButtonBarEvent& evt);
		void OnCloseImage(wxRibbonButtonBarEvent& evt);
		void OnOpenDisk(wxRibbonButtonBarEvent& evt);
		void OnWriteImage(wxRibbonButtonBarEvent& evt);
		void OnAddFile(wxRibbonButtonBarEvent& evt);
		void OnAddFolder(wxRibbonButtonBarEvent& evt);
		void OnNewFolder(wxRibbonButtonBarEvent& evt);
		void OnExtractFile(wxRibbonButtonBarEvent& evt);
		void OnCopyFile(wxRibbonButtonBarEvent& evt);
		void OnCutFile(wxRibbonButtonBarEvent& evt);
		void OnPasteFile(wxRibbonButtonBarEvent& evt);
		void OnRenameFile(wxRibbonButtonBarEvent& evt);
		void OnDeleteFile(wxRibbonButtonBarEvent& evt);

		void OnCmdComplete(wxThreadEvent& evt);
		void OnCmdProgress(wxThreadEvent& evt);
		void OnBackendQuestion(wxThreadEvent& evt);
		void OnAccelerator(wxCommandEvent& evt);

		wxDECLARE_EVENT_TABLE();

		void OpenImage();
		void CloseImage();

		void PasteFile();
		void DeleteFile();

		// Variables
		backend* mpBackend;
		Ext4ImgFileArchive* mpArchiveFile;
		PhysicalDevice* mpPhysicalDevice;

		// GUI Elements
		wxRibbonBar* mpMenu;
		FileTree* mpFileTree;
		wxProgressDialog* mpProgressDlg;

		bool mCutClipboard;
};
