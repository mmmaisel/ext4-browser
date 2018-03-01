/**********************************************************************\
 * ext4-browser
 * writeImageDlg.h
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
#pragma once

class writeImageDialog
	: public wxDialog
{
	public:
		writeImageDialog(wxWindow* parent, wxWindowID id, const wxString& title,
			wxString* pDevices, size_t deviceCount,
			const wxPoint& pos = wxDefaultPosition);
		writeImageDialog(const writeImageDialog&) = delete;
		writeImageDialog(writeImageDialog&&) = delete;

		wxString GetImageFilename() const {return mImgFilename;}
		int GetSelectedDisk() const {return mSelection;}

	private:
		void OnOk(wxCommandEvent& event);
		void OnFileDlg(wxCommandEvent& event);
		//void OnUpdateUI(wxUpdateUIEvent& event);
		wxDECLARE_EVENT_TABLE();

		wxTextCtrl* mpFileName;
		wxComboBox* mpSelectDisk;

		wxString mImgFilename;
		int mSelection;
};
