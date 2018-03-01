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
#include "stdafx.h"
#include "physicalDevice.h"

wxString PhysicalDevice::FormatDisplayName() const
{
	if(mVolumes.size() == 0)
		return wxString("<unmounted>");
	else
	{
		wxString retval;
		int first = 1;
		for(const auto& vol : mVolumes)
		{
			if(!first)
				retval.append(", ");
			else
				first = 0;
			retval.append(vol);
		}
		retval.append(wxString::Format(" - %s (%d MiB)",
			mFriendlyName, uint32_t(mSize / 1024 / 1024)));
		return retval;
	}
}

int PhysicalDevice::Open(wxFile& file)
{
#if defined(__APPLE__)
	struct statfs* mntEntries = NULL;
	int mntEntriesNum = 0;
#endif
    
	// Unmount volumes that belong to the selected target device
#if defined(WIN32) || defined(WIN64)
	DWORD ret;

	for(const auto& it : mVolumes)
	{
		HANDLE volume = CreateFile(
			wxString::Format("\\\\.\\%s", it.ToStdWstring().c_str()),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);
		if(volume == INVALID_HANDLE_VALUE)
		{
			return ERR_CREATE_FILE;
		}
		// Trying to lock the volume but ignore if we failed
		// (such call seems to be required for dismounting the volume on WinXP)
		DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &ret, NULL);
		if(!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &ret, NULL))
		{
			CloseHandle(volume);
			return ERR_UMOUNT;
		}
		CloseHandle(volume);
		volume = INVALID_HANDLE_VALUE;
	}

	// In Windows QFile with write mode uses disposition OPEN_ALWAYS,
	// but WinAPI requires OPEN_EXISTING for physical devices.
	// Therefore we have to use native API.
	HANDLE fileHandle = CreateFile(
		mSystemName.ToStdWstring().c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
		NULL
	);
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		return ERR_CREATE_FILE;
	}
	// Lock the opened device
	if(!DeviceIoControl
		(fileHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL,0, &ret, NULL))
	{
		return ERR_IOCTL;
	}

	// Construct wxFile around the device handle;
	// close() will now close the handle automatically
	int fd = _open_osfhandle(reinterpret_cast<intptr_t>(fileHandle), 0);
	if(fd == -1)
	{
		CloseHandle(fileHandle);
		return ERR_OPEN_FD;
	}

	file.Attach(fd);

#elif defined(__APPLE__)

	mntEntriesNum = getmntinfo(&mntEntries, MNT_WAIT);
	for (int i = 0; i < mntEntriesNum; ++i)
	{
	for (const auto& it : mVolumes)
		{
			wxString mntname = wxString(mntEntries[i].f_mntfromname);
			// Check that the mount point is either our target device itself or a partition on it
			if ((mntEntries[i].f_mntfromname == it) || mntname.StartsWith(it + 's'))
			{
				// Mount point is the selected device or one of its partitions - try to unmount it
				if (unmount(mntEntries[i].f_mntonname, MNT_FORCE) != 0)
					return ERR_UMOUNT;
			}
		}
	}
	file.Open(mSystemName, wxFile::read_write);

#elif defined(__linux)
#warning Physical devices are not supported on Linux yet.
	file.Open(mSystemName, wxFile::read_write);
#endif

	if(file.IsOpened())
		return NOERROR;
	return ERR_OPEN_FD;
}

int PhysicalDevice::WriteImage(wxFile& imgFile, IBackendTask* pInterface)
{
	int retval = NOERROR;
	void* buffer = NULL;
	wxFile deviceFile;
	ssize_t readBytes;
	size_t writtenBytes;

#if defined(_WIN32) || defined(_WIN64)
	// Using VirtualAlloc so that the buffer was properly aligned (required for
	// direct access to devices and for unbuffered reading/writing)
	buffer = VirtualAlloc(NULL, TRANSFER_BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if(buffer == NULL)
	{
		retval =  ERR_ALLOC;
		goto exit_error;
	}
#elif defined(__APPLE__) || defined(__linux)
	buffer = malloc(TRANSFER_BLOCK_SIZE);
	if(buffer == NULL)
	{
		retval = ERR_ALLOC;
		goto exit_error;
	}
#endif

	pInterface->ResetCanceled();
	imgFile.Seek(0);

	// Open the target USB device for writing
	if((retval = Open(deviceFile)) != NOERROR)
	{
		goto exit_error;
	}
	deviceFile.Seek(0); // Required on windows, without all writes will fail

	retval = NOERROR;
	do
	{
		readBytes = imgFile.Read(buffer, TRANSFER_BLOCK_SIZE);
		if(readBytes == wxInvalidOffset)
		{
			retval = ERR_READ_IMG;
			goto exit_error;
		}
		// If the image is read completely and image size is a multiple of
		// TRANSFER_BLOCK_SIZE, readBytes can get 0.
		// Write operation with size 0 cause problems on OSX
		// so break out of loop here if there is no remainder.
		if(readBytes == 0)
			break;

		int remainder = readBytes % mSectorSize;
		if(remainder)
		{
			memset((uint8_t*)buffer+readBytes, 0, readBytes % mSectorSize);
			readBytes += remainder;
		}

		writtenBytes = deviceFile.Write(buffer, readBytes);
		if(static_cast<size_t>(readBytes) != writtenBytes)
		{
			retval = ERR_WRITE_DISK;
			goto exit_error;
		}

#if defined(__APPLE__) || defined(__linux)
		// In Linux/MacOS the USB device is opened with buffering.
		// Using forced sync to validate progress bar.
		// For unknown reason, deviceFile.flush() does not work as intended here.
		fsync(deviceFile.fd());
#endif

		if(pInterface)
		{
			if(pInterface->IsCanceled())
			{
				retval = ERR_CANCELED;
				goto exit_error;
			}
			pInterface->OnProgress(readBytes / mSectorSize);
		}
	}
	while(static_cast<size_t>(readBytes) >= TRANSFER_BLOCK_SIZE);

exit_error:
	deviceFile.Close();

	if(buffer != NULL)
#if defined(_WIN32) || defined(_WIN64)
		VirtualFree(buffer, TRANSFER_BLOCK_SIZE, MEM_DECOMMIT | MEM_RELEASE);
#elif defined(__APPLE__) || defined(__linux)
		free(buffer);
#endif

	return retval;
}

int PhysicalDevice::EnumFlashDevices(std::list<PhysicalDevice>& devs)
{
	int retval = NOERROR;
#if defined(_WIN32) || defined(_WIN64)
	// Using WMI for enumerating the USB devices

	// Namespace of the WMI classes
	BSTR strNamespace       = NULL;
	// "WQL" - the query language we're gonna use (the only possible, actually)
	BSTR strQL              = NULL;
	// Query string for requesting physical devices
	BSTR strQueryDisks      = NULL;
	// Query string for requesting partitions for each of the the physical devices
	BSTR strQueryPartitions = NULL;
	// Query string for requesting logical disks for each of the partitions
	BSTR strQueryLetters    = NULL;

	// Various COM objects for executing the queries, enumerating lists and retrieving properties
	IWbemLocator*         pIWbemLocator         = NULL;
	IWbemServices*        pWbemServices         = NULL;
	IEnumWbemClassObject* pEnumDisksObject      = NULL;
	IEnumWbemClassObject* pEnumPartitionsObject = NULL;
	IEnumWbemClassObject* pEnumLettersObject    = NULL;
	IWbemClassObject*     pDiskObject           = NULL;
	IWbemClassObject*     pPartitionObject      = NULL;
	IWbemClassObject*     pLetterObject         = NULL;

	// Start with allocating the fixed strings
	strNamespace = SysAllocString(L"root\\cimv2");
	if(!strNamespace)
	{
		retval = ERR_SYS_STR_ALLOC;
		goto exit_error;
	}
	strQL = SysAllocString(L"WQL");
	if(!strQL)
	{
		retval = ERR_SYS_STR_ALLOC;
		goto exit_error;
	}
	strQueryDisks = SysAllocString(L"SELECT * FROM Win32_DiskDrive WHERE InterfaceType = \"USB\"");
	if(!strQueryDisks)
	{
		retval = ERR_SYS_STR_ALLOC;
		goto exit_error;
	}

	// Create the IWbemLocator and execute the first query (list of physical disks attached via USB)
	if(CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL,
		CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown,
		reinterpret_cast<void**>(&pIWbemLocator)) != S_OK)
	{
		retval = ERR_CO_CREATE_INSTANCE;
		goto exit_error;
	}
	if(pIWbemLocator->ConnectServer(strNamespace, NULL, NULL,
		NULL, 0, NULL, NULL, &pWbemServices) != S_OK)
	{
		retval = ERR_CONNECT_SERVER;
		goto exit_error;
	}
	if(pWbemServices->ExecQuery(strQL, strQueryDisks,
		WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDisksObject) != S_OK)
	{
		retval = ERR_QUERY_USB;
		goto exit_error;
	}

	// Enumerate the received list of devices
	for (;;)
	{
		// Get the next available device or exit the loop
		ULONG uReturned;
		HRESULT wbemRes;

		wbemRes = pEnumDisksObject->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturned);
		// pEnumDisksObject->Next can return WBEM_E_ACCESS_DENIED and other errors
		if(wbemRes != WBEM_S_NO_ERROR
			&& wbemRes != WBEM_S_NO_MORE_DATA
			&& wbemRes != ERROR_INVALID_FUNCTION)
		{
			retval = ERR_WBEM;
			goto exit_error;
		}

		if (uReturned == 0)
			break;

		VARIANT val;

		// Fetch the required properties and store them in the UsbDevice object
		PhysicalDevice deviceData;

		// User-friendly name of the device
		if (pDiskObject->Get(L"Model", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
		{
			if (val.vt == VT_BSTR)
			{
				deviceData.mFriendlyName = wxString(val.bstrVal);
			}
			VariantClear(&val);
		}

		// System name of the device
		if (pDiskObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
		{
			if (val.vt == VT_BSTR)
			{
				deviceData.mSystemName = wxString(val.bstrVal);
			}
			VariantClear(&val);
		}

		// Size of the device
		if (pDiskObject->Get(L"Size", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
		{
			if (val.vt == VT_BSTR)
			{
				wxString(val.bstrVal).ToULongLong(&deviceData.mSize, 10);
			}
			VariantClear(&val);
		}

		// Sector size of the device
		if (pDiskObject->Get(L"BytesPerSector", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
		{
			if (val.vt == VT_I4)
			{
				deviceData.mSectorSize = val.intVal;
			}
			VariantClear(&val);
		}

		// The device object is no longer needed, release it
		SAFE_RELEASE(pDiskObject);

		// Construct the request for listing the partitions on the current disk
		wxString wxStrQueryPartitions;
			wxStrQueryPartitions += "ASSOCIATORS OF {Win32_DiskDrive.DeviceID='"
			+ deviceData.mSystemName
			+ "'} WHERE AssocClass = Win32_DiskDriveToDiskPartition";
		strQueryPartitions = SysAllocString(reinterpret_cast<const wchar_t*>
			(wxStrQueryPartitions.ToStdWstring().c_str()));
		if(!strQueryPartitions)
		{
			retval = ERR_SYS_STR_ALLOC;
			goto exit_error;
		}

		// Execute the query
		if(pWbemServices->ExecQuery(strQL, strQueryPartitions,
			WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumPartitionsObject) != S_OK)
		{
			retval = ERR_QUERY_PARTITIONS;
			goto exit_error;
		}

		// Enumerate the received list of partitions
		for (;;)
		{
			// Get the next available partition or exit the loop
			wbemRes = pEnumPartitionsObject->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturned);
			// pEnumPartitionsObject->Next can return WBEM_E_ACCESS_DENIED and other errors
			if(wbemRes != WBEM_S_NO_ERROR
				&& wbemRes != WBEM_S_NO_MORE_DATA
				&& wbemRes != ERROR_INVALID_FUNCTION)
			{
				retval = ERR_WBEM;
				goto exit_error;
			}

			if (uReturned == 0)
				break;

			// Fetch the DeviceID property and store it for using in the next request
			wxString wxStrQueryLetters = "";
			if (pPartitionObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
			{
				if (val.vt == VT_BSTR)
				{
					wxStrQueryLetters = wxString(val.bstrVal);
				}
				VariantClear(&val);
			}

			// The partition object is no longer needed, release it
			SAFE_RELEASE(pPartitionObject);

			// If DeviceID was fetched proceed to the logical disks
			if (wxStrQueryLetters != "")
			{
				// Construct the request for listing the logical disks related to the current partition
				wxStrQueryLetters =
					"ASSOCIATORS OF {Win32_DiskPartition.DeviceID='"
					+ wxStrQueryLetters
					+ "'} WHERE AssocClass = Win32_LogicalDiskToPartition";
				strQueryLetters = SysAllocString(reinterpret_cast<const wchar_t*>
					(wxStrQueryLetters.ToStdWstring().c_str()));
				if(!strQueryLetters)
				{
					retval = ERR_SYS_STR_ALLOC;
					goto exit_error;
				}

				// Execute the query
				if(pWbemServices->ExecQuery(strQL, strQueryLetters,
					WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumLettersObject) != S_OK)
				{
					retval = ERR_SYS_STR_ALLOC;
					goto exit_error;
				}

				// Enumerate the received list of logical disks
				for (;;)
				{
					// Get the next available logical disk or exit the loop
					wbemRes = pEnumLettersObject->Next(WBEM_INFINITE, 1, &pLetterObject, &uReturned);
					// pEnumLettersObject->Next can return WBEM_E_ACCESS_DENIED and other errors
					if(wbemRes != WBEM_S_NO_ERROR
						&& wbemRes != WBEM_S_NO_MORE_DATA
						&& wbemRes != ERROR_INVALID_FUNCTION)
					{
						retval = ERR_WBEM;
						goto exit_error;
					}

					if (uReturned == 0)
						break;

					// Fetch the disk letter and add it to the list of volumes in the UsbDevice object
					if (pLetterObject->Get(L"Caption", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
					{
						if (val.vt == VT_BSTR)
						{
							deviceData.mVolumes.push_back(wxString(val.bstrVal));
						}
						VariantClear(&val);
					}

					// The logical disk object is no longer needed, release it
					SAFE_RELEASE(pLetterObject);
				}

				// Release the logical disks enumerator object and the corresponding query string
				SAFE_RELEASE(pEnumLettersObject);
				FREE_BSTR(strQueryLetters);
			}
		}

		// Release the partitions enumerator object and the corresponding query string
		SAFE_RELEASE(pEnumPartitionsObject);
		FREE_BSTR(strQueryPartitions);

		// The device information is now complete, append the entry
		devs.push_back(std::move(deviceData));
	}

exit_error:

	SAFE_RELEASE(pLetterObject);
	SAFE_RELEASE(pPartitionObject);
	SAFE_RELEASE(pDiskObject);
	SAFE_RELEASE(pEnumDisksObject);
	SAFE_RELEASE(pEnumPartitionsObject);
	SAFE_RELEASE(pEnumLettersObject);
	SAFE_RELEASE(pWbemServices);
	SAFE_RELEASE(pIWbemLocator);

	FREE_BSTR(strNamespace);
	FREE_BSTR(strQL);
	FREE_BSTR(strQueryDisks);
	FREE_BSTR(strQueryPartitions);
	FREE_BSTR(strQueryLetters);

#elif defined(__APPLE__)

	CFMutableDictionaryRef matchingDict;
	io_iterator_t iter;
	kern_return_t kr;
	io_service_t device;

	// Set up a matching dictionary for the class
	matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
	if (matchingDict == NULL)
	{
		return ERR_IO_MATCHING;
	}

	// Obtain iterator
	kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
	if (kr != KERN_SUCCESS)
	{
		return ERR_IO_MATCHING;
	}

	CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

	// Enumerate the devices
	while ((device = IOIteratorNext(iter)))
	{
		// Skip all non-removable devices
		if (!readBooleanRegKey(device, CFSTR(kIOMediaRemovableKey)))
		{
			IOObjectRelease(device);
			continue;
		}

		// Skip devices without BSD names (that is, not real disks)
		CFStringRef tempStr = readStringRegKey(device, CFSTR(kIOBSDNameKey));
		if (tempStr == nil)
		{
			IOObjectRelease(device);
			continue;
		}

		// Fetch the required properties and store them in the UsbDevice object
		PhysicalDevice deviceData;

		// Physical device name
		// Using "rdiskN" instead of BSD name "diskN" to work around an OS X bug when writing
		// to "diskN" is extremely slow
		wxString bsdName = CFStrToWxStr(tempStr, encodingMethod);
		CFRelease(tempStr);
		deviceData.mSystemName = "/dev/r" + bsdName;
		// Volume names are very long, so display the device name instead
		deviceData.mVolumes.push_back("/dev/" + bsdName);

		// User-friendly device name: vendor+product
		tempStr = readStringRegKey(device, CFSTR(kUSBVendorString));
		if (tempStr != nil)
		{
			deviceData.mFriendlyName =
				CFStrToWxStr(tempStr, encodingMethod).Trim();
			CFRelease(tempStr);
		}
		tempStr = readStringRegKey(device, CFSTR(kUSBProductString));
		if (tempStr != nil)
		{
			deviceData.mFriendlyName += " ";
			deviceData.mFriendlyName += CFStrToWxStr(tempStr, encodingMethod);
			deviceData.mFriendlyName.Trim();
			CFRelease(tempStr);
		}

		// Size of the flash disk
		deviceData.mSize = readIntegerRegKey(device, CFSTR(kIOMediaSizeKey));
		deviceData.mSectorSize = readIntegerRegKey(device, CFSTR(kIOMediaPreferredBlockSizeKey));

		// The device information is now complete, append the entry
		devs.push_back(std::move(deviceData));

		// Free the resources
		IOObjectRelease(device);
	}

	IOObjectRelease(iter);
	return NOERROR;
#endif
	return retval;
}

#if defined(__APPLE__)
bool PhysicalDevice::readBooleanRegKey(io_service_t device, CFStringRef key)
{
	CFTypeRef value = IORegistryEntrySearchCFProperty(
		device,
		kIOServicePlane,
		key,
		kCFAllocatorDefault,
		kIORegistryIterateRecursively
	);
	bool res = false;
	if (value != nil)
	{
		if (CFGetTypeID(value) == CFBooleanGetTypeID())
		    res = (CFBooleanGetValue((CFBooleanRef)value) ? true : false);
		CFRelease(value);
	}
	return res;
}

unsigned long long PhysicalDevice::readIntegerRegKey(io_service_t device, CFStringRef key)
{
	CFTypeRef value = IORegistryEntrySearchCFProperty(
		device,
		kIOServicePlane,
		key,
		kCFAllocatorDefault,
		kIORegistryIterateRecursively
	);
	unsigned long long res = 0;
	if (value != nil)
	{
		CFNumberGetValue((CFNumberRef)value, kCFNumberLongLongType, &res);
		CFRelease(value);
	}
	return res;
}


CFStringRef PhysicalDevice::readStringRegKey(io_service_t device, CFStringRef key)
{
	CFTypeRef value = IORegistryEntrySearchCFProperty(
		device,
		kIOServicePlane,
		key,
		kCFAllocatorDefault,
		kIORegistryIterateRecursively
	);
	CFStringRef res = nil;
	if (value != nil)
	{
		if (CFGetTypeID(value) == CFStringGetTypeID())
			res = (CFStringRef)value;
		else
			CFRelease(value);
	}
	return res;
}

wxString PhysicalDevice::CFStrToWxStr(CFStringRef cfstr, CFStringEncoding encodingMethod)
{
	wxString wxstr;
	const char* pstr = CFStringGetCStringPtr(cfstr, encodingMethod);
	if(pstr)
		wxstr = pstr;
	else
	{
		CFIndex length = CFStringGetLength(cfstr);
		length = CFStringGetMaximumSizeForEncoding(length, encodingMethod) + 1;
		char* tmpbuf = new char[length];
		CFStringGetCString(cfstr, tmpbuf, length, encodingMethod);
		wxstr = tmpbuf;
		delete[] tmpbuf;
	}
	return wxstr;
}

#endif

