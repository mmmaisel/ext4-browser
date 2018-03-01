/**********************************************************************\
 * ext4-browser
 * writeImageDlg.cpp
 *
 * Write image dialog
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
#include "writeImageDlg.h"
#include "resource/messages.h"

writeImageDialog::writeImageDialog(wxWindow* parent, wxWindowID id,
	const wxString& title, wxString* pDevices, size_t deviceCount,
	const wxPoint& pos)
#ifdef __linux
	: wxDialog(parent, id, title, pos, wxSize(410, 120))
#else
	: wxDialog(parent, id, title, pos, wxSize(410, 150))
#endif
{
	// Buttons (y = 80)
	new wxButton(this, wxID_OK, MSG_IMGDLG_WRITE_IMG, wxPoint(100, 80));
	new wxButton(this, wxID_CANCEL, MSG_CANCEL, wxPoint(210, 80));

	// first column (text, x = 10)
	new wxStaticText(this, wxID_ANY, MSG_IMGDLG_IMGFILE, wxPoint(10, 10));
	new wxStaticText(this, wxID_ANY, MSG_IMGDLG_DISK, wxPoint(10, 45));

	// second column (input, x = 90)
	mpFileName		= new wxTextCtrl
		(this, wxID_ANY, wxEmptyString, wxPoint(90, 10), wxSize(210, 24));
	mpSelectDisk	= new wxComboBox
		(this, wxID_ANY, wxEmptyString, wxPoint(90, 45), wxSize(270, 25), deviceCount, pDevices);

	// file dialog button (x = 300)
	new wxButton(this, wxID_HIGHEST + 1, MSG_NEW_IMG_SELECT_FILE, wxPoint(310, 10));
}

void writeImageDialog::OnOk(wxCommandEvent& event)
{
	mImgFilename = mpFileName->GetValue();
	mSelection  = mpSelectDisk->GetSelection();

	if(mImgFilename.IsEmpty() || mSelection < 0)
	{
		wxMessageBox(MSG_INVALID_INPUT, MSG_ERROR, wxICON_ERROR | wxOK);
	}
	else
	{
		EndModal(wxID_OK);
	}
}

void writeImageDialog::OnFileDlg(wxCommandEvent& event)
{
	wxFileDialog filedlg
		(this, wxFileSelectorPromptStr, "", "", SELECTOR_IMG_FILES, wxFD_OPEN);

	if(filedlg.ShowModal() == wxID_CANCEL)
		return;

	mpFileName->SetValue(filedlg.GetPath());
}

wxBEGIN_EVENT_TABLE(writeImageDialog, wxDialog)
	EVT_BUTTON(wxID_OK, writeImageDialog::OnOk)
	EVT_BUTTON(wxID_HIGHEST + 1, writeImageDialog::OnFileDlg)
wxEND_EVENT_TABLE()
