#include "Win32IOHelper.h"
Win32IOHelper* Win32IOHelper::instance = new Win32IOHelper();

int Win32IOHelper::WMI_getPhyDisks()
{
	using namespace std;
	HRESULT hres;
	/*
		COM had been initialized, don't know why
	*/



	//// Step 1: --------------------------------------------------
	//// Initialize COM. ------------------------------------------
	//
	//hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED );
	//if (FAILED(hres))
	//{
	//	cout << "Failed to initialize COM library. Error code = 0x"
	//		<< hex << hres << endl;
	//	return 1;                  // Program has failed.
	//}

	//// Step 2: --------------------------------------------------
	//// Set general COM security levels --------------------------

	//hres = CoInitializeSecurity(
	//	NULL,
	//	-1,                          // COM authentication
	//	NULL,                        // Authentication services
	//	NULL,                        // Reserved
	//	RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
	//	RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
	//	NULL,                        // Authentication info
	//	EOAC_NONE,                   // Additional capabilities 
	//	NULL                         // Reserved
	//);


	//if (FAILED(hres))
	//{
	//	cout << "Failed to initialize security. Error code = 0x"
	//		<< hex << hres << endl;
	//	CoUninitialize();
	//	return 1;                    // Program has failed.
	//}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return 1;                // Program has failed.
	}

	cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_DiskDrive"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	this->diskList.clear();
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtModel;
		VARIANT vtDeviceID;

		wstring strModel;
		wstring strPhyAddr;

		// Get the value of the Name property
		hr = pclsObj->Get(L"Model", 0, &vtModel, 0, 0);


		wcout << "Name: " << vtModel.bstrVal;

		strModel = wstring(vtModel.bstrVal);
		VariantClear(&vtModel);

		hr = pclsObj->Get(L"DeviceID", 0, &vtDeviceID, 0, 0);


		wcout << "@PHY ADDR: " << vtDeviceID.bstrVal << endl;


		strPhyAddr = wstring(vtDeviceID.bstrVal);
		VariantClear(&vtDeviceID);

		this->diskList.push_back(pair<wstring, wstring>(strModel, strPhyAddr));

		pclsObj->Release();
	}

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	//CoUninitialize();

	return 0;
}

bool Win32IOHelper::Win32_ReadOneLBA(std::string phyAddr, std::vector<uint8_t>& LBAdata, int LBA)
{
	using namespace std;
	phyAddr.insert(0, 1, '\\');
	phyAddr.insert(3, 1, '\\');
	HANDLE disk = CreateFile(phyAddr.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (disk == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		int iRet = 0;

		vector<BYTE> outputBuffer;
		outputBuffer.resize(64*1024+10);
		DWORD byteReceived = 0;
		SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
		ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
		sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sptdwb.sptd.PathId = 0;
		sptdwb.sptd.TargetId = 1;
		sptdwb.sptd.Lun = 0;
		sptdwb.sptd.CdbLength = 10;
		sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
		sptdwb.sptd.SenseInfoLength = 24;
		sptdwb.sptd.DataTransferLength = 512;
		sptdwb.sptd.TimeOutValue = 2;
		sptdwb.sptd.DataBuffer = outputBuffer.data();
		sptdwb.sptd.SenseInfoOffset =
			offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
		sptdwb.sptd.Cdb[0] = 0x28;
		uint32_t readLBA = LBA; //our 23
		uint16_t readLength = 1;
		sptdwb.sptd.Cdb[2] = (readLBA >> 24) & 0xFF;
		sptdwb.sptd.Cdb[3] = (readLBA >> 16) & 0xFF;
		sptdwb.sptd.Cdb[4] = (readLBA >> 8) & 0xFF;
		sptdwb.sptd.Cdb[5] = (readLBA) & 0xFF;

		sptdwb.sptd.Cdb[7] = (readLength >> 8) & 0xFF;
		sptdwb.sptd.Cdb[8] = (readLength) & 0xFF;


		
		iRet = DeviceIoControl(disk,
			IOCTL_SCSI_PASS_THROUGH_DIRECT,
			&sptdwb,
			sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
			&sptdwb,
			sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
			&byteReceived,
			nullptr
		);
		if (iRet)
		{
			LBAdata.clear();
			LBAdata.insert(LBAdata.begin(), outputBuffer.begin(), outputBuffer.begin() + 511);
			
		}

		CloseHandle(disk);
		return true;
	}

	return false;
}

Win32IOHelper::~Win32IOHelper()
{

}
