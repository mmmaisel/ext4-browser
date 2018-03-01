/**********************************************************************\
 * ext4-browser
 * file_dev.h
 *
 * lwext4 cross platform file device
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

#include <ext4_config.h>
#include <ext4_blockdev.h>

#include <stdint.h>
#include <stdbool.h>

#include "physicalDevice.h"

enum : uint32_t { BLOCKDEV_DEFAULT_BSIZE = 512 };

struct blockdev
{
	wxString* name;
	wxFile* file;
	uint8_t* ph_bbuf;
	struct ext4_blockdev_iface iface;
	struct ext4_blockdev bd;
	PhysicalDevice* p_physical_dev;
};

struct blockdev* blockdev_create
	(wxString& n, uint32_t blockSize, PhysicalDevice* pDev);
void blockdev_destroy(struct blockdev* dev);
uint8_t blockdev_is_valid(struct blockdev* dev);
