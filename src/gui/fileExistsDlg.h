/**********************************************************************\
 * ext4-browser
 * fileExistsDlg.h
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
#pragma once

class fileExistsDialog
	: public wxDialog
{
	public:
		fileExistsDialog(wxWindow* parent, wxWindowID id, const wxString& title,
			bool isDir, wxString& filename, const wxPoint& pos = wxDefaultPosition);
		fileExistsDialog(const fileExistsDialog&) = delete;
		fileExistsDialog(fileExistsDialog&&) = delete;

		enum
		{
			ID_SKIP = wxID_HIGHEST + 1,
			ID_SKIP_ALL,
			ID_OVERWRITE,
			ID_OVERWRITE_ALL
		};

	private:
		void OnSkip(wxCommandEvent& event);
		void OnSkipAll(wxCommandEvent& event);
		void OnOverwrite(wxCommandEvent& event);
		void OnOverwriteAll(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		wxDECLARE_EVENT_TABLE();

		bool mIsDir;
};
