// RDMaddonRW.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "RDMaddonRW.h"
#include "childwins.h"
#include "rssi_itc.h"

//#define MYDEBUG

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

BOOL _bSipShow=FALSE;

//get additinal extent of font
SIZE getTextSizeExtent(HWND hwnd, TCHAR* fontName, int FontSize, TCHAR* text){
	HDC hdc=GetDC(hwnd);
	PLOGFONT plf;
	HFONT hfnt, hfntPrev;
	plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT));
	// Specify a font typeface name and weight.
	lstrcpy(plf->lfFaceName, fontName);
	plf->lfHeight = -((FontSize * GetDeviceCaps(hdc, LOGPIXELSY)) / 72);//12;
	plf->lfWeight = FW_BOLD;
    plf->lfEscapement = 90*10;
	plf->lfOrientation = 90*10;
	plf->lfPitchAndFamily=FIXED_PITCH;
	SetTextAlign(hdc, TA_BASELINE);
	hfnt = CreateFontIndirect(plf);
	hfntPrev = (HFONT)SelectObject(hdc, hfnt);
	SIZE textSize;
	//calc textbox
	GetTextExtentPoint32(hdc, text, wcslen(text), &textSize); 
	
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	
	textSize.cy-= tm.tmDescent;

	SelectObject(hdc, hfntPrev);
	DeleteObject(hfnt);
	ReleaseDC(hwnd,hdc);
	return textSize;
}

void SipShowHide(BOOL bShow) 
{
	// TODO: Add your control notification handler code here
	if ( bShow ){
		if(!SipShowIM(SIPF_ON)){
			DEBUGMSG(1, (L"SipShowIM failed: %i\n", GetLastError()));
		}
	}
	else{
		if(!SipShowIM(SIPF_OFF)){
			DEBUGMSG(1, (L"SipShowIM failed: %i\n", GetLastError()));
		}
	}
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

//a thread that watches for TSSHELLWND and terminates this app, if TSSHELLWND not found
HANDLE hWatchThread=NULL;
DWORD dwWatchThreadID=0;
BOOL bStopNow=FALSE;
DWORD watchThread(LPVOID lParam){
	HWND hWnd=(HWND)lParam;
	DEBUGMSG(1, (L"watchThread started for hWnd=0x%08x...\n", hWnd));
	do{
		Sleep(5000);
	}while(FindWindow(L"TSSHELLWND", NULL)!=NULL && !bStopNow);
	PostMessage(hWnd, WM_CLOSE, -3, NULL);
	DEBUGMSG(1, (L"...watchThread ended\n"));
	//DWORD dwProcID; HANDLE hProc;
	//if(GetWindowThreadProcessId(hWnd, &dwProcID)){
	//	if ( (hProc=OpenProcess(SYNCHRONIZE, FALSE, dwProcID))!=INVALID_HANDLE_VALUE )
	//		TerminateProcess(hProc, -3);
	//}
	return 0;
}

int runProcess(TCHAR* szFullName, TCHAR* args){
	STARTUPINFO startInfo;
	memset(&startInfo, 0, sizeof(STARTUPINFO));
	PROCESS_INFORMATION processInformation;
	DWORD exitCode=0;

	if(CreateProcess(szFullName, args, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInformation)!=0){
		// Successfully created the process.  Wait for it to finish.
		DEBUGMSG(1, (L"Process '%s' started.\n", szFullName));

		// Close the handles.
		CloseHandle( processInformation.hProcess );
		CloseHandle( processInformation.hThread );
		return 0;
 	}
	else{
		//error
		DWORD dwErr=GetLastError();
		DEBUGMSG(1, (L"CreateProcess for '%s' failed with error code=%i\n", szFullName, dwErr));
		return -1;
	}
}

//############################### WINDOW STUF #########################################

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND hWndRDM;

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

	hWndRDM = FindWindow(L"TSSHELLWND", NULL);
#ifndef MYDEBUG
	if(hWndRDM==NULL){
		DEBUGMSG(1, (L"### TSSHELLWND not found. EXIT. ###\n"));
		return FALSE;
	}
#else
		hWndRDM=GetForegroundWindow();
		DEBUGMSG(1, (L"### using foregroundwindow ###\n"));
#endif

    //hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
    //    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	screenX = GetSystemMetrics(SM_CXSCREEN);
	screenY = GetSystemMetrics(SM_CYSCREEN);
	DWORD dwX, dwY, dwW, dwH;

	//resize and move RDM window
	RECT rectTS;
	//HWND hChildWin=FindChildWindowByParent(hWndRDM, L"UIMainClass");
	//GetWindowRect(hChildWin, &rectTS);
	GetWindowRect(hWndRDM, &rectTS);
	LONG lStyleTS = GetWindowLong(hWndRDM, GWL_STYLE);
	LONG lExStyleTS = GetWindowLong(hWndRDM, GWL_EXSTYLE);
	SetWindowLong(hWndRDM, GWL_STYLE, lStyleTS | WS_POPUP | WS_VISIBLE);
	SetWindowPos(hWndRDM, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
	
	//SetWindowPos(hChildWin, HWND_TOPMOST, rectTS.left, rectTS.top , rectTS.right, rectTS.bottom-FLOATHEIGHT, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	//SetWindowPos(hWndRDM, HWND_TOPMOST, rectTS.left, rectTS.top , rectTS.right, rectTS.bottom-FLOATHEIGHT, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	//MoveWindow(hWndRDM, rectTS.left, rectTS.top, rectTS.right, rectTS.bottom-FLOATHEIGHT, TRUE);
	//UpdateWindow(hWndRDM);

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
		hWndRDM, // NULL, 
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
	static RECT rectHomeButton;

    static SHACTIVATEINFO s_sai;
	static BOOL bToggle;
	static int iUpdateCount=0;

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

			hTimer=SetTimer(hWnd, dwTimerID, 1000, NULL);
			
			_BattLevel=getBatteryLevel();
			_WifiLevel=getPercentRSSI(getRSSILevel());

			hWatchThread = CreateThread(NULL, 0, watchThread, (LPVOID) hWnd, 0, &dwWatchThreadID);
			if(hWatchThread)
				DEBUGMSG(1, (L"Thread created for hWnd=0x%08x\n", hWnd));
			else
				DEBUGMSG(1, (L"Thread create FAILED\n"));
            break;
		case WM_TIMER:
			if(wParam==dwTimerID){
//#ifndef MYDEBUG
//				if(FindWindow(L"TSSHELLWND", NULL)==NULL)
//					PostQuitMessage(-2);
//#endif
				_BattLevel = getBatteryLevel();
				DEBUGMSG(1, (L"getBatteryLevel=%i\n", _BattLevel));

				_WifiLevel = getPercentRSSI(getRSSILevel());
				DEBUGMSG(1, (L"getPercentRSSI=%i\n", _WifiLevel));

				GetClientRect(hWnd, &rect);
				InvalidateRect(hWnd, &rect, FALSE);
			}
			break;
		case WM_LBUTTONUP:
			int xPos, yPos; HWND hWndLauncher; RECT rectLauncher;
			xPos = LOWORD(lParam); // horizontal position of cursor
			yPos = HIWORD(lParam); // vertical position of cursor
			//toggle
			_bSipShow=!_bSipShow;
			//left is absolute and right is width, top is absolute and bottom is height
			if(	xPos>rectSIPbitmap.left && yPos>0 && xPos<rectSIPbitmap.right+rectSIPbitmap.left && yPos<rectSIPbitmap.bottom){
				DEBUGMSG(1, (L"SIP hit\n"));
				SipShowHide(_bSipShow);
			}else{
				DEBUGMSG(1, (L"SIP not hit\n"));
			}
			if(xPos>rectHomeButton.left && yPos>0 && xPos<rectHomeButton.left+rectHomeButton.right && yPos<rectHomeButton.bottom){
				DEBUGMSG(1, (L"HOME hit\n"));
				//close RDM (minimize)
				PostMessage(hWndRDM, WM_CLOSE, 0, 0);
				//show Launcher
				hWndLauncher=FindWindow(NULL, L"Launcher");
				ShowWindow(hWndLauncher, SW_MAXIMIZE);
			}else{
				DEBUGMSG(1, (L"HOME not hit\n"));
			}

			break;
        case WM_PAINT:
			BITMAP 			bitmap;
			HBITMAP			hBitmapBatt;
			HBITMAP			hBitmapWifi;
			HDC 			hdcMem;
			HGDIOBJ 		oldBitmap;
			int posX;
			TCHAR myText[6]; RECT myRect; SYSTEMTIME myLocalTime; TEXTMETRIC tm;

			hdc = BeginPaint(hWnd, &ps);
            
    		hdcMem = CreateCompatibleDC(hdc);

            // TODO: Add any drawing code here...

			//the clock display
			GetLocalTime(&myLocalTime);
			if(bToggle)
				wsprintf(myText, L"%02i:%02i", myLocalTime.wHour, myLocalTime.wMinute);
			else
				wsprintf(myText, L"%02i %02i", myLocalTime.wHour, myLocalTime.wMinute);
			bToggle=!bToggle;
			//calc text rectangle needed
			GetTextMetrics(hdc, &tm); 
			myRect.right=screenX;// - 5*tm.tmAveCharWidth;
			myRect.left =screenX - 5*tm.tmAveCharWidth;
			myRect.top=0;
			myRect.bottom=FLOATHEIGHT;
			posX=screenX-myRect.left;

			SetBkColor(hdc, RGB(0,0,0));
			SetTextColor(hdc, RGB(160,160,160));
			DrawText(hdc,myText,wcslen(myText), &myRect, DT_CENTER | DT_VCENTER);

			//update the more static symbols only every tenth call
			if(iUpdateCount==0){
				//### Keypad
				oldBitmap=SelectObject(hdcMem, hShowSip);
				GetObject(hShowSip, sizeof(bitmap), &bitmap);
				posX=screenX - posX - bitmap.bmWidth;

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
				rectHomeButton.left=posX; rectHomeButton.top=0; rectHomeButton.right=bitmap.bmWidth; rectHomeButton.bottom=bitmap.bmHeight;

				//### ReweWWS
				oldBitmap=SelectObject(hdcMem, hReweWWS);
				GetObject(hReweWWS, sizeof(bitmap), &bitmap);
				//posX=posX-bitmap.bmWidth; 
				BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY); //left most position
				
				SelectObject(hdcMem, oldBitmap);
			}
			iUpdateCount++;
			if(iUpdateCount==10)
				iUpdateCount=0;

			DeleteDC(hdcMem);

            EndPaint(hWnd, &ps);
            break;
		case WM_CLOSE:
        case WM_DESTROY:
			DEBUGMSG(1, (L"WM_DESTROY received\n"));
            CommandBar_Destroy(g_hWndMenuBar);
            PostQuitMessage(0);
			bStopNow=TRUE;
			Sleep(5000);
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
