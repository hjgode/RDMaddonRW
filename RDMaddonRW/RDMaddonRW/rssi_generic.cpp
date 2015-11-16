//	rssi_generic.cpp

#include "stdafx.h"
#include "rssi_generic.h"

#ifndef USE_RSSI_ITC
	#include "WifiPeek.h"

	#define UNKNOWERROR -250
	#define LOADLIBFAILED -251
	#define CONNECTFAILED -252
	#define QUERYFAILED -253
	#define SCANNING -254
	#define NOTASSOCIATED -255

	//for WiFi Peek usage
	//////////////////////////////////////////////////////////////////////////
	INT GetSignalStrength(TCHAR *ptcDeviceName, INT *piSignalStrength)
	{
		PNDISUIO_QUERY_OID queryOID;
		DWORD dwBytesReturned = 0;
		UCHAR QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];
		HANDLE ndisAccess = INVALID_HANDLE_VALUE;
		BOOL retval;
		INT hr;

		// Attach to NDISUIO.
		ndisAccess = CreateFile(NDISUIO_DEVICE_NAME, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, INVALID_HANDLE_VALUE );

		if (ndisAccess == INVALID_HANDLE_VALUE) 
			return -1;

		// Get Signal strength
		queryOID = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
		queryOID->ptcDeviceName = ptcDeviceName;
		queryOID->Oid = OID_802_11_RSSI;

		retval = DeviceIoControl(ndisAccess, IOCTL_NDISUIO_QUERY_OID_VALUE, (LPVOID)queryOID, sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD), (LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD), &dwBytesReturned, NULL);

		if (retval && piSignalStrength)
		{
			hr = 0;
			*piSignalStrength = *(DWORD *)&queryOID->Data;
		}
		else
		{
			DEBUGMSG(1, (L"### DeviceIoControl failed: retval=%i, LastError=%i\n", retval, GetLastError()));
			hr = -2;
		}

		CloseHandle(ndisAccess);

		return(hr);
	}

	CWifiPeek m_wp;
	TCHAR* szAdapterName=new TCHAR[64];

	ULONG GetCurrentValue(){
		WCHAR buf[1024];
		DWORD dwSize;
		dwSize=sizeof(buf);
		if(false == m_wp.GetAdapters(buf, dwSize) || dwSize == 0)
		{
			return UNKNOWERROR;
		}
		else{
			//we are only interested in first returned name
			wsprintf(szAdapterName, L"%s", buf);
			int iRSSI=0;
			int iRes = GetSignalStrength(szAdapterName, &iRSSI);
			if(iRes == ERROR_SUCCESS){
				return iRSSI; //ie -45
			}
			else{
				return LOADLIBFAILED;
			}
		}
	}

	int getPercentRSSI(int iRSSI){
		//iRSSI = -30 to -100
		int iRet=iRSSI+100;
		iRet=(int)(iRet*1.4);
		return iRet;
	}

	int getRSSILevel(){
		return GetCurrentValue();
		//return 0;
	}

#endif