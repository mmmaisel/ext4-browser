/**********************************************************************\
 * ext4-browser
 * physical_dev.h
 *
 * Physical device interface
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
#pragma once
#include "backendTask.h"

class PhysicalDevice
{
	public:
		PhysicalDevice()
			: mFriendlyName("Unknown Device")
			, mVolumes()
			, mSize(0)
			, mSectorSize(512)
			, mSystemName("") {}
		~PhysicalDevice() {}

		inline const wxString& GetFriendlyName() const { return mFriendlyName; }
		inline const wxString& GetSystemName() const { return mSystemName; }
		inline const uint32_t GetSectorSize() const {return mSectorSize;}
		inline const uint64_t GetSize() const {return mSize;}

		wxString FormatDisplayName() const;
		int Open(wxFile& file);
		int WriteImage(wxFile& imgFile, IBackendTask* pInterface);

		static int EnumFlashDevices(std::list<PhysicalDevice>& devs);

		enum eError
		{
			NOERROR,
			ERR_CANCELED,
			ERR_ALLOC,
			ERR_CREATE_FILE,
			ERR_IOCTL,
			ERR_OPEN_FD,
			ERR_UMOUNT,
			ERR_SYS_STR_ALLOC,
			ERR_CO_CREATE_INSTANCE,
			ERR_CONNECT_SERVER,
			ERR_QUERY_USB,
			ERR_QUERY_PARTITIONS,
			ERR_WBEM,
			ERR_IO_MATCHING,
			ERR_READ_IMG,
			ERR_WRITE_DISK
		};

	private:
		wxString mFriendlyName;
		std::list<wxString> mVolumes;
		uint64_t mSize;
		uint32_t mSectorSize;
		wxString mSystemName;

		static const uint32_t TRANSFER_BLOCK_SIZE = 1024*1024;

#if defined(_WIN32) || defined(_WIN64)
		// Releases the COM object and nullifies the pointer
		template<class T> static inline void SAFE_RELEASE(T& obj)
		{
			if(obj != NULL)
			{
				obj->Release();
				obj = NULL;
			}
		}

		// Releases the BSTR string and nullifies the pointer
		static inline void FREE_BSTR(BSTR& str)
		{
			SysFreeString(str);
			str = NULL;
		}

#elif defined(__APPLE__)
		static bool readBooleanRegKey(io_service_t device, CFStringRef key);
		static unsigned long long readIntegerRegKey(io_service_t device, CFStringRef key);
		static CFStringRef readStringRegKey(io_service_t device, CFStringRef key);
		static wxString CFStrToWxStr(CFStringRef cfstr, CFStringEncoding encodingMethod);
#endif
};
