/**********************************************************************\
 * ext4-browser
 * fileTree.cpp
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
#include "stdafx.h"
#include "fileTree.h"
#include "resource/messages.h"

FileTree::FileTree(wxWindow* parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size,
		long style, const wxString& name)
	: wxTreeListCtrl(parent, id, pos, size, style, name)
	, mpFileImageList(0)
	, mpArchiveFile(0)
{
	mpFileImageList = new wxImageList(16, 16);

	// The order must be the same as for the enum elements.
	mpFileImageList->Add(wxIcon(L"icons/file-small.png", wxBITMAP_TYPE_PNG));
	mpFileImageList->Add(wxIcon(L"icons/folder-small.png", wxBITMAP_TYPE_PNG));
	mpFileImageList->Add(wxIcon(L"icons/disk-small.png", wxBITMAP_TYPE_PNG));
	mpFileImageList->Add(wxIcon(L"icons/partition-small.png", wxBITMAP_TYPE_PNG));

	SetImageList(mpFileImageList);

	AppendColumn(MSG_FILETREE_FILE,
		wxCOL_WIDTH_DEFAULT,
		wxALIGN_LEFT,
		wxCOL_RESIZABLE | wxCOL_SORTABLE);
	AppendColumn(MSG_FILETREE_TYPE,
		WidthFor("very long file typename"),
		wxALIGN_RIGHT,
		wxCOL_RESIZABLE | wxCOL_SORTABLE);
	AppendColumn(MSG_FILETREE_SIZE,
		WidthFor("1,000,000 KiB"),
		wxALIGN_RIGHT,
		wxCOL_RESIZABLE | wxCOL_SORTABLE);

	SetItemComparator(&mComparator);
}

FileTree::~FileTree()
{
	if(mpFileImageList)
		delete mpFileImageList;
}

bool FileTree::IsDirectory(wxTreeListItem item)
{
	itemData* pType = static_cast<itemData*>(GetItemData(item));
	if(pType)
		return pType->GetType() == TYPE_DIR;
	return false;
}

bool FileTree::IsFile(wxTreeListItem item)
{
	itemData* pType = static_cast<itemData*>(GetItemData(item));
	if(pType)
		return pType->GetType() == TYPE_FILE;
	return false;
}

wxTreeListItem FileTree::GetLowestCommonAncestor(wxTreeListItems& items)
{
	wxTreeListItems ancestors;
	wxTreeListItem item;
	wxTreeListItem localRoot;
	wxTreeListItem root = GetRootItem();
	root = GetFirstChild(root);

	if(items.size() == 0)
		return wxTreeListItem();

	if(items.size() == 1)
		return items[0];

	// Lowest common ancestor algorithm from:
	// http://bio4j.com/blog/2012/02/finding-the-lowest-common-ancestor-of-a-set-of-ncbi-taxonomy-nodes-with-bio4j/

	// List all ancestors of an arbitrary node
	item = items[0];
	while(item != root)
	{
		ancestors.push_back(item);
		item = GetItemParent(item);
	}
	std::reverse(ancestors.begin(), ancestors.end());

	for(size_t i = 1; i < items.size(); ++i)
	{
		item = items[i];
		// Find intersection of item parents with ancestor list
		while(item != root)
		{
			auto it = std::find(ancestors.begin(), ancestors.end(), item);
			if(it != ancestors.end())
			{
				// Delete everything higher than intersection
				ancestors.erase(it+1, ancestors.end());
				break;
			}
			item = GetItemParent(item);
		}

		// Skip further iterations if least common ancestor is already found
		if(ancestors.size() == 1)
			break;
	}

	// Last element is lowest common ancestor
	localRoot = *ancestors.rbegin();

	// If root is file, use parent node.
	if(IsFile(localRoot))
		localRoot = GetItemParent(localRoot);

	return localRoot;
}

wxTreeListItem FileTree::GetSingleSelectionWithDefault()
{
	wxTreeListItems selection = GetSelectionWithDefault();
	wxTreeListItem root = GetRootItem();
	root = GetFirstChild(root); // img

	if(selection.size() > 1)
		return wxTreeListItem();

	return selection[0];
}

wxTreeListItems FileTree::GetSelectionWithDefault()
{
	wxTreeListItems selection;
	wxTreeListCtrl::GetSelections(selection);

	if(selection.size() == 0)
	{
		wxTreeListItem root = GetRootItem();
		root = GetFirstChild(root); // archive name

		if(mpArchiveFile->GetPartitionCount() == 1)
			selection.push_back(GetFirstChild(root));
		else
			selection.push_back(root);
	}
	return selection;
}

void FileTree::DenestSelection(wxTreeListItems& selection)
{
	wxTreeListItem root = GetRootItem();
	root = GetFirstChild(root);

	// Remove nested selections
	for(unsigned int i = 0; i < selection.size(); ++i)
	{
		wxTreeListItem item = selection[i];
		while(item != root)
		{
			for(unsigned int j = 0; j < selection.size(); ++j)
			{
				if(i == j)
					continue;

				if(item == selection[j])
				{
					selection.erase(selection.begin()+i);
					--i; --j; // selections is a vector so move up indices
				}
			}
			item = GetItemParent(item);
		}
	}
}

void FileTree::UniqueParents(wxTreeListItems& selection)
{
	for(unsigned int i = 0; i < selection.size(); ++i)
	{
		selection[i] = GetItemParent(selection[i]);
		for(unsigned int j = 0; j < i; ++j)
		{
			if(selection[i] == selection[j])
			{
				selection.erase(selection.begin()+i);
				--i; --j; // selections is a vector so move up indices
			}
		}
	}
}

wxString FileTree::GetPathFromTree(wxTreeListItem item)
{
	wxString path = mpArchiveFile->GetRootName();
	int ROOTNAME_LEN = path.length();
	wxTreeListItem root = GetRootItem();
	root = GetFirstChild(root);
	path.RemoveLast();	// remove trailing '/'

	bool isFile = IsFile(item);

	// Invalid items can occur when double clicking on header fields on OSX.
	while(item != root && item.IsOk())
	{
		path.insert(ROOTNAME_LEN-1, GetItemText(item, TREE_COL_NAME));
		item = GetItemParent(item);
		path.insert(ROOTNAME_LEN-1, L'/');
	}

	if(!isFile)
		path.append(L'/');
	return path;
}

wxTreeListItem FileTree::GetItemFromPath(const wxString& path)
{
	wxString localName;
	wxString root = mpArchiveFile->GetRootName();
	size_t pos = root.length();
	size_t nextPos = 0;
	size_t isDirCorrection = 0;
	// item is first partition
	wxTreeListItem item = GetFirstChild(GetFirstChild(GetRootItem()));

	if(!path.StartsWith(root))
		return wxTreeListItem();

	if(path.Last() == '/')
		// Ignore trailing / in case of directories
		isDirCorrection = 1;

	while(pos < path.length()-isDirCorrection)
	{
		nextPos = path.find('/', pos);
		if(static_cast<ssize_t>(nextPos) == wxNOT_FOUND)
			nextPos = path.length();
		localName = path.substr(pos, nextPos - pos);

		while(item.IsOk())
		{
			if(localName == GetItemText(item))
			{
				// Found
				if(nextPos == path.length()-isDirCorrection)
					return item;
				// Step into dir
				item = GetFirstChild(item);
				break;
			}
			item = GetNextSibling(item);
		}
		pos = nextPos+1;
	}

	// Not found
	return wxTreeListItem();
}

void FileTree::InitRoot(const std::list<partition>& partitions, Archive* pArchive)
{
	wxTreeListItem disk;
	wxTreeListItem part_node;
	auto it = partitions.cbegin();

	// SetItemText can generate false ITEM_EXPANDING
	// events which cause an infinite listing loop.
	// These events are called from the same thread before
	// SetItemText returns (especialy on OSX).
	wxEventBlocker blocker(this, wxEVT_TREELIST_ITEM_EXPANDING);

	mpArchiveFile = pArchive;
	if(!mpArchiveFile)
		return;
	if(partitions.size() == 0)
		return;

	Freeze();

	// First partitions entry is root name
	if(partitions.size() > 0)
	{
		disk = AppendItem
			(GetRootItem(), it->first, ICON_DISK, ICON_DISK, 0);
		SetItemText(disk, TREE_COL_TYPE, MSG_FILETREE_TYPE_DISK);
		SetItemText(disk, TREE_COL_SIZE, FormatSize(it->second));
		++it;
	}

	for(BYTE i = 0; it != partitions.cend(); ++it, ++i)
	{
		part_node = AppendItem(disk, it->first, ICON_PARTITION, ICON_PARTITION, 0);
		SetItemText(part_node, TREE_COL_TYPE, MSG_FILETREE_TYPE_PARTITION);
		SetItemText(part_node, TREE_COL_SIZE, FormatSize(it->second));

		ListDirectory(part_node, false);
		Expand(disk);
		Expand(part_node);
	}
	Thaw();
}

void FileTree::ListDirectory(wxTreeListItem item, bool expand)
{
	wxString path = GetPathFromTree(item);

	// SetItemText can generate false ITEM_EXPANDING
	// events which cause an infinite listing loop.
	// These events are called from the same thread before
	// SetItemText returns (especially on OSX).
	wxEventBlocker blocker(this, wxEVT_TREELIST_ITEM_EXPANDING);

	Freeze();

	// Delete tree items
	wxTreeListItem tmpitem = GetFirstChild(item);
	while(tmpitem.IsOk())
	{
		DeleteItem(tmpitem);
		tmpitem = GetFirstChild(item);
	}

	std::list<ls_ent> ls;
	if(mpArchiveFile->ListDirectory(path, ls, false) != Archive::NOERROR)
	{
		wxMessageBox(MSG_LIST_DIR_FAILED);
		goto exit_error;
	}

	// Ignore self (first list element)
	if(ls.size())
	{
		if(ls.begin()->name == path)
			ls.pop_front();
	}

	// Order of dir entries is not defined -> sort them
	ls.sort();

	for(const auto& it : ls)
	{
		if(it.isDir)
		{
			std::list<ls_ent> tmp;
			mpArchiveFile->ListDirectory(it.name + "/", tmp, false);

			wxTreeListItem newItem = AppendItem
				(item, it.name.substr(path.length()),
				ICON_FOLDER, ICON_FOLDER, new itemData(TYPE_DIR, tmp.size()));
			SetItemText(newItem, TREE_COL_TYPE, MSG_FILETREE_TYPE_DIR);
			// Item client data is freed automatically by wxTreeListCtrl

			if(tmp.size() > 1) // 1st element is self
			{
				// If dir is not empty, add dummy item to make it expandable
				AppendItem(newItem, "", ICON_FOLDER, ICON_FOLDER, 0);
			}
#if defined(_WIN32) || defined(_WIN64)
			// Windows does not support C99 %zu format string
			SetItemText(newItem, TREE_COL_SIZE, wxString::Format("%Iu Objects", tmp.size()-1));
#else
			SetItemText(newItem, TREE_COL_SIZE, wxString::Format("%zu Objects", tmp.size()-1));
#endif
		}
		else
		{
			wxTreeListItem newItem = AppendItem
				(item, it.name.substr(path.length()),
				ICON_NORMAL_FILE, ICON_NORMAL_FILE, new itemData(TYPE_FILE, it.size));
			SetItemText(newItem, TREE_COL_TYPE, MSG_FILETREE_TYPE_FILE);
			SetItemText(newItem, TREE_COL_SIZE, FormatSize(it.size));
			// Item client data is freed automatically by wxTreeListCtrl
		}
	}

exit_error:
	if(expand)
		Expand(item);
	Thaw();
}

void FileTree::RefreshTree()
{
	std::list<wxString> expandedPaths;
	wxTreeListItem localRoot = GetFirstChild(GetRootItem());
	wxTreeListItem item = GetFirstChild(localRoot);
	wxTreeListItem tmpItem;

	// Build list of all expanded paths
	while(item != localRoot)
	{
		if(IsExpanded(item))
		{
			expandedPaths.push_back(GetPathFromTree(item));
			tmpItem = GetFirstChild(item);
		}
		else
			tmpItem = GetNextSibling(item);

		// No next sibling: step up
		while(!tmpItem.IsOk())
		{
			item = GetItemParent(item);
			if(item == localRoot)
				break;
			// direct parent item is already listed
			tmpItem = GetNextSibling(item);
		}

		if(item != localRoot)
			item = tmpItem;
	}

	// List all expanded paths
	for(const auto& path : expandedPaths)
	{
		item = GetItemFromPath(path);
		// Some paths may be deleted
		if(item.IsOk())
			ListDirectory(item, true);
	}
}

void FileTree::OnItemExpanding(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();
	if(item.IsOk())
	{
		wxTreeListItem localRoot = GetFirstChild(GetRootItem());
		if(item != localRoot)
			ListDirectory(item, false);
	}
}

#ifdef _DEBUG
// List selected file/directory for debugging
void FileTree::OnItemActivated(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();
	if(!item.IsOk())
		return;

	// Can't get any paths no archive is opened.
	if(!mpArchiveFile)
		return;

	wxString path = GetPathFromTree(item);
	std::list<ls_ent> ls;

	mpArchiveFile->ListDirectory(path, ls, true);

	wxString files;
	for(const auto& it : ls)
	{
		files.append(it.name);
		files.append(L'\n');
	}

	// Don't show empty message boxes
	if(ls.size())
		wxMessageBox(files);
}
#endif

wxBEGIN_EVENT_TABLE(FileTree, wxTreeListCtrl)
	EVT_TREELIST_ITEM_EXPANDING(wxID_ANY, FileTree::OnItemExpanding)
#ifdef _DEBUG
	EVT_TREELIST_ITEM_ACTIVATED(wxID_ANY, FileTree::OnItemActivated)
#endif
wxEND_EVENT_TABLE()

wxString FileTree::FormatSize(size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
	// Windows does not support C99 %zu format string
	if(size < 1024)
		return wxString::Format(wxT("%Iu B"), size);
	else if(size < 1024*1024)
		return wxString::Format(wxT("%Iu KiB"), size / 1024);
	else if(size < 1024*1024*1024)
		return wxString::Format(wxT("%Iu MiB"), size / 1024 / 1024);
	else if(size < 1024ULL*1024ULL*1024ULL*1024ULL)
		return wxString::Format(wxT("%Iu GiB"), size / 1024 / 1024 / 1024);
#else
	if(size < 1024)
		return wxString::Format(wxT("%zu B"), size);
	else if(size < 1024*1024)
		return wxString::Format(wxT("%zu KiB"), size / 1024);
	else if(size < 1024*1024*1024)
		return wxString::Format(wxT("%zu MiB"), size / 1024 / 1024);
	else if(size < 1024ULL*1024ULL*1024ULL*1024ULL)
		return wxString::Format(wxT("%zu GiB"), size / 1024 / 1024 / 1024);
#endif
	return wxString("Error");
}

int FileTree::itemComparator::Compare
	(wxTreeListCtrl* treelist, unsigned int column,
	wxTreeListItem item1, wxTreeListItem item2)
{
	itemData* pData1 = static_cast<itemData*>(treelist->GetItemData(item1));
	itemData* pData2 = static_cast<itemData*>(treelist->GetItemData(item2));
	wxString  name1  = treelist->GetItemText(item1, TREE_COL_NAME);
	wxString  name2  = treelist->GetItemText(item2, TREE_COL_NAME);

	switch(column)
	{
		case TREE_COL_NAME:
		case TREE_COL_TYPE:
		{
			// Sort alphabetically, directories before files
			if(pData1->GetType() == pData2->GetType())
				return name1.CmpNoCase(name2);
			else if(pData1->GetType() == TYPE_DIR)
				return -1;
			else
				return 1;
		}

		case TREE_COL_SIZE:
		{
			// Sort by size, directories by member count, directories on top
			if(pData1->GetType() == pData2->GetType())
				return pData1->GetSize() - pData2->GetSize();
			else if(pData1->GetType() == TYPE_DIR)
				return -1;
			else
				return 1;
		}
	}
	return 0;
}
