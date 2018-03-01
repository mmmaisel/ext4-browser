/**********************************************************************\
 * ext4-browser
 * newImageDialog.cpp
 *
 * Image file creation dialog
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
#include "newImageDlg.h"
#include "resource/messages.h"

newImageDialog::newImageDialog(wxWindow* parent, wxWindowID id,
	const wxString& title, Ext4ImgFileArchive* pImgFile, const wxPoint& pos)
#ifdef __linux
	: wxDialog(parent, id, title, pos, wxSize(400, 150))
#else
	: wxDialog(parent, id, title, pos, wxSize(400, 170))
#endif
	, mpImgFile(pImgFile)
{
	// get all fs types
	const BYTE CHOISE_COUNT = 3;
	wxString choises[CHOISE_COUNT] =
	{
		"ext2", "ext3", "ext4"
	};

	// Buttons (y = 110)
	new wxButton(this, wxID_OK, MSG_OK, wxPoint(100, 110));
	new wxButton(this, wxID_CANCEL, MSG_CANCEL, wxPoint(190, 110));

	// first column (text, x = 10)
	new wxStaticText(this, wxID_ANY, MSG_NEW_IMG_PATH, wxPoint(10, 10));
	new wxStaticText(this, wxID_ANY, MSG_NEW_IMG_TYPE, wxPoint(10, 40));
	new wxStaticText(this, wxID_ANY, MSG_NEW_IMG_SIZE_MIB, wxPoint(10, 70));

	// second column (input, x = 80)
	mpFileName		= new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(80, 10), wxSize(210, 24));
	mpSelectType	= new wxComboBox(this, wxID_ANY, wxEmptyString, wxPoint(80, 40), wxSize(120, 25), CHOISE_COUNT, choises);
	mpSizeMiB		= new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(80, 70), wxSize(120, 25));

	mpSelectType->SetStringSelection("ext4");

	// file dialog button (x = 300)
	new wxButton(this, wxID_HIGHEST + 1, MSG_NEW_IMG_SELECT_FILE, wxPoint(300, 10));
}

void newImageDialog::OnOk(wxCommandEvent& event)
{
	ULONG64 size_MiB = 0;
	wxString path    = mpFileName->GetValue();
	BYTE type        = 0;
	std::string sel  = mpSelectType->GetStringSelection().ToStdString();
#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
	mpSizeMiB->GetValue().ToULongLong(&size_MiB, 10);
#else
	mpSizeMiB->GetValue().ToULong(&size_MiB, 10);
#endif

	if(sel == "ext2")
		type = F_SET_EXT2;
	else if(sel == "ext3")
		type = F_SET_EXT3;
	else if(sel == "ext4")
		type = F_SET_EXT4;

	if(path.IsEmpty() || type == 0 || size_MiB <= 0)
	{
		wxMessageBox(MSG_INVALID_INPUT, MSG_ERROR, wxICON_ERROR | wxOK);
	}
	else
	{
		if(mpImgFile->Create(path, size_MiB, type) != Ext4ImgFileArchive::NOERROR)
		{
			wxMessageBox(MSG_CREATE_IMAGE_FAILED);
			EndModal(wxID_CANCEL);
			return;
		}
		EndModal(wxID_OK);
	}
}

void newImageDialog::OnFileDlg(wxCommandEvent& event)
{
	wxFileDialog filedlg
		(this, wxFileSelectorPromptStr, "", "", SELECTOR_IMG_FILES,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if(filedlg.ShowModal() == wxID_CANCEL)
		return;

	mpFileName->SetValue(filedlg.GetPath());
}

wxBEGIN_EVENT_TABLE(newImageDialog, wxDialog)
	EVT_BUTTON(wxID_OK, newImageDialog::OnOk)
	EVT_BUTTON(wxID_HIGHEST + 1, newImageDialog::OnFileDlg)
wxEND_EVENT_TABLE()
