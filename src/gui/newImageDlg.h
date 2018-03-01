/**********************************************************************\
 * ext4-browser
 * newImageDlg.h
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
#pragma once

#include "ext4ImgArchive.h"

class newImageDialog
	: public wxDialog
{
	public:
		newImageDialog(wxWindow* parent, wxWindowID id, const wxString& title,
			Ext4ImgFileArchive* pImgFile, const wxPoint& pos = wxDefaultPosition);
		newImageDialog(const newImageDialog&) = delete;
		newImageDialog(newImageDialog&&) = delete;

		enum eIDs : WORD {ID_ERROR = wxID_HIGHEST + 1};

	private:
		void OnOk(wxCommandEvent& event);
		void OnFileDlg(wxCommandEvent& event);
		//void OnUpdateUI(wxUpdateUIEvent& event);
		wxDECLARE_EVENT_TABLE();

		wxTextCtrl* mpFileName;
		wxComboBox* mpSelectType;
		wxTextCtrl* mpSizeMiB;

		Ext4ImgFileArchive* mpImgFile;
};
