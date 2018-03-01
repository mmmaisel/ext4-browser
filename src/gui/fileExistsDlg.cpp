/**********************************************************************\
 * ext4-browser
 * fileExistsDlg.cpp
 *
 * File exists dialog
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
#include "fileExistsDlg.h"
#include "archive.h"
#include "resource/messages.h"

fileExistsDialog::fileExistsDialog(wxWindow* parent, wxWindowID id,
	const wxString& title, bool isDir, wxString& filename, const wxPoint& pos)
	: wxDialog(parent, id, title, pos)
	, mIsDir(isDir)
{
	const int BORDER_WIDTH = 15;
	int vSizerFlags = wxEXPAND | wxTOP | wxALIGN_CENTER;
	int hSizerFlags = wxEXPAND | wxLEFT | wxRIGHT | wxALIGN_CENTER;

	wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer* hSizerButton = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* hSizerText = new wxBoxSizer(wxHORIZONTAL);

	hSizerButton->AddStretchSpacer();
	hSizerButton->Add
		(new wxButton(this, ID_SKIP, MSG_SKIP), 0, hSizerFlags, BORDER_WIDTH);
	hSizerButton->Add
		(new wxButton(this, ID_SKIP_ALL, MSG_SKIP_ALL), 0, hSizerFlags, BORDER_WIDTH);

	if(mIsDir)
	{
		hSizerButton->Add(new wxButton(this, ID_OVERWRITE, MSG_MERGE_DIR),
			0, hSizerFlags, 10);
		hSizerButton->Add(new wxButton(this, ID_OVERWRITE_ALL, MSG_MERGE_DIR_ALL),
			0, hSizerFlags, 10);
		hSizerText->Add(new wxStaticText(this, wxID_ANY,
			wxString::Format(MSG_DIR_EXISTS, filename)), 0, hSizerFlags, BORDER_WIDTH);
	}
	else
	{
		hSizerButton->Add(new wxButton(this, ID_OVERWRITE, MSG_OVERWRITE),
			0, hSizerFlags, 10);
		hSizerButton->Add(new wxButton(this, ID_OVERWRITE_ALL, MSG_OVERWRITE_ALL),
			0, hSizerFlags, 10);
		hSizerText->Add(new wxStaticText(this, wxID_ANY,
			wxString::Format(MSG_FILE_EXISTS, filename)), 0, hSizerFlags, BORDER_WIDTH);
	}

	hSizerButton->AddStretchSpacer();
	vSizer->Add(hSizerText, 0, vSizerFlags, BORDER_WIDTH);
	vSizer->Add(hSizerButton, 0, vSizerFlags | wxBOTTOM, BORDER_WIDTH);
	SetSizerAndFit(vSizer);
}

void fileExistsDialog::OnSkip(wxCommandEvent& event)
{
	if(mIsDir)
		EndModal(Archive::ANSWER_SKIP_DIR);
	else
		EndModal(Archive::ANSWER_SKIP);
}

void fileExistsDialog::OnSkipAll(wxCommandEvent& event)
{
	if(mIsDir)
		EndModal(Archive::ANSWER_SKIP_DIR_ALL);
	else
		EndModal(Archive::ANSWER_SKIP_ALL);
}

void fileExistsDialog::OnOverwrite(wxCommandEvent& event)
{
	if(mIsDir)
		EndModal(Archive::ANSWER_MERGE);
	else
		EndModal(Archive::ANSWER_OVERWRITE);
}

void fileExistsDialog::OnOverwriteAll(wxCommandEvent& event)
{
	if(mIsDir)
		EndModal(Archive::ANSWER_MERGE_ALL);
	else
		EndModal(Archive::ANSWER_OVERWRITE_ALL);
}

void fileExistsDialog::OnClose(wxCloseEvent& event)
{
	EndModal(Archive::ANSWER_SKIP);
}

wxBEGIN_EVENT_TABLE(fileExistsDialog, wxDialog)
	EVT_BUTTON(ID_SKIP, fileExistsDialog::OnSkip)
	EVT_BUTTON(ID_SKIP_ALL, fileExistsDialog::OnSkipAll)
	EVT_BUTTON(ID_OVERWRITE, fileExistsDialog::OnOverwrite)
	EVT_BUTTON(ID_OVERWRITE_ALL, fileExistsDialog::OnOverwriteAll)
	EVT_CLOSE(fileExistsDialog::OnClose)
wxEND_EVENT_TABLE()
