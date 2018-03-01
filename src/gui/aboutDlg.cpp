/**********************************************************************\
 * ext4-browser
 * aboutDlg.cpp
 *
 * About Dialog
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
#include "aboutDlg.h"
#include "resource/messages.h"

aboutDialog::aboutDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos)
	: wxDialog(parent, id, title, pos)
{
	wxStaticText* pCopyright;
	wxStaticText* pCaption;
	wxFont captionFont(22, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	wxFont copyrightFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer* borders = new wxBoxSizer(wxHORIZONTAL);

	pCaption = new wxStaticText(this, wxID_ANY, "ext4-browser");
	pCaption->SetFont(captionFont);
	sizer->Add(pCaption, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	sizer->Add(new wxStaticText(this, wxID_ANY, "ext4 disk image browser and editor"),
		0, wxALIGN_CENTER | wxBOTTOM, 2);
	sizer->Add(new wxStaticText(this, wxID_ANY, "Version 1.0"),
		0, wxALIGN_CENTER | wxBOTTOM, 10);

	sizer->Add(new wxStaticText(this, wxID_ANY,
		L"Copyright © 2017 - Max Maisel - https://github.com/mmmaisel/ext4-browser"),
		0, wxALIGN_CENTER | wxBOTTOM, 2);
	sizer->Add(new wxStaticText(this, wxID_ANY,
		L"licensed under the GNU GPL version 3 or later"),
		0, wxALIGN_CENTER | wxBOTTOM, 15);

	sizer->Add(new wxStaticText(this, wxID_ANY,
		L"Portions of this software are copyrighted by the parties listed below:"),
		0, wxALIGN_LEFT | wxBOTTOM, 10);

	pCopyright = new wxStaticText(this, wxID_ANY,
		L"lwext4, developed by the lwext4 Team - https://github.com/gkostka/lwext4\n"
		L"\tlicensed under GNU GPL version 2 or later");
	pCopyright->SetFont(copyrightFont);
	sizer->Add(pCopyright, 0, wxALIGN_LEFT | wxBOTTOM, 5);

	pCopyright = new wxStaticText(this, wxID_ANY,
		L"ROSA ImageWriter, developed by Konstantin Vlasov, ROSA\n"
		L"\thttp://wiki.rosalab.com/en/index.php/ROSA_ImageWriter\n"
		L"\tlicensed under GNU GPL version 3");
	pCopyright->SetFont(copyrightFont);
	sizer->Add(pCopyright, 0, wxALIGN_LEFT | wxBOTTOM, 5);

	pCopyright = new wxStaticText(this, wxID_ANY,
		L"wxWidgets, developed by the wxWidgets Team - http://wxwidgets.org\n"
		L"\tlicensed under the wxWidgets Library Licence");
	pCopyright->SetFont(copyrightFont);
	sizer->Add(pCopyright, 0, wxALIGN_LEFT | wxBOTTOM, 5);

	pCopyright = new wxStaticText(this, wxID_ANY,
		L"Icons used/modified from http://openiconlibrary.sourceforge.net\n"
		L"\tlicensed under the GNU LGPL-2.1\n"
		L"\tdetailed information can be found in the file \"iconLicenses.txt\"");
	pCopyright->SetFont(copyrightFont);
	sizer->Add(pCopyright, 0, wxALIGN_LEFT | wxBOTTOM, 15);

	sizer->Add(new wxButton(this, wxID_CANCEL, MSG_CLOSE),
		0, wxALIGN_CENTER | wxBOTTOM, 15);

	borders->AddSpacer(15);
	borders->Add(sizer);
	borders->AddSpacer(15);

	SetSizerAndFit(borders);
}
