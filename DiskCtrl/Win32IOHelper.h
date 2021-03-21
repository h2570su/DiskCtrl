#pragma once
#define _WIN32_DCOM
#define SPT_SENSE_LENGTH 32

#include <iostream>
#include <vector>
#include <utility>

#include <Windows.h>
#include <ioapiset.h>
#include <fileapi.h>
#include <comdef.h>
#include <WbemIdl.h>

#include <ntddscsi.h>
#include <ntddkbd.h>

#pragma comment(lib, "wbemuuid.lib")



typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
         SCSI_PASS_THROUGH_DIRECT sptd;
         ULONG                  Filler;           // realign buffer to double word boundary
         UCHAR                  ucSenseBuf[SPT_SENSE_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;


class Win32IOHelper
{
	//Model, Physical Address
	std::vector<std::pair<std::wstring, std::wstring>> diskList;



public:
	static Win32IOHelper* instance;

	int WMI_getPhyDisks();
	std::vector<std::pair<std::wstring, std::wstring>> getDiskList() { return this->diskList; };

	bool Win32_ReadOneLBA(std::string phyAddr, std::vector<uint8_t>& LBAdata, int LBA);

	~Win32IOHelper();
};

