/**********************************************************************\
 * ext4-browser
 * file_dev.c.h
 *
 * lwext4 cross platform file device
 **********************************************************************
 * Copyright (C) 2017 - Max Maisel
 * Copyright 2016 ROSA and Konstantin Vlasov <konstantin.vlasov@rosalab.ru>
 *
 * The following code is a derivative work of the code from ROSA
 * ImageWriter, which is licensed GPLv3. This code therefore is also
 * licensed under the terms of the GNU Public License, verison 3.
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

/*
 * Copyright (c) 2013 Grzegorz Kostka (kostka.grzegorz@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "stdafx.h"
#include "blockdev.h"

#include <ext4_config.h>
#include <ext4_blockdev.h>
#include <ext4_errno.h>


/**********************BLOCKDEV INTERFACE**************************************/
static int blockdev_open(struct ext4_blockdev *bdev);
static int blockdev_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
			 uint32_t blk_cnt);
static int blockdev_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt);
static int blockdev_close(struct ext4_blockdev *bdev);

/******************************************************************************/

static int blockdev_open(struct ext4_blockdev *bdev)
{
	struct blockdev* p_dev = (blockdev*)bdev->bdif->p_user;

	if(p_dev->file->IsOpened())
		return EOK;

	if(p_dev->p_physical_dev)
		p_dev->p_physical_dev->Open(*p_dev->file);
	else
		p_dev->file->Open(*p_dev->name, wxFile::read_write);

	if(!p_dev->file->IsOpened())
		return EIO;

	p_dev->bd.part_offset = 0;
	if(p_dev->p_physical_dev)
		p_dev->bd.part_size = p_dev->p_physical_dev->GetSize();
	else
		p_dev->bd.part_size = p_dev->file->Length();

	p_dev->bd.bdif->ph_bcnt = p_dev->bd.part_size / p_dev->bd.bdif->ph_bsize;
	return EOK;
}

static int blockdev_bread(struct ext4_blockdev *bdev,
	void *buf, uint64_t blk_id, uint32_t blk_cnt)
{
	struct blockdev* p_dev = (blockdev*)bdev->bdif->p_user;

	if(p_dev->file->Seek(blk_id * bdev->bdif->ph_bsize) == wxInvalidOffset)
		return EIO;
	if(!blk_cnt)
		return EOK;
	if(p_dev->file->Read(buf, bdev->bdif->ph_bsize * blk_cnt) == wxInvalidOffset)
		return EIO;

	return EOK;
}

static int blockdev_bwrite(struct ext4_blockdev *bdev, const void *buf,
	uint64_t blk_id, uint32_t blk_cnt)
{
	struct blockdev* p_dev = (blockdev*)bdev->bdif->p_user;

	if(p_dev->file->Seek(blk_id * bdev->bdif->ph_bsize) == wxInvalidOffset)
		return EIO;
	if(!blk_cnt)
		return EOK;
	if(p_dev->file->Write(buf, bdev->bdif->ph_bsize * blk_cnt)
		!= bdev->bdif->ph_bsize * blk_cnt)
		return EIO;

	return EOK;
}

static int blockdev_close(struct ext4_blockdev *bdev)
{
	struct blockdev* p_dev = (blockdev*)bdev->bdif->p_user;

	if(!p_dev->file->IsOpened())
		return EIO;

#if defined(__APPLE__) || defined(__linux)
	fsync(p_dev->file->fd());
#endif

	if(!p_dev->file->Close())
		return EIO;
	return EOK;
}
/******************************************************************************/

struct blockdev* blockdev_create
	(wxString& n, uint32_t blockSize, PhysicalDevice* pDev)
{
	struct blockdev* p_file_dev = (blockdev*)malloc(sizeof(struct blockdev));
	memset(p_file_dev, 0, sizeof(struct blockdev));

	p_file_dev->p_physical_dev = pDev;
#if defined(WIN32) || defined(WIN64)
	if(p_file_dev->p_physical_dev)
	{
		// Using VirtualAlloc so that the buffer was properly aligned
		// (required for direct access to devices and for unbuffered
		// reading/writing)
		p_file_dev->ph_bbuf = (uint8_t*)VirtualAlloc(NULL, blockSize,
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if(!p_file_dev->ph_bbuf)
			return p_file_dev;
	}
#else
	if(0) ;
#endif
	else
		p_file_dev->ph_bbuf = (uint8_t*)malloc(blockSize);

	p_file_dev->name = new wxString(n);
	p_file_dev->iface.open = blockdev_open;
	p_file_dev->iface.bread = blockdev_bread;
	p_file_dev->iface.bwrite = blockdev_bwrite;
	p_file_dev->iface.close = blockdev_close;
	p_file_dev->iface.lock = 0;
	p_file_dev->iface.unlock = 0;
	p_file_dev->iface.ph_bsize = blockSize;
	p_file_dev->iface.ph_bcnt = 0;
	p_file_dev->iface.ph_bbuf = p_file_dev->ph_bbuf;

	p_file_dev->bd.bdif = &p_file_dev->iface;
	p_file_dev->bd.bdif->p_user = p_file_dev;
	p_file_dev->bd.part_offset = 0;
	p_file_dev->bd.part_size = 0;

	p_file_dev->file = new wxFile();

	return p_file_dev;
}

void blockdev_destroy(struct blockdev* dev)
{
	ext4_block_fini(&dev->bd);
	// PhysicalDevice is deleted separately
	//if(dev->p_physical_dev)
	//	delete dev->p_physical_dev;
	if(dev->name)
		delete dev->name;
	if(dev->file)
		delete dev->file;
	if(dev->ph_bbuf)
	{
#if defined(WIN32) || defined(WIN64)
		if(dev->p_physical_dev)
			VirtualFree(dev->ph_bbuf, dev->iface.ph_bsize,
				MEM_DECOMMIT | MEM_RELEASE);
#else
		if(0) ;
#endif
		else
			free(dev->ph_bbuf);
	}
	free(dev);
}

uint8_t blockdev_is_valid(struct blockdev* dev)
{
	if(!dev)
		return 0;
	if(!dev->ph_bbuf)
		return 0;
	if(!dev->name)
		return 0;

	return 1;
}
