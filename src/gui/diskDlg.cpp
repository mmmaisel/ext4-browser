/**********************************************************************\
 * ext4-browser
 * diskDlg.cpp
 *
 * Disk selection dialog
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
#include "diskDlg.h"
#include "resource/messages.h"

diskSelectionDialog::diskSelectionDialog(wxWindow* parent, wxWindowID id,
	const wxString& title, wxString* pDevices, size_t deviceCount,
	const wxPoint& pos)
#ifdef __linux
	: wxDialog(parent, id, title, pos, wxSize(370, 90))
#else
	: wxDialog(parent, id, title, pos, wxSize(370, 120))
#endif
{
	// Buttons
	new wxButton(this, wxID_OK, MSG_OK, wxPoint(85, 50));
	new wxButton(this, wxID_CANCEL, MSG_CANCEL, wxPoint(195, 50));

	// first column (text, x = 10)
	new wxStaticText(this, wxID_ANY, MSG_SELECT_DISK, wxPoint(10, 10));

	// second column (input, x = 90)
	mpSelectDisk	= new wxComboBox
		(this, wxID_ANY, wxEmptyString, wxPoint(90, 10), wxSize(270, 25), deviceCount, pDevices);
}

void diskSelectionDialog::OnOk(wxCommandEvent& event)
{
	mSelection  = mpSelectDisk->GetSelection();
	if(mSelection < 0)
		wxMessageBox(MSG_INVALID_INPUT, MSG_ERROR, wxICON_ERROR | wxOK);
	else
		EndModal(wxID_OK);
}

wxBEGIN_EVENT_TABLE(diskSelectionDialog, wxDialog)
	EVT_BUTTON(wxID_OK, diskSelectionDialog::OnOk)
wxEND_EVENT_TABLE()
