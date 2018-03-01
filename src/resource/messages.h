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
#pragma once

// Common
extern const char* MSG_OK;
extern const char* MSG_CANCEL;
extern const char* MSG_ERROR;
extern const char* MSG_ABOUT;
extern const char* MSG_CLOSE;

// Menu
extern const char* MSG_MENU_PAGE_HOME;
extern const char* MSG_MENU_CREATE_IMAGE;
extern const char* MSG_MENU_OPEN_IMAGE;
extern const char* MSG_MENU_CLOSE;
extern const char* MSG_MENU_OPEN_DISK;
extern const char* MSG_MENU_WRITE_IMAGE;
extern const char* MSG_MENU_PAGE_FILE;
extern const char* MSG_MENU_ADD_FILES;
extern const char* MSG_MENU_ADD_FOLDERS;
extern const char* MSG_MENU_NEW_FOLDER;
extern const char* MSG_MENU_EXTRACT;
extern const char* MSG_MENU_COPY;
extern const char* MSG_MENU_CUT;
extern const char* MSG_MENU_PASTE;
extern const char* MSG_MENU_RENAME;
extern const char* MSG_MENU_DELETE;

// Message boxes
extern const char* MSG_OP_SUCCESSFUL;
extern const char* MSG_OP_CANCELED;
extern const char* MSG_OP_FAILED;
extern const char* MSG_CONFIRM_CLOSE;
extern const char* MSG_CONFIRM_CLOSE_TITLE;
extern const char* MSG_RESTART_ROOT;
extern const char* MSG_RESTART_ROOT_TITLE;
extern const char* MSG_SELECT_DISK;
extern const char* MSG_SELECT_DISK_TITLE;
extern const char* MSG_IN_PROGRESS;
extern const char* MSG_WRITE_IMG_PROGRESS;
extern const char* MSG_IN_PROGRESS_TITLE;
extern const char* MSG_MKDIR_NAME;
extern const char* MSG_RENAME;
extern const char* MSG_CONFIRM_DELETE;
extern const char* MSG_CONFIRM_DELETE_TITLE;
extern const char* MSG_CONFIRM_WRITE_IMG;
extern const char* MSG_CONFIRM_WRITE_IMG_TITLE;
extern const char* MSG_ARCHIVE_ERROR;

// New image dialog
extern const char* MSG_NEW_IMG_TITLE;
extern const char* MSG_NEW_IMG_PATH;
extern const char* MSG_NEW_IMG_TYPE;
extern const char* MSG_NEW_IMG_SIZE_MIB;
extern const char* MSG_NEW_IMG_SELECT_FILE;

// Disk dialog
extern const char* MSG_IMGDLG_WRITE_IMG;
extern const char* MSG_IMGDLG_IMGFILE;
extern const char* MSG_IMGDLG_DISK;

// File exists dialog
extern const char* MSG_SKIP;
extern const char* MSG_SKIP_ALL;
extern const char* MSG_OVERWRITE;
extern const char* MSG_OVERWRITE_ALL;
extern const char* MSG_MERGE_DIR;
extern const char* MSG_MERGE_DIR_ALL;
extern const char* MSG_FILE_EXISTS;
extern const char* MSG_DIR_EXISTS;
extern const char* MSG_EXISTS_DLG_TITLE;

// File selectors
extern const char* SELECTOR_ALL_FILES;
extern const char* SELECTOR_IMG_FILES;

// File tree
extern const char* MSG_FILETREE_FILE;
extern const char* MSG_FILETREE_TYPE;
extern const char* MSG_FILETREE_SIZE;
extern const char* MSG_FILETREE_TYPE_DIR;
extern const char* MSG_FILETREE_TYPE_FILE;
extern const char* MSG_FILETREE_TYPE_DISK;
extern const char* MSG_FILETREE_TYPE_PARTITION;

// Errors
extern const char* MSG_ALLOC_FAILED;
extern const char* MSG_INVALID_INPUT;
extern const char* MSG_OPEN_IMAGE_FAILED;
extern const char* MSG_CREATE_IMAGE_FAILED;
extern const char* MSG_ENUM_DISKS_FAILED;
extern const char* MSG_NO_IMAGE_OPENED;
extern const char* MSG_EMPTY_DIR_NOT_ADDED;
//extern const char* MSG_OPEN_FILE_FAILED;
extern const char* MSG_OPEN_DIR_FAILED;
extern const char* MSG_LIST_DIR_FAILED;
extern const char* MSG_DST_NOT_ON_PARTITON;
extern const char* MSG_INVALID_FILENAME;
extern const char* MSG_INVALID_SELECTION;
//extern const char* MSG_SINGLE_SELECTION_REQUIRED;
extern const char* MSG_ADD_FAILED;
extern const char* MSG_EXTRACT_FAILED;
extern const char* MSG_WRITE_IMAGE_FAILED;
extern const char* MSG_MKDIR_FAILED;
extern const char* MSG_COPY_FAILED;
extern const char* MSG_MOVE_FAILED;
extern const char* MSG_MOUNT_FAILED;
extern const char* MSG_DEL_FAILED;
extern const char* MSG_RESTART_ROOT_FAILED;
