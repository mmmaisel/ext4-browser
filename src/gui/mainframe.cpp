/**********************************************************************\
 * ext4-browser
 * mainframe.cpp
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
#include "stdafx.h"
#include "mainframe.h"
#include "newImageDlg.h"
#include "diskDlg.h"
#include "writeImageDlg.h"
#include "fileExistsDlg.h"
#include "aboutDlg.h"
#include "resource/messages.h"

#include "elevation.h"

/**********************************************************************\
 * Performs all initializations and creates GUI.
\**********************************************************************/

mainframe::mainframe()
	: wxFrame(0, wxID_ANY, "ext4-browser", wxDefaultPosition,
		wxSize(800, 600), wxDEFAULT_FRAME_STYLE)
	, mpBackend(0)
	, mpArchiveFile(0)
	, mpPhysicalDevice(0)
	, mpMenu(0)
	, mpFileTree(0)
	, mpProgressDlg(0)
	, mCutClipboard(false)
{
	wxImage::AddHandler(new wxPNGHandler);

	/**********************************************************************\
	 * GUI Creation
	\**********************************************************************/

	// Load and set program icon.
	SetIcon(wxIcon(L"icons/ext4-browser.png", wxBITMAP_TYPE_PNG));

	//
	// Create ribbon menu.
	//
	mpMenu = new wxRibbonBar(this, ID_MAIN_MENU, wxDefaultPosition,
		wxDefaultSize, (wxRIBBON_BAR_DEFAULT_STYLE | wxRIBBON_BAR_ALWAYS_SHOW_TABS)
		& ~(wxRIBBON_BAR_SHOW_TOGGLE_BUTTON));
	mpMenu->GetArtProvider()->SetColourScheme
		(wxColour(200,200,200), wxColour(255,223,114), wxColour(0,0,0));
	{
		// first page
		wxRibbonPage* page = new wxRibbonPage(mpMenu, wxID_ANY, wxT("Home"));
		{
			wxRibbonPanel* toolbar_panel = new wxRibbonPanel
				(page, wxID_ANY, MSG_MENU_PAGE_HOME, wxNullBitmap, wxDefaultPosition,
				wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
			{
				wxRibbonButtonBar* button_bar = new wxRibbonButtonBar
					(toolbar_panel, -1, wxDefaultPosition, wxDefaultSize,
					wxRIBBON_BUTTONBAR_BUTTON_LARGE);
				{
					button_bar->AddButton(ID_CREATE_IMAGE, MSG_MENU_CREATE_IMAGE,
						wxImage(L"icons/new-image.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_OPEN_IMAGE, MSG_MENU_OPEN_IMAGE,
						wxImage(L"icons/open-image.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_CLOSE_IMAGE, MSG_MENU_CLOSE,
						wxImage(L"icons/close-image.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_OPEN_DISK, MSG_MENU_OPEN_DISK,
						wxImage(L"icons/open-disk.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_WRITE_IMAGE, MSG_MENU_WRITE_IMAGE,
						wxImage(L"icons/write-image.png", wxBITMAP_TYPE_PNG));
				}
			}
			toolbar_panel = new wxRibbonPanel
				(page, wxID_ANY, MSG_MENU_PAGE_FILE, wxNullBitmap, wxDefaultPosition,
				wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
			{
				wxRibbonButtonBar* button_bar = new wxRibbonButtonBar
					(toolbar_panel, -1, wxDefaultPosition, wxDefaultSize,
					wxRIBBON_BUTTONBAR_BUTTON_LARGE);
				{
					button_bar->AddButton(ID_ADD_FILE, MSG_MENU_ADD_FILES,
						wxImage(L"icons/add-file.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_ADD_FOLDER, MSG_MENU_ADD_FOLDERS,
						wxImage(L"icons/add-folder.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_NEW_FOLDER, MSG_MENU_NEW_FOLDER,
						wxImage(L"icons/new-folder.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_EXTRACT_FILE, MSG_MENU_EXTRACT,
						wxImage(L"icons/extract-file.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_COPY_FILE, MSG_MENU_COPY,
						wxImage(L"icons/copy-file.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_CUT_FILE, MSG_MENU_CUT,
						wxImage(L"icons/cut-file.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_PASTE_FILE, MSG_MENU_PASTE,
						wxImage(L"icons/paste-file.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_RENAME_FILE, MSG_MENU_RENAME,
						wxImage(L"icons/rename-file.png", wxBITMAP_TYPE_PNG));
					button_bar->AddButton(ID_DELETE_FILE, MSG_MENU_DELETE,
						wxImage(L"icons/delete-file.png", wxBITMAP_TYPE_PNG));
				}
			}
		}
	}
	mpMenu->Realize();

	//
	// Set window layout.
	//

	// Create main file listing
	mpFileTree = new FileTree(this, ID_FILE_TREE_CTRL,
		wxDefaultPosition, wxDefaultSize, wxTL_MULTIPLE);

	// Create ribbon <-> windows sizer.
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(mpMenu, 0, wxEXPAND);
	s->Add(mpFileTree, 1, wxEXPAND);
	SetSizer(s);

	SetMinSize(wxSize(480, 360));

	// Setup keyboard shortcuts
	wxAcceleratorEntry entries[5];
	entries[0].Set(wxACCEL_NORMAL, WXK_F5, ID_ACCEL_REFRESH);
	entries[1].Set(wxACCEL_NORMAL, WXK_DELETE, ID_ACCEL_DELETE);
	entries[2].Set(wxACCEL_CTRL, 'C', ID_ACCEL_COPY);
	entries[3].Set(wxACCEL_CTRL, 'X', ID_ACCEL_CUT);
	entries[4].Set(wxACCEL_CTRL, 'V', ID_ACCEL_PASTE);
	wxAcceleratorTable accel(5, entries);
	SetAcceleratorTable(accel);

	mpBackend = new backend(this);
}

mainframe::~mainframe()
{
	if(mpBackend)
		delete mpBackend;

	if(mpArchiveFile)
		delete mpArchiveFile;

	if(mpPhysicalDevice)
		delete mpPhysicalDevice;

	if(mpProgressDlg)
		delete mpProgressDlg;

	mpFileTree->Destroy();

	// Detach sizer manually to prevent some assertion
	// issues on close.
	GetSizer()->Detach(mpFileTree);
}

/**********************************************************************\
 * GUI events - home
\**********************************************************************/

void mainframe::OnClose(wxCloseEvent& event)
{
	Destroy();
}

void mainframe::OnAbout(wxRibbonBarEvent& WXUNUSED(evt))
{
	aboutDialog aboutdlg(this, wxID_ANY, MSG_ABOUT);
	aboutdlg.ShowModal();
}

void mainframe::OnCreateImage(wxRibbonButtonBarEvent& evt)
{
	if(mpArchiveFile)
	{
		if(wxMessageBox(MSG_CONFIRM_CLOSE, MSG_CONFIRM_CLOSE_TITLE,
			wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
		CloseImage();
	}
	mpArchiveFile = new Ext4ImgFileArchive();

	newImageDialog imgDlg(this, wxID_ANY, MSG_NEW_IMG_TITLE, mpArchiveFile);
	if(imgDlg.ShowModal() == wxID_CANCEL)
	{
		delete mpArchiveFile;
		mpArchiveFile = 0;
		return;
	}
	OpenImage();
}

void mainframe::OnOpenImage(wxRibbonButtonBarEvent& evt)
{
	wxString path;
	wxFileDialog filedlg
		(this, wxFileSelectorPromptStr, "", "",
		SELECTOR_IMG_FILES, wxFD_OPEN);

	if(mpArchiveFile)
	{
		if(wxMessageBox(MSG_CONFIRM_CLOSE, MSG_CONFIRM_CLOSE_TITLE,
			wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
		CloseImage();
	}

	if(filedlg.ShowModal() == wxID_CANCEL)
		return;
	mpArchiveFile = new Ext4ImgFileArchive();
	if(mpArchiveFile == 0)
	{
		wxMessageBox(MSG_ALLOC_FAILED);
		return;
	}

	path = filedlg.GetPath();
	if(mpArchiveFile->Open(path) != Archive::NOERROR)
	{
		wxMessageBox(MSG_OPEN_IMAGE_FAILED);
		delete mpArchiveFile;
		mpArchiveFile = 0;
		return;
	}
	OpenImage();
}

void mainframe::OnCloseImage(wxRibbonButtonBarEvent& evt)
{
	CloseImage();
}

void mainframe::OnOpenDisk(wxRibbonButtonBarEvent& evt)
{
	if(mpArchiveFile)
	{
		if(wxMessageBox(MSG_CONFIRM_CLOSE, MSG_CONFIRM_CLOSE_TITLE,
			wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
		CloseImage();
	}

	if(!IsSelfElevated())
	{
		wxString self(wxStandardPaths::Get().GetExecutablePath());
		if(wxMessageBox(MSG_RESTART_ROOT, MSG_RESTART_ROOT_TITLE,
			wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
		if(StartElevated(self))
			Close();
		else
			wxMessageBox(MSG_RESTART_ROOT_FAILED);
		return;
	}

	std::list<PhysicalDevice> devices;
	int retval = PhysicalDevice::EnumFlashDevices(devices);

	if(retval != PhysicalDevice::NOERROR)
	{
		wxMessageBox(wxString::Format(MSG_ENUM_DISKS_FAILED, retval));
		return;
	}

	wxString* choises = new wxString[devices.size()];
	int i = 0;
	for(const auto& dev : devices)
	{
		choises[i++] = dev.FormatDisplayName();
	}

	diskSelectionDialog diskdlg
		(this, wxID_ANY, MSG_SELECT_DISK_TITLE,
		choises, devices.size());
	if(diskdlg.ShowModal() == wxID_CANCEL)
	{
		delete[] choises;
		return;
	}

	auto device = devices.begin();
	std::advance(device, diskdlg.GetSelectedDisk());
	delete[] choises;

	mpArchiveFile = new Ext4ImgFileArchive();
	if(mpArchiveFile == 0)
	{
		wxMessageBox(MSG_ALLOC_FAILED);
		return;
	}

	mpPhysicalDevice = new PhysicalDevice(*device);

	if(mpArchiveFile->OpenPhysical(mpPhysicalDevice) != Archive::NOERROR)
	{
		wxMessageBox(MSG_OPEN_IMAGE_FAILED);
		delete mpArchiveFile;
		mpArchiveFile = 0;
		delete mpPhysicalDevice;
		mpPhysicalDevice = 0;
		return;
	}
	OpenImage();
}

void mainframe::OnWriteImage(wxRibbonButtonBarEvent& evt)
{
	if(mpArchiveFile)
	{
		if(wxMessageBox(MSG_CONFIRM_CLOSE, MSG_CONFIRM_CLOSE_TITLE,
			wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
		CloseImage();
	}

	if(!IsSelfElevated())
	{
		wxString self(wxStandardPaths::Get().GetExecutablePath());
		if(wxMessageBox(MSG_RESTART_ROOT, MSG_RESTART_ROOT_TITLE,
			wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
		if(StartElevated(self))
		{
			Close();
			return;
		}
	}

	std::list<PhysicalDevice> devices;
	int retval = PhysicalDevice::EnumFlashDevices(devices);

	if(retval != PhysicalDevice::NOERROR)
	{
		wxMessageBox(wxString::Format(MSG_ENUM_DISKS_FAILED, retval));
		return;
	}

	wxString* choises = new wxString[devices.size()];
	int i = 0;
	for(const auto& dev : devices)
	{
		choises[i++] = dev.FormatDisplayName();
	}

	writeImageDialog diskdlg
		(this, wxID_ANY, MSG_SELECT_DISK_TITLE, choises, devices.size());
	if(diskdlg.ShowModal() == wxID_CANCEL)
	{
		delete[] choises;
		return;
	}

	auto device = devices.begin();
	std::advance(device, diskdlg.GetSelectedDisk());
	delete[] choises;

	// Show data loss warning message,
	// get device name independently from advanced iterator (to be sure).
	if(wxMessageBox(
		wxString::Format(MSG_CONFIRM_WRITE_IMG, device->FormatDisplayName()),
		MSG_CONFIRM_WRITE_IMG_TITLE,
		wxICON_WARNING | wxOK | wxCANCEL) != wxOK)
	{
		// Aborted by user
		return;
	}

	wxFile* pFile = new wxFile(diskdlg.GetImageFilename());
	if(!pFile->IsOpened())
	{
		delete pFile;
		wxMessageBox(MSG_OPEN_IMAGE_FAILED);
		return;
	}

	mpPhysicalDevice = new PhysicalDevice(*device);

	// Get image size in blocks
	pFile->SeekEnd(0);
	size_t imgsize = pFile->Tell() / mpPhysicalDevice->GetSectorSize();
	pFile->Seek(0);

	// wxPD_AUTO is required for OSX
	mpProgressDlg = new wxProgressDialog
		(MSG_WRITE_IMG_PROGRESS, MSG_IN_PROGRESS_TITLE, imgsize, this,
		wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

	mpBackend->WriteImage(mpPhysicalDevice, pFile);
	// Continues in OnCmdComplete()
}

void mainframe::OnAddFile(wxRibbonButtonBarEvent& evt)
{
	wxString* dstpath = new wxString();
	wxArrayString srcpaths;
	wxTreeListItem selection;
	std::list<ls_ent>* ls = new std::list<ls_ent>;

	wxFileDialog filedlg
		(this, wxFileSelectorPromptStr, "", "", SELECTOR_ALL_FILES,
		wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		goto exit_error;
	}

	selection = mpFileTree->GetSingleSelectionWithDefault();
	if(!selection.IsOk())
	{
		wxMessageBox(MSG_INVALID_SELECTION);
		goto exit_error;
	}

	if(mpFileTree->IsFile(selection))
		selection = mpFileTree->GetItemParent(selection);

	*dstpath = mpFileTree->GetPathFromTree(selection);
	if(!mpArchiveFile->IsValidPath(*dstpath))
	{
		wxMessageBox(MSG_DST_NOT_ON_PARTITON);
		goto exit_error;
	}

	if(filedlg.ShowModal() == wxID_CANCEL)
		goto exit_error;

	filedlg.GetPaths(srcpaths);

	for(unsigned int i = 0; i < srcpaths.size(); ++i)
		ls->push_back(ls_ent(srcpaths[i], false));

	// wxPD_AUTO is required for OSX
	mpProgressDlg = new wxProgressDialog
		(MSG_IN_PROGRESS, MSG_IN_PROGRESS_TITLE, srcpaths.size(), this,
		wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

	// On windows, \ is the path separator
#if defined(_WIN32) || defined(_WIN64)
	mpBackend->AddFiles(dstpath, ls, ls->begin()->name.find_last_of('\\')+1);
#else
	mpBackend->AddFiles(dstpath, ls, ls->begin()->name.find_last_of('/')+1);
#endif
	// Continues in OnCmdComplete()
	return;

exit_error:
	delete dstpath;
	delete ls;
}

void mainframe::OnAddFolder(wxRibbonButtonBarEvent& evt)
{
	wxString* dstpath = new wxString();
	//wxArrayString srcpaths;
	wxString srcpath;
	wxTreeListItem selection;
	std::list<ls_ent>* ls = new std::list<ls_ent>;
	int prefixLen;

	wxDirDialog dirdlg(this);

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		goto exit_error;
	}

	selection = mpFileTree->GetSingleSelectionWithDefault();
	if(!selection.IsOk())
	{
		wxMessageBox(MSG_INVALID_SELECTION);
		goto exit_error;
	}

	if(mpFileTree->IsFile(selection))
		selection = mpFileTree->GetItemParent(selection);

	*dstpath = mpFileTree->GetPathFromTree(selection);
	if(!mpArchiveFile->IsValidPath(*dstpath))
	{
		wxMessageBox(MSG_DST_NOT_ON_PARTITON);
			return;
	}

	if(dirdlg.ShowModal() == wxID_CANCEL)
		goto exit_error;

	srcpath = dirdlg.GetPath();
	//filedlg.GetPaths(srcpaths);

	//for(unsigned int i = 0; i < srcpaths.size(); ++i)
	{
		DirectoryLister dirLister(ls);
		wxString name;
		wxDir dir(srcpath/*s[i]*/);
		if(!dir.IsOpened())
		{
			wxMessageBox(MSG_OPEN_DIR_FAILED);
			goto exit_error;
		}

		dir.Traverse(dirLister, wxEmptyString,
			wxDIR_FILES | wxDIR_DIRS | wxDIR_HIDDEN
			| /*wxDIR_DOTDOT | */wxDIR_NO_FOLLOW);

		if(dirLister.GetError())
		{
			wxMessageBox(MSG_LIST_DIR_FAILED);
			goto exit_error;
		}

		if(ls->size() == 0)
		{
			wxMessageBox(wxString::Format(MSG_EMPTY_DIR_NOT_ADDED, srcpath));
			goto exit_error;
		}
		ls->sort();

		// On windows, \ is the path separator
#if defined(_WIN32) || defined(_WIN64)
		prefixLen = srcpath.rfind(L'\\', srcpath.length()-1);
#else
		prefixLen = srcpath.rfind(L'/', srcpath.length()-1);
#endif

		// wxPD_AUTO is required for OSX
		mpProgressDlg = new wxProgressDialog
			(MSG_IN_PROGRESS, MSG_IN_PROGRESS_TITLE, ls->size(), this,
			wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

		mpBackend->AddFiles(dstpath, ls, prefixLen+1);
	}
	// Continues in OnCmdComplete()
	return;

exit_error:
	delete dstpath;
	delete ls;
}

void mainframe::OnNewFolder(wxRibbonButtonBarEvent& evt)
{
	wxString newName;
	wxString dstpath;
	wxTreeListItem selection;
	wxTextEntryDialog txtdlg(this, MSG_MKDIR_NAME);

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		return;
	}

	selection = mpFileTree->GetSingleSelectionWithDefault();
	if(!selection.IsOk())
	{
		wxMessageBox(MSG_INVALID_SELECTION);
		return;
	}

	if(mpFileTree->IsFile(selection))
		selection = mpFileTree->GetItemParent(selection);

	dstpath = mpFileTree->GetPathFromTree(selection);
	if(!mpArchiveFile->IsValidPath(dstpath))
	{
		wxMessageBox(MSG_DST_NOT_ON_PARTITON);
		return;
	}

	if(txtdlg.ShowModal() == wxID_CANCEL)
		return;

	newName = txtdlg.GetValue();
	// / and \ are not allowed in filenames
	if(newName.find_first_of('/') != wxString::npos
		|| newName.find_first_of('\\') != wxString::npos)
	{
		wxMessageBox(MSG_INVALID_FILENAME);
		return;
	}
	dstpath.append(newName);

	if(mpArchiveFile->MakeDirectory(dstpath, false) != Archive::NOERROR)
	{
		wxMessageBox(MSG_MKDIR_FAILED);
		return;
	}

	mpFileTree->ListDirectory(selection, true);
}

void mainframe::OnExtractFile(wxRibbonButtonBarEvent& evt)
{
	wxString* path = new wxString();
	wxString prefix;
	wxDirDialog dirdlg(this);
	wxTreeListItems selection;
	wxTreeListItem localRoot;
	std::list<ls_ent>* ls = new std::list<ls_ent>();
	int nPrefixLen;

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		goto exit_error;
	}

	if(dirdlg.ShowModal() == wxID_CANCEL)
		goto exit_error;

	*path = dirdlg.GetPath();

	selection = mpFileTree->GetSelectionWithDefault();
	mpFileTree->DenestSelection(selection);
	localRoot = mpFileTree->GetLowestCommonAncestor(selection);
	if(mpFileTree->IsFile(localRoot) || mpFileTree->IsDirectory(localRoot))
		localRoot = mpFileTree->GetItemParent(localRoot);

	prefix = mpFileTree->GetPathFromTree(localRoot);
	nPrefixLen = prefix.size()-1; // dirdlg returns path without trailing /

	for(unsigned int i = 0; i < selection.size(); ++i)
	{
		wxString selpath = mpFileTree->GetPathFromTree(selection[i]);

		// If root is selected, extract all partitions
		if(selpath == Ext4ImgFileArchive::ROOTNAME)
		{
			for(int i = 0; i < 4; ++i)
			{
				selpath.Append('p');
				selpath.Append((char)('0' + i), (size_t)1);
				selpath.Append('/');
				if(mpArchiveFile->ListDirectory(selpath, *ls, true)
					!= Archive::NOERROR)
				{
					wxMessageBox(MSG_LIST_DIR_FAILED);
					goto exit_error;
				}
				selpath.RemoveLast(3);
			}
		}
		else if(mpArchiveFile->ListDirectory(selpath, *ls, true)
			!= Archive::NOERROR)
		{
			wxMessageBox(MSG_LIST_DIR_FAILED);
			goto exit_error;
		}
	}
	ls->sort();

	// wxPD_AUTO is required for OSX
	mpProgressDlg = new wxProgressDialog
		(MSG_IN_PROGRESS, MSG_IN_PROGRESS_TITLE, ls->size(), this,
		wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

	// Mixed / and \ is OK in wxFile on windows
	mpBackend->ExtractFiles(path, ls, nPrefixLen);

	// Continues in OnCmdComplete()
	return;

exit_error:
	delete path;
	delete ls;
}

void mainframe::OnCopyFile(wxRibbonButtonBarEvent& evt)
{
	mpFileTree->StoreSelection();
	mCutClipboard = false;
}

void mainframe::OnCutFile(wxRibbonButtonBarEvent& evt)
{
	mpFileTree->StoreSelection();
	mCutClipboard = true;
}

void mainframe::OnPasteFile(wxRibbonButtonBarEvent& evt)
{
	PasteFile();
}

void mainframe::OnRenameFile(wxRibbonButtonBarEvent& evt)
{
	int pos;
	wxString newName;
	wxString srcpath;
	wxString* dstpath = new wxString();
	std::list<ls_ent>* ls = new std::list<ls_ent>();
	wxTreeListItem selection;
	wxTextEntryDialog txtdlg(this, MSG_RENAME);

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		goto exit_error;
	}

	selection = mpFileTree->GetSingleSelectionWithDefault();
	if(!selection.IsOk())
	{
		wxMessageBox(MSG_INVALID_SELECTION);
		goto exit_error;
	}

	srcpath = mpFileTree->GetPathFromTree(selection);
	if(mpArchiveFile->IsRootPath(srcpath))
	{
		wxMessageBox(MSG_INVALID_SELECTION);
		goto exit_error;
	}
	if(!mpArchiveFile->IsValidPath(srcpath))
	{
		wxMessageBox(MSG_DST_NOT_ON_PARTITON);
		goto exit_error;
	}

	txtdlg.SetValue(mpFileTree->GetItemText(selection));
	if(txtdlg.ShowModal() == wxID_CANCEL)
		goto exit_error;

	newName = txtdlg.GetValue();
	// / and \ are no allowed in filenames
	if(newName.find_first_of('/') != wxString::npos
		|| newName.find_first_of('\\') != wxString::npos)
	{
		wxMessageBox(MSG_INVALID_FILENAME);
		goto exit_error;
	}
	dstpath->append(newName);

	if(mpArchiveFile->ListDirectory(srcpath, *ls, false) != Archive::NOERROR)
	{
		wxMessageBox(MSG_LIST_DIR_FAILED);
		goto exit_error;
	}

	*dstpath = srcpath;
	pos = dstpath->find_last_of('/', dstpath->length()-2);
	dstpath->erase(dstpath->begin()+pos+1, dstpath->end());
	dstpath->append(txtdlg.GetValue());

	mpBackend->MoveFiles(dstpath, ls, ls->begin()->name.length());

	// Continues in OnCmdComplete()
	return;

exit_error:
	delete dstpath;
	delete ls;
}

void mainframe::OnDeleteFile(wxRibbonButtonBarEvent& evt)
{
	DeleteFile();
}

void mainframe::OnCmdComplete(wxThreadEvent& evt)
{
	int id = evt.GetId();

	if(mpProgressDlg)
	{
		delete mpProgressDlg;
		mpProgressDlg = 0;
	}

	switch(id)
	{
		case backend::CMD_ADD:
			if(evt.GetInt() == Ext4ImgFileArchive::NOERROR)
				wxMessageBox(MSG_OP_SUCCESSFUL);
			else if(evt.GetInt() == Ext4ImgFileArchive::ERR_CANCELED)
				wxMessageBox(MSG_OP_CANCELED);
			else
				wxMessageBox(MSG_ADD_FAILED);

			mpFileTree->RefreshTree();
			break;

		case backend::CMD_EXTRACT:
			if(evt.GetInt() != Ext4ImgFileArchive::NOERROR)
				wxMessageBox(MSG_EXTRACT_FAILED);
			else
				wxMessageBox(MSG_OP_SUCCESSFUL);
			break;

		case backend::CMD_WRITE_IMAGE:
			if(evt.GetInt() == PhysicalDevice::NOERROR)
				wxMessageBox(MSG_OP_SUCCESSFUL);
			else if(evt.GetInt() == PhysicalDevice::ERR_CANCELED)
				wxMessageBox(MSG_OP_CANCELED);
			else
				wxMessageBox(MSG_WRITE_IMAGE_FAILED);

			// Invalidate physical device (already deleted in ctx writeImageCtx destructor)
			mpPhysicalDevice = 0;
			break;

		case backend::CMD_COPY:
			if(evt.GetInt() == PhysicalDevice::NOERROR)
				wxMessageBox(MSG_OP_SUCCESSFUL);
			else
				wxMessageBox(MSG_COPY_FAILED);

			mpFileTree->RefreshTree();
			break;

		case backend::CMD_MOVE:
			if(evt.GetInt() == PhysicalDevice::NOERROR)
				wxMessageBox(MSG_OP_SUCCESSFUL);
			else
				wxMessageBox(MSG_MOVE_FAILED);

			mpFileTree->RefreshTree();
			break;

		case backend::CMD_DELETE:
			if(evt.GetInt() != Ext4ImgFileArchive::NOERROR)
			{
				wxMessageBox(MSG_DEL_FAILED);
			}
			else
			{
				wxMessageBox(MSG_OP_SUCCESSFUL);
				mpFileTree->RefreshTree();
			}
			break;

		default:
			wxMessageBox("Unknown command event received");
			break;
	}
}

void mainframe::OnCmdProgress(wxThreadEvent& evt)
{
	if(mpProgressDlg)
	{
		// update returns false if canceled
		if(!mpProgressDlg->Update(evt.GetInt()))
		{
			mpBackend->Cancel();
			delete mpProgressDlg;
			mpProgressDlg = 0;
		}
	}
}

void mainframe::OnBackendQuestion(wxThreadEvent& evt)
{
	if(evt.GetId() == IBackendTask::QUESTION_FILE_EXISTS)
	{
		bool isDir = evt.GetInt();
		wxString filename = evt.GetString();
		int result = 0;
		fileExistsDialog dlg
			(this, wxID_ANY, MSG_EXISTS_DLG_TITLE, isDir, filename);
		result = dlg.ShowModal();
		mpBackend->AnswerQuestion(result);
	}
	else if(evt.GetId() == IBackendTask::QUESTION_ERROR)
	{
		wxString filename = evt.GetString();
		int result = wxMessageBox(
			wxString::Format(MSG_ARCHIVE_ERROR, filename), MSG_ERROR, wxYES_NO);
		if(result == wxYES)
			mpBackend->AnswerQuestion(Archive::ANSWER_CONTINUE);
		else
			mpBackend->AnswerQuestion(Archive::ANSWER_ABORT);
	}
}

void mainframe::OnAccelerator(wxCommandEvent& evt)
{
	int id = evt.GetId();

	switch(id)
	{
		case ID_ACCEL_DELETE:
			DeleteFile();
			break;

		case ID_ACCEL_REFRESH:
			mpFileTree->RefreshTree();
			break;

		case ID_ACCEL_COPY:
			mpFileTree->StoreSelection();
			mCutClipboard = false;
			break;

		case ID_ACCEL_CUT:
			mpFileTree->StoreSelection();
			mCutClipboard = true;
			break;

		case ID_ACCEL_PASTE:
			PasteFile();
			break;
	}
}

/**********************************************************************\
 * wx event table
\**********************************************************************/

wxBEGIN_EVENT_TABLE(mainframe, wxFrame)
	EVT_CLOSE(mainframe::OnClose)
	EVT_RIBBONBAR_HELP_CLICK(ID_MAIN_MENU, mainframe::OnAbout)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_CREATE_IMAGE, mainframe::OnCreateImage)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_OPEN_IMAGE, mainframe::OnOpenImage)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_CLOSE_IMAGE, mainframe::OnCloseImage)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_OPEN_DISK, mainframe::OnOpenDisk)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_WRITE_IMAGE, mainframe::OnWriteImage)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_ADD_FILE, mainframe::OnAddFile)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_ADD_FOLDER, mainframe::OnAddFolder)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_NEW_FOLDER, mainframe::OnNewFolder)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_EXTRACT_FILE, mainframe::OnExtractFile)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_COPY_FILE, mainframe::OnCopyFile)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_CUT_FILE, mainframe::OnCutFile)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_PASTE_FILE, mainframe::OnPasteFile)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_RENAME_FILE, mainframe::OnRenameFile)
	EVT_RIBBONBUTTONBAR_CLICKED(mainframe::ID_DELETE_FILE, mainframe::OnDeleteFile)
	EVT_BACKEND_THREAD(wxID_ANY, BACKEND_CMD_RESULT, mainframe::OnCmdComplete)
	EVT_BACKEND_THREAD(wxID_ANY, BACKEND_CMD_PROGRESS, mainframe::OnCmdProgress)
	EVT_BACKEND_THREAD(wxID_ANY, BACKEND_QUESTION, mainframe::OnBackendQuestion)
	EVT_MENU(mainframe::ID_ACCEL_DELETE, mainframe::OnAccelerator)
	EVT_MENU(mainframe::ID_ACCEL_REFRESH, mainframe::OnAccelerator)
	EVT_MENU(mainframe::ID_ACCEL_CUT, mainframe::OnAccelerator)
	EVT_MENU(mainframe::ID_ACCEL_COPY, mainframe::OnAccelerator)
	EVT_MENU(mainframe::ID_ACCEL_PASTE, mainframe::OnAccelerator)
wxEND_EVENT_TABLE()

/**********************************************************************\
 * Private functions
\**********************************************************************/

void mainframe::OpenImage()
{
	wxString name = mpArchiveFile->GetName();
	size_t size = mpArchiveFile->GetSize();

	std::list<FileTree::partition> init_list;
	init_list.push_back(FileTree::partition(name, size));

	if(mpArchiveFile->Mount() != Archive::NOERROR)
	{
		wxMessageBox(MSG_MOUNT_FAILED);
		CloseImage();
		return;
	}

	uint8_t partitions = mpArchiveFile->Partitions();

	for(int i = 0; i < 4; ++i)
	{
		if(partitions & (1 << i))
		{
			name = wxString::Format("p%d", i);
			size = mpArchiveFile->GetPartitionSize(i);
			init_list.push_back(FileTree::partition(name, size));
		}
	}

	mpFileTree->InitRoot(init_list, mpArchiveFile);
	mpBackend->SetImageFile(mpArchiveFile);
}

void mainframe::CloseImage()
{
	mpFileTree->DeleteAllItems();

	if(mpArchiveFile)
	{
		mpArchiveFile->UMount();
		mpBackend->SetImageFile(0);
		delete mpArchiveFile;
		mpArchiveFile = 0;
	}

	if(mpPhysicalDevice)
	{
		delete mpPhysicalDevice;
		mpPhysicalDevice = 0;
	}
}

void mainframe::PasteFile()
{
	wxString* path = new wxString();
	wxString prefix;
	wxTreeListItems clipboard = mpFileTree->GetStoredSelection();
	wxTreeListItem selection;
	wxTreeListItem localRoot;
	std::list<ls_ent>* ls = new std::list<ls_ent>();
	int nPrefixLen;

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		goto exit_error;
	}

	if(!clipboard.size())
		goto exit_error;

	selection = mpFileTree->GetSingleSelectionWithDefault();
	if(!selection.IsOk())
	{
		wxMessageBox(MSG_INVALID_SELECTION);
		goto exit_error;
	}

	if(mpFileTree->IsFile(selection))
		selection = mpFileTree->GetItemParent(selection);

	*path = mpFileTree->GetPathFromTree(selection);

	mpFileTree->DenestSelection(clipboard);
	localRoot = mpFileTree->GetLowestCommonAncestor(clipboard);
	if(mpFileTree->IsFile(localRoot) || mpFileTree->IsDirectory(localRoot))
		localRoot = mpFileTree->GetItemParent(localRoot);

	prefix = mpFileTree->GetPathFromTree(localRoot);
	nPrefixLen = prefix.size();

	for(unsigned int i = 0; i < clipboard.size(); ++i)
	{
		wxString selpath = mpFileTree->GetPathFromTree(clipboard[i]);

		// Root is no valid selection for copy/cut/paste
		if(selpath == Ext4ImgFileArchive::ROOTNAME)
		{
			wxMessageBox(MSG_INVALID_SELECTION);
			goto exit_error;
		}
		else if(mpArchiveFile->ListDirectory(selpath, *ls, true)
			!= Archive::NOERROR)
		{
			wxMessageBox(MSG_LIST_DIR_FAILED);
			goto exit_error;
		}
	}
	ls->sort();

	// wxPD_AUTO is required for OSX
	mpProgressDlg = new wxProgressDialog
		(MSG_IN_PROGRESS, MSG_IN_PROGRESS_TITLE, ls->size(), this,
		wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

	if(mCutClipboard)
		mpBackend->MoveFiles(path, ls, nPrefixLen);
	else
		mpBackend->CopyFiles(path, ls, nPrefixLen);

	// Continues in OnCmdComplete()
	return;

exit_error:
	delete path;
	delete ls;

	mpFileTree->ClearStoredSelection();
}

void mainframe::DeleteFile()
{
	wxString confirmPaths(MSG_CONFIRM_DELETE);
	wxTreeListItems selection;
	std::list<ls_ent>* ls = new std::list<ls_ent>();

	if(!mpArchiveFile)
	{
		wxMessageBox(MSG_NO_IMAGE_OPENED);
		return;
	}

	selection = mpFileTree->GetSelectionWithDefault();
	mpFileTree->DenestSelection(selection);

	for(unsigned int i = 0; i < selection.size(); ++i)
	{
		wxString selpath = mpFileTree->GetPathFromTree(selection[i]);
		// Don't delete root or partitions
		if(mpArchiveFile->IsRootPath(selpath))
		{
			if(selection.size() == 1)
			{
				wxMessageBox(MSG_INVALID_SELECTION);
				return;
			}
			else
				continue;
		}

		if(mpArchiveFile->ListDirectory(selpath, *ls, true) != Archive::NOERROR)
		{
			wxMessageBox(MSG_LIST_DIR_FAILED);
			return;
		}
		// Don't show /loop0/ on confirm dialog
		confirmPaths.append(selpath.begin() + selpath.find_first_of('/', 1),
			selpath.end());
		confirmPaths += '\n';
	}

	if(wxMessageBox(confirmPaths, MSG_CONFIRM_DELETE_TITLE, wxYES_NO) == wxYES)
	{
		ls->sort();
		ls->reverse();

		// wxPD_AUTO is required for OSX
		mpProgressDlg = new wxProgressDialog
			(MSG_IN_PROGRESS, MSG_IN_PROGRESS_TITLE, ls->size(), this,
			wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

		mpBackend->DeleteFiles(ls);
		// Continues in OnCmdComplete()
	}
}
