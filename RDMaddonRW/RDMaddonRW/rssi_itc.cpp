//rssi_itc.cpp

#include "stdafx.h"
#include "rssi_itc.h"

#ifdef USE_RSSI_ITC

	//get WLAN RSSI
	typedef UINT (*PFN_GetRSSI)(int*);
	PFN_GetRSSI GetRSSI=NULL;

	// The loadlibrary HINSTANCE destination
	HINSTANCE h802lib=NULL;

	int getPercentRSSI(int iRSSI){
		//iRSSI = -30 to -100
		int iRet=iRSSI+100;
		iRet=(int)(iRet*1.4);
		return iRet;
	}

	int _rssilevel=-30;
	int _rssiValue=0;

	int getRSSILevel(){

	#ifdef MYDEBUG
		if(_rssilevel<-100)
			_rssilevel=-30;
		_rssilevel-=10;
		DEBUGMSG(1, (L"MYDEBUG: _rssilevel=%i\n", _rssilevel));
		return _rssilevel;
	#endif

		int iLevel = -100;
		if(h802lib == NULL)
			h802lib = LoadLibrary(_T("80211api.dll"));
		
		if(GetRSSI==NULL)
			GetRSSI = (PFN_GetRSSI)GetProcAddress(h802lib, _T("GetRSSI")); // Range is -100 dBm to -30 dBm
		if(GetRSSI!=NULL)
		{
			int iRes=0;
			if( (iRes=GetRSSI(&iLevel)) == 0 ){
				DEBUGMSG(1, (L"GetRSSI =%i\n", iLevel)); // -30 to -100
			}
			else{
				DEBUGMSG(1, (L"GetRSSI error=%i\n", iRes)); //ie ERR_CONNECT_FAILED
				return -100;
			}
		}
		else{
			DEBUGMSG(1, (L"Could not load GetRSSI: error %i \n", GetLastError()));
		}
		return iLevel;
	}

#endif //USE_RSSI_ITC