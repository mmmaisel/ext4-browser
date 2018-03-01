/**********************************************************************\
 * ext4-browser
 * fileTree.h
 *
 * File tree control
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

#include "archive.h"

class FileTree
	: public wxTreeListCtrl
{
	public:
		FileTree(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxTL_DEFAULT_STYLE,
			const wxString& name = wxTreeListCtrlNameStr);
		FileTree(const FileTree&) = delete;
		FileTree(FileTree&&) = delete;
		virtual ~FileTree();

		bool IsDirectory(wxTreeListItem item);
		bool IsFile(wxTreeListItem item);

		wxTreeListItem GetLowestCommonAncestor(wxTreeListItems& items);
		wxTreeListItem GetSingleSelectionWithDefault();
		wxTreeListItems GetSelectionWithDefault();
		void DenestSelection(wxTreeListItems& selection);
		void UniqueParents(wxTreeListItems& selection);
		wxString GetPathFromTree(wxTreeListItem item);
		wxTreeListItem GetItemFromPath(const wxString& path);

		typedef std::pair<wxString, size_t> partition;

		void InitRoot(const std::list<partition>& partitions, Archive* pArchive);
		void ListDirectory(wxTreeListItem item, bool expand);
		void RefreshTree();

		inline void StoreSelection()
		{
			wxTreeListItems items;
			GetSelections(items);
			mClipboard.clear();
			for(const auto& it : items)
				mClipboard.push_back(GetPathFromTree(it));
		}

		inline wxTreeListItems GetStoredSelection()
		{
			wxTreeListItems items;
			for(const auto& it : mClipboard)
			{
				wxTreeListItem x = GetItemFromPath(it);
				if(x.IsOk())
					items.push_back(x);
			}
			return items;
		}

		inline void ClearStoredSelection()
			{ mClipboard.clear(); }

		enum
		{
			TREE_COL_NAME,
			TREE_COL_TYPE,
			TREE_COL_SIZE,
			TREE_COL_TIMESTAMP,
			TREE_COL_PERMISSION
		};

		void OnItemExpanding(wxTreeListEvent& event);
#ifdef _DEBUG
		void OnItemActivated(wxTreeListEvent& event);
#endif

	private:
		wxImageList* mpFileImageList;
		Archive* mpArchiveFile;

		// Clipboard
		std::list<wxString> mClipboard;

		enum eIcons
		{
			ICON_NORMAL_FILE,
			ICON_FOLDER,
			ICON_DISK,
			ICON_PARTITION
		};

		wxDECLARE_EVENT_TABLE();

		wxString FormatSize(size_t size);

		class itemData
			: public wxClientData
		{
			public:
				itemData() : mType(0), mSize(0) {}
				itemData(int type, size_t size)
					: mType(type), mSize(size) {}

				void SetType(int type) { mType = type; }
				void SetSize(size_t size) { mSize = size; }
				int GetType() const { return mType; }
				size_t GetSize() const { return mSize; }

			private:
				int mType;
				size_t mSize;
		};

		enum eItemType
		{
			TYPE_FILE,
			TYPE_DIR,
			TYPE_OTHER
		};

		class itemComparator
			: public wxTreeListItemComparator
		{
			public:
				virtual int Compare(wxTreeListCtrl* treelist, unsigned int column,
					wxTreeListItem item1, wxTreeListItem item2) override;
		};

		itemComparator mComparator;
};

