// RDMaddonRW.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "RDMaddonRW.h"

//#define MYDEBUG

//get WLAN RSSI
typedef UINT (*PFN_GetRSSI)(int*);
PFN_GetRSSI GetRSSI=NULL;

// The loadlibrary HINSTANCE destination
HINSTANCE h802lib=NULL;

#define MAX_LOADSTRING 100

// global vars
DWORD dwTimerID=1000;
UINT hTimer=NULL;
int _BattLevel=100;
int _WifiLevel=100;

//screen size
DWORD screenX=0;
DWORD screenY=0;
DWORD FLOATWIDTH	=	240;// Width of floating wnd
DWORD FLOATHEIGHT	=	27-2; // Height of floating wnd
DWORD blockMargin=1;
HWND g_hWnd=NULL;

HBITMAP hReweWWS=NULL;
HBITMAP hHome=NULL;
HBITMAP hShowSip=NULL;

HBITMAP hBatt100=NULL;
HBITMAP hBatt75=NULL;
HBITMAP hBatt50=NULL;
HBITMAP hBatt25=NULL;
HBITMAP hBatt0=NULL;
HBITMAP hBattCharging=NULL;
int battPosX=100;

HBITMAP hWifi100=NULL;
HBITMAP hWifi075=NULL;
HBITMAP hWifi050=NULL;
HBITMAP hWifi025=NULL;
HBITMAP hWifi000=NULL;
int wifiPosX=130;

void SipShowHide(BOOL bShow) 
{
	// TODO: Add your control notification handler code here
   if ( bShow )
      if(!SipShowIM(SIPF_ON))
		  DEBUGMSG(1, (L"SipShowIM failed: %i\n", GetLastError()));
   else
      SipShowIM(SIPF_OFF);
}

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
	int iRSSI=getPercentRSSI(_rssilevel);
	_rssilevel-=25/1.4;
	if(_rssilevel<-100)
		_rssilevel=-30;
	return iRSSI;
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

int _level=100;
int getBatteryLevel(){
	SYSTEM_POWER_STATUS_EX pwrStatus;
#ifdef MYDEBUG
	_level-=15;
	if(_level==1000)
		_level=100;
	if(_level<0)
		_level=1000;
	return _level;
#endif
	GetSystemPowerStatusEx(&pwrStatus, TRUE);
	if(pwrStatus.ACLineStatus==AC_LINE_ONLINE)
		return 1000;
	else
		return pwrStatus.BatteryLifePercent;
}

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RDMADDONRW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RDMADDONRW));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_RDMADDONRW, szWindowClass, MAX_LOADSTRING);
	
	//ReweWWS
	hReweWWS=LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_REWEWWS));
	hHome=LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_HOME));
	hShowSip=LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SHOWSIP));

	//battery display
	hBatt100 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BAT100));
	hBatt75 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BAT075));
	hBatt50 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BAT050));
	hBatt25 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BAT025));
	hBatt0 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BAT000));
	hBattCharging = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BATCHARGING));

	//wifi display
	hWifi100 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WIFI100));
	hWifi075 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WIFI075));
	hWifi050 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WIFI050));
	hWifi025 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WIFI025));
	hWifi000 = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WIFI000));

    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

//    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	HWND hWndTS = FindWindow(L"TSSHELLWND", NULL);
#ifndef MYDEBUG
	if(hWndTS==NULL){
		DEBUGMSG(1, (L"### TSSHELLWND not found. EXIT. ###\n"));
		return FALSE;
	}
#else
		hWndTS=GetForegroundWindow();
		DEBUGMSG(1, (L"### using foregroundwindow ###\n"));
#endif

    //hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
    //    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	screenX = GetSystemMetrics(SM_CXSCREEN);
	screenY = GetSystemMetrics(SM_CYSCREEN);
	DWORD dwX, dwY, dwW, dwH;

	//size and position
	dwX=0;
	dwY=screenY-FLOATHEIGHT;
	dwW=screenX;
	dwH=FLOATHEIGHT;

	hWnd = CreateWindowEx(
#ifdef USE_TRANSPARENCY
		WS_EX_LAYERED | WS_EX_TRANSPARENT,
#else
		0, //WS_EX_TOPMOST | WS_EX_ABOVESTARTUP, 
#endif
		szWindowClass, 
		NULL /* szTitle */, // NO CAPTION
		WS_VISIBLE, //WS_VISIBLE | WS_EX_ABOVESTARTUP, // | WS_EX_TOOLWINDOW | WS_CHILD | WS_POPUP | WS_NONAVDONEBUTTON, 
		dwX, 
		dwY, 
		dwW, 
		dwH, 
		hWndTS, // NULL, 
		NULL, 
		hInstance, 
		NULL);

    if (!hWnd)
    {
        return FALSE;
    }

	g_hWnd=hWnd;

	LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
	DEBUGMSG(1, (L"GetWindowLong GWL_SYTLE=%08x\n", lStyle));

	SetWindowPos(hWnd, HWND_TOPMOST, dwX, dwY, dwW, dwH, SWP_SHOWWINDOW);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
	RECT rect;
	static RECT rectSIPbitmap;

    static SHACTIVATEINFO s_sai;
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
                case IDM_OK:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);

			hTimer=SetTimer(hWnd, dwTimerID, 10000, NULL);
			
			_BattLevel=getBatteryLevel();
			_WifiLevel=getPercentRSSI(getRSSILevel());

            break;
		case WM_TIMER:
			if(wParam==dwTimerID){
#ifndef MYDEBUG
				if(FindWindow(L"TSSHELLWND", NULL)==NULL)
					PostQuitMessage(-2);
#endif
				_BattLevel = getBatteryLevel();
				DEBUGMSG(1, (L"getBatteryLevel=%i\n", _BattLevel));

				_WifiLevel = getPercentRSSI(getRSSILevel());
				DEBUGMSG(1, (L"getPercentRSSI=%i\n", _WifiLevel));

				GetClientRect(hWnd, &rect);
				InvalidateRect(hWnd, &rect, TRUE);
			}
			break;
		case WM_LBUTTONUP:
			int xPos, yPos;
			xPos = LOWORD(lParam); // horizontal position of cursor
			yPos = HIWORD(lParam); // vertical position of cursor
			if(	xPos>rectSIPbitmap.left && yPos>0 && xPos<rectSIPbitmap.right && yPos<rectSIPbitmap.bottom){
				DEBUGMSG(1, (L"SIP hit\n"));
				SipShowHide(TRUE);
			}else{
				DEBUGMSG(1, (L"SIP not hit\n"));
			}
			break;
        case WM_PAINT:
			BITMAP 			bitmap;
			HBITMAP			hBitmapBatt;
			HBITMAP			hBitmapWifi;
			HDC 			hdcMem;
			HGDIOBJ 		oldBitmap;
			int posX;

			hdc = BeginPaint(hWnd, &ps);
            
    		hdcMem = CreateCompatibleDC(hdc);

            // TODO: Add any drawing code here...
			//### Keypad
			oldBitmap=SelectObject(hdcMem, hShowSip);
			GetObject(hShowSip, sizeof(bitmap), &bitmap);
			posX=screenX-bitmap.bmWidth;
			BitBlt(hdc, posX, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
			rectSIPbitmap.top=0; rectSIPbitmap.left=posX; rectSIPbitmap.bottom=bitmap.bmHeight; rectSIPbitmap.right=posX+bitmap.bmWidth;

			//### BATTERY STUFF ###
			if(_BattLevel==1000){
				oldBitmap = SelectObject(hdcMem, hBattCharging);
				hBitmapBatt=hBattCharging;
			}
			else if(_BattLevel>75){
				oldBitmap = SelectObject(hdcMem, hBatt100);
				hBitmapBatt=hBatt100;
			}
			else if(_BattLevel>50){
				oldBitmap = SelectObject(hdcMem, hBatt75);
				hBitmapBatt=hBatt75;
			}
			else if(_BattLevel>25){
				oldBitmap = SelectObject(hdcMem, hBatt50);
				hBitmapBatt=hBatt50;
			}
			else if(_BattLevel>12){
				oldBitmap = SelectObject(hdcMem, hBatt25);
				hBitmapBatt=hBatt25;
			}
			else {
				oldBitmap = SelectObject(hdcMem, hBatt0);
				hBitmapBatt=hBatt0;
			}
			GetObject(hBatt100, sizeof(bitmap), &bitmap);
			posX=posX-bitmap.bmWidth;
			BitBlt(hdc, posX, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
			//### BATTERY STUFF END ###
			
			//### WIFI stuff
			if(_WifiLevel>75){
				oldBitmap = SelectObject(hdcMem, hWifi100);
				hBitmapWifi=hWifi100;
			}
			else if(_WifiLevel>50){
				oldBitmap = SelectObject(hdcMem, hWifi075);
				hBitmapWifi=hWifi075;
			}
			else if(_WifiLevel>25){
				oldBitmap = SelectObject(hdcMem, hWifi050);
				hBitmapWifi=hWifi050;
			}
			else if(_WifiLevel>12){
				oldBitmap = SelectObject(hdcMem, hWifi025);
				hBitmapWifi=hWifi025;
			}
			else {
				oldBitmap = SelectObject(hdcMem, hWifi000);
				hBitmapWifi=hWifi000;
			}
			GetObject(hWifi100, sizeof(bitmap), &bitmap);
			posX=posX-bitmap.bmWidth;
			BitBlt(hdc, posX, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
			//### WIFI STUFF END ###

			//### HOME
			oldBitmap=SelectObject(hdcMem, hHome);
			GetObject(hHome, sizeof(bitmap), &bitmap);
			posX=posX-bitmap.bmWidth;
			BitBlt(hdc, posX, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			//### ReweWWS
			oldBitmap=SelectObject(hdcMem, hReweWWS);
			GetObject(hReweWWS, sizeof(bitmap), &bitmap);
			//posX=posX-bitmap.bmWidth; 
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY); //left most position
			
			SelectObject(hdcMem, oldBitmap);

			DeleteDC(hdcMem);

            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            CommandBar_Destroy(g_hWndMenuBar);
            PostQuitMessage(0);
            break;

        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

    }
    return (INT_PTR)FALSE;
}
