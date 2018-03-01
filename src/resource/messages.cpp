/**********************************************************************\
 * ext4-browser
 * messages.h
 *
 * Strings and messages
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
#include "messages.h"

// Common
const char* MSG_OK = "OK";
const char* MSG_CANCEL = "Cancel";
const char* MSG_ERROR = "Error";
const char* MSG_ABOUT = "About ext4-browser";
const char* MSG_CLOSE = "Close";

// Menu
const char* MSG_MENU_PAGE_HOME		= "Home";
const char* MSG_MENU_CREATE_IMAGE	= "Create Image";
const char* MSG_MENU_OPEN_IMAGE		= "Open Image";
const char* MSG_MENU_CLOSE			= "Close";
const char* MSG_MENU_OPEN_DISK		= "Open Disk";
const char* MSG_MENU_WRITE_IMAGE	= "Write to Disk";
const char* MSG_MENU_PAGE_FILE		= "File";
const char* MSG_MENU_ADD_FILES		= "Add Files";
const char* MSG_MENU_ADD_FOLDERS	= "Add Folders";
const char* MSG_MENU_NEW_FOLDER		= "New Folder";
const char* MSG_MENU_EXTRACT		= "Extract";
const char* MSG_MENU_COPY			= "Copy";
const char* MSG_MENU_CUT			= "Cut";
const char* MSG_MENU_PASTE			= "Paste";
const char* MSG_MENU_RENAME			= "Rename";
const char* MSG_MENU_DELETE			= "Delete";

// Message boxes
const char* MSG_OP_SUCCESSFUL = "Operation completed successfully.";
const char* MSG_OP_CANCELED = "Operation canceled by user.";
const char* MSG_OP_FAILED = "Operation failed.";
const char* MSG_CONFIRM_CLOSE =
	"An Image file is currently open. Do you want to close it?";
const char* MSG_CONFIRM_CLOSE_TITLE = "Confirm close";
const char* MSG_RESTART_ROOT =
	"The application must be restarted as root to perform the selected operation.\n"
	"Do you want to restart it now?";
const char* MSG_RESTART_ROOT_TITLE = "Restart as root";
const char* MSG_SELECT_DISK = "Select disk";
const char* MSG_SELECT_DISK_TITLE = "Select disk";
const char* MSG_IN_PROGRESS = "In progress...";
const char* MSG_WRITE_IMG_PROGRESS = "Writing Image...";
const char* MSG_IN_PROGRESS_TITLE = "In progress...";
const char* MSG_MKDIR_NAME = "Folder name:";
const char* MSG_RENAME = "New name:";
const char* MSG_CONFIRM_DELETE = "Are you sure that you want to delete the following files?\n\n";
const char* MSG_CONFIRM_DELETE_TITLE = "Delete files";
const char* MSG_CONFIRM_WRITE_IMG = "Are you sure that you want to write the selected "
	"image file to\n%s?\n\nAll files on the disk will be destroyed!";
const char* MSG_CONFIRM_WRITE_IMG_TITLE = "Write image to disk";
const char* MSG_ARCHIVE_ERROR = "An error occurred while processing \"%s\"!\n\n"
	"The file may be corrupted. Do you want to continue?";

// New image dialog
const char* MSG_NEW_IMG_TITLE = "Create new image";
const char* MSG_NEW_IMG_PATH = "Path";
const char* MSG_NEW_IMG_TYPE = "Type";
const char* MSG_NEW_IMG_SIZE_MIB = "Size (MiB)";
const char* MSG_NEW_IMG_SELECT_FILE = "Select File";

// Disk dialog
const char* MSG_IMGDLG_WRITE_IMG = "Write Image";
const char* MSG_IMGDLG_IMGFILE = "Image File";
const char* MSG_IMGDLG_DISK = "Disk";

// File exists dialog
const char* MSG_SKIP = "Skip";
const char* MSG_SKIP_ALL = "Skip All";
const char* MSG_OVERWRITE = "Overwrite";
const char* MSG_OVERWRITE_ALL = "Overwrite All";
const char* MSG_MERGE_DIR = "Merge";
const char* MSG_MERGE_DIR_ALL = "Merge All";
const char* MSG_FILE_EXISTS = "A file named %s already exists.";
const char* MSG_DIR_EXISTS = "A folder named %s already exists.";
const char* MSG_EXISTS_DLG_TITLE = "File exists";

// File selectors
const char* SELECTOR_ALL_FILES = "All files|*";
const char* SELECTOR_IMG_FILES = "Image-Files (*.img)|*.img";

// File tree
const char* MSG_FILETREE_FILE = "File";
const char* MSG_FILETREE_TYPE = "Type";
const char* MSG_FILETREE_SIZE = "Size";
const char* MSG_FILETREE_TYPE_DIR = "Directory";
const char* MSG_FILETREE_TYPE_FILE = "File";
const char* MSG_FILETREE_TYPE_DISK = "Disk";
const char* MSG_FILETREE_TYPE_PARTITION = "Partition";

// Errors
const char* MSG_ALLOC_FAILED = "Memory allocation failed!";
const char* MSG_INVALID_INPUT = "Error: Invalid input!";
const char* MSG_OPEN_IMAGE_FAILED = "Error: Can not open image!";
const char* MSG_CREATE_IMAGE_FAILED = "Error: Creating image file failed!";
const char* MSG_ENUM_DISKS_FAILED = "Error: Enumerate disks failed!\nCode: %d";
const char* MSG_NO_IMAGE_OPENED = "No image file or disk opened!";
const char* MSG_EMPTY_DIR_NOT_ADDED = "Empty directory %s not added!";
//const char* MSG_OPEN_FILE_FAILED = "Error Open file failed!";
const char* MSG_OPEN_DIR_FAILED = "Error: Open directory failed!";
const char* MSG_LIST_DIR_FAILED = "Error: Listing directory failed!";
const char* MSG_DST_NOT_ON_PARTITON = "Error: destination path must be on a partition!";
const char* MSG_INVALID_FILENAME = "Error: Invalid filename entered!";
const char* MSG_INVALID_SELECTION = "Error: invalid selection!";
//const char* MSG_SINGLE_SELECTION_REQUIRED = "Error: a single item must be selected!";
const char* MSG_ADD_FAILED = "Adding files failed!";
const char* MSG_EXTRACT_FAILED = "Extracting files failed!";
const char* MSG_WRITE_IMAGE_FAILED = "Writing image failed!";
const char* MSG_MKDIR_FAILED = "Creating new folder failed!";
const char* MSG_COPY_FAILED = "Copying files failed!";
const char* MSG_MOVE_FAILED = "Moving files failed!";
const char* MSG_MOUNT_FAILED = "Mounting partitions failed";
const char* MSG_DEL_FAILED = "Deleting files failed";
const char* MSG_RESTART_ROOT_FAILED = "Error: Restarting as root failed!";

