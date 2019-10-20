///////////////////////////////////////////////////////////////////////////////
//
// Raw Input API sample showing joystick support
//
// Author: Alexander B鯿ken
// Date:   04/22/2011
//
// Copyright 2011 Alexander B鯿ken
//
///////////////////////////////////////////////////////////////////////////////


#include <Windows.h>
#include <tchar.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <hidsdi.h>


#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#define WC_MAINFRAME	TEXT("MainFrame")
#define MAX_BUTTONS		128
#define CHECK(exp)		{ if(!(exp)) goto Error; }
#define SAFE_FREE(p)	{ if(p) { HeapFree(hHeap, 0, p); (p) = NULL; } }







//
// Global variables
//

BOOL pre_bButtonStates[MAX_BUTTONS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
LONG pre_lAxisX=0;
LONG pre_lAxisY=0;
LONG pre_rAxisX=0;
LONG pre_rAxisY=0;
LONG pre_lHat;

BOOL bButtonStates[MAX_BUTTONS];
LONG lAxisX;
LONG lAxisY;
LONG rAxisX;
LONG rAxisY;
LONG lHat;
INT  g_NumberOfButtons=0;

//按键关系
//bButtonStates序号：1 2 3 4 5 6  7  8  9  10 11    12    13    14       15 
//手柄按键：         A B   X Y   LT  RT LB RB BACK  START HOME  LMIDDLE  RMIDDLE
//对应键盘、鼠标：   1 2   3 4   tab t  lm rm alt   b     g     ctrl       mm
BYTE mapVKey[15] = { 0x31 ,0x32, 0, 0x33, 0x34, 0, VK_TAB , 0x54 , VK_LBUTTON , VK_RBUTTON , VK_MENU , 0x42 , 0x47 , VK_CONTROL , VK_SPACE };

// lAxisX, lAxisY  左摇杆
// 当前方向如果在正负50度内，就算a s d x之一
int preLeftDownFlag[4] = { 0,0,0,0 }; //指上一次左侧摇杆角度对应的方向
double  angleLeftInterval = 120;//指左侧各个按键对应的角度

// rAxisX, rAxisY  右摇杆
// 鼠标移动

// lHat  分8个方向，即

int GetAngleDownFlag(LONG x, LONG y, int outDownFlag[4])
{
    for (int i = 0; i < 4; ++i)
        outDownFlag[i] = 0;
    if (abs(x) < 20 && abs(y) < 20)
        return;
    //num 1
    double dLenLongEdge = sqrt(x * x + y * y);
    double cosAngleBoarder = cos(angleLeftInterval / 2 / 180.0*3.1415926);

    if (y < 0 && abs(1.0*y) / dLenLongEdge > cosAngleBoarder) {
        outDownFlag[1] = 1;
    }

    if (y > 0 && abs(1.0*y) / dLenLongEdge > cosAngleBoarder) {
        outDownFlag[0] = 1;
    }

    if (x < 0 && abs(1.0*x) / dLenLongEdge > cosAngleBoarder) {
        outDownFlag[2] = 1;
    }

    if (x > 0 && abs(1.0*x) / dLenLongEdge > cosAngleBoarder) {
        outDownFlag[3] = 1;
    }
}

int GetAngle(LONG x, LONG y)
{
    // 0 invalid 1 s 2 a 3 d
    if (abs(x) < 20 && abs(y)<20) {
        return 0;
    }
    if (x > 0) {
        if (y < 0 && 1.0*y / x < -0.58) {
            return 1;
        }
        else {
            return 3;
        }
    }
    else if(x<0){
        if (y<0 && 1.0*y / x > 0.58) {
            return 1;
        }
        else {
            return 2;
        }
    }
    else {
        if (y<0) {
            return 1;
        }
        else {
            return 2;
        }
    }
}


void ApplyCurToPre() {

    //需要做到的逻辑：
    //1. 对比当前的状态与之前的状态，如果有变化则做出相应的动作：手柄按键如果由无到有，则按下，反之则抬起
    for (int i = 0; i < 15; ++i)
    {
        if(i+1!=9 && i + 1 != 10 && i + 1 != 14)
        {
            if (pre_bButtonStates[i] != bButtonStates[i])
            {
                if (bButtonStates[i]) {
                    keybd_event(mapVKey[i], 0, 0, 0);
                }
                else {
                    keybd_event(mapVKey[i], 0, KEYEVENTF_KEYUP, 0);
                }
            }
        }
        else if(i + 1 == 9)
        {
            if (pre_bButtonStates[i] != bButtonStates[i])
            {
                if (bButtonStates[i]) {
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                }
                else {
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                }
            }
            
        }
        else if (i + 1 == 10) {
            if (pre_bButtonStates[i] != bButtonStates[i])
            {
                if (bButtonStates[i]) {
                    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                }
                else {
                    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                }
            }
        }
        else if (i + 1 == 14) {
            if (pre_bButtonStates[i] != bButtonStates[i])
            {
                if (bButtonStates[i]) {
                    mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
                }
                else {
                    mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
                }
            }
        }
        
        pre_bButtonStates[i] = bButtonStates[i];
    }

    //2. 左摇杆看角度，如果进入一个asdx角度，则按下相应键，否则抬起相应键
    // 0 invalid 1 s 2 a 3 d
    //int prevAngle = GetAngle(pre_lAxisX,pre_lAxisY);
    //int curAngle = GetAngle(lAxisX, lAxisY);
    BYTE mapAngleVKey[4] = { 0,0x53 ,0x41 ,0x44 };
    int tempLeftDownFlag[4];
    GetAngleDownFlag(lAxisX, lAxisY, tempLeftDownFlag);
    for (int i = 0; i < 4; ++i)
    {
        if (tempLeftDownFlag[i] != preLeftDownFlag[i])
        {
            if(1 == tempLeftDownFlag[i])
                keybd_event(mapAngleVKey[i], 0, 0, 0);
            else
                keybd_event(mapAngleVKey[i], 0, KEYEVENTF_KEYUP, 0);
        }
        preLeftDownFlag[i] = tempLeftDownFlag[i];
    }
    //if (curAngle != prevAngle)
    //{
    //    if (prevAngle != 0)
    //    {
    //        keybd_event(mapAngleVKey[prevAngle], 0, KEYEVENTF_KEYUP, 0);
    //    }
    //    if (curAngle != 0)
    //    {
    //        keybd_event(mapAngleVKey[curAngle], 0, 0, 0);
    //    }
    //}
    //pre_lAxisX = lAxisX;
    //pre_lAxisY = lAxisY;

//3. 右摇杆看角度和距离，向相应的位置移动鼠标
    mouse_event(MOUSEEVENTF_MOVE, rAxisX / 20, rAxisY / 20, 0, 0);


    pre_rAxisX = rAxisX;
    pre_rAxisY = rAxisY;

//4. 十字键，左（6），上（0），右（2），下（4）左右分别代表qwef
    BYTE maplHatVKey[7] = { 0x57,0,0x45,0,0x46 ,0,0x51 };
    if (lHat != pre_lHat) {
        if (pre_lHat < 7 && maplHatVKey[pre_lHat]!=0) {
            keybd_event(maplHatVKey[pre_lHat], 0, KEYEVENTF_KEYUP, 0);
        }
        if(lHat < 7 && maplHatVKey[lHat] != 0) {
            keybd_event(maplHatVKey[lHat], 0, 0, 0);
        }
        pre_lHat = lHat;
    }
}

void ParseRawInput(PRAWINPUT pRawInput)
{

	PHIDP_PREPARSED_DATA pPreparsedData;
	HIDP_CAPS            Caps;
	PHIDP_BUTTON_CAPS    pButtonCaps;
	PHIDP_VALUE_CAPS     pValueCaps;
	USHORT               capsLength;
	UINT                 bufferSize;
	HANDLE               hHeap;
	USAGE                usage[MAX_BUTTONS];
	ULONG                i, usageLength, value;

	pPreparsedData = NULL;
	pButtonCaps    = NULL;
	pValueCaps     = NULL;
	hHeap          = GetProcessHeap();

	//
	// Get the preparsed data block
	//

	CHECK( GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize) == 0 );
	CHECK( pPreparsedData = (PHIDP_PREPARSED_DATA)HeapAlloc(hHeap, 0, bufferSize) );
	CHECK( (int)GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize) >= 0 );

	//
	// Get the joystick's capabilities
	//

	// Button caps
	CHECK( HidP_GetCaps(pPreparsedData, &Caps) == HIDP_STATUS_SUCCESS )
	CHECK( pButtonCaps = (PHIDP_BUTTON_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps) );

	capsLength = Caps.NumberInputButtonCaps;
	CHECK( HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS )
	g_NumberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

	// Value caps
	CHECK( pValueCaps = (PHIDP_VALUE_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps) );
	capsLength = Caps.NumberInputValueCaps;
	CHECK( HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS )

	//
	// Get the pressed buttons
	//

	usageLength = g_NumberOfButtons;
	CHECK(
		HidP_GetUsages(
			HidP_Input, pButtonCaps->UsagePage, 0, usage, &usageLength, pPreparsedData,
			(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid
		) == HIDP_STATUS_SUCCESS );

	ZeroMemory(bButtonStates, sizeof(bButtonStates));
	for(i = 0; i < usageLength; i++)
		bButtonStates[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

	//
	// Get the state of discrete-valued-controls
	//

	for(i = 0; i < Caps.NumberInputValueCaps; i++)
	{
		CHECK(
			HidP_GetUsageValue(
				HidP_Input, pValueCaps[i].UsagePage, 0, pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
				(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid
			) == HIDP_STATUS_SUCCESS );

		switch(pValueCaps[i].Range.UsageMin)
		{
		case 0x30:	// X-axis
			lAxisX = (LONG)value - 128;
			break;

		case 0x31:	// Y-axis
			lAxisY = (LONG)value - 128;
			break;

		case 0x32: // Z-axis
			rAxisX = (LONG)value - 128;
			break;

		case 0x35: // Rotate-Z
			rAxisY = (LONG)value - 128;
			break;

		case 0x39:	// Hat Switch
			lHat = value;
			break;
		}
	}

	//
	// Clean up
	//
    ApplyCurToPre();
Error:
	SAFE_FREE(pPreparsedData);
	SAFE_FREE(pButtonCaps);
	SAFE_FREE(pValueCaps);
}


void DrawButton(HDC hDC, int i, int x, int y, BOOL bPressed)
{
	HBRUSH hOldBrush, hBr;
	TCHAR  sz[4];
	RECT   rc;

	if(bPressed)
	{
		hBr       = CreateSolidBrush(RGB(192, 0, 0));
		hOldBrush = (HBRUSH)SelectObject(hDC, hBr);
	}

	rc.left   = x;
	rc.top    = y;
	rc.right  = x + 30;
	rc.bottom = y + 30;
	Ellipse(hDC, rc.left, rc.top, rc.right, rc.bottom);
	_stprintf_s(sz, ARRAY_SIZE(sz), TEXT("%d"), i);
	DrawText(hDC, sz, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	if(bPressed)
	{
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBr);
	}
}


void DrawCrosshair(HDC hDC, int x, int y, LONG xVal, LONG yVal)
{
	Rectangle(hDC, x, y, x + 256, y + 256);
	MoveToEx(hDC, x + xVal - 5 + 128, y + yVal + 128, NULL);
	LineTo(hDC, x + xVal + 5 + 128, y + yVal + 128);
	MoveToEx(hDC, x + xVal + 128, y + yVal - 5 + 128, NULL);
	LineTo(hDC, x + xVal + 128, y + yVal + 5 + 128);
}


void DrawDPad(HDC hDC, int x, int y, LONG value)
{
	LONG i;

	for(i = 0; i < 8; i++)
	{
		HBRUSH hOldBrush;
		HBRUSH hBr;
		int xPos = (int)(sin(-2 * M_PI * i / 8 + M_PI) * 80.0) + 80;
		int yPos = (int)(cos(2 * M_PI * i / 8 + M_PI) * 80.0) + 80;

		if(value == i)
		{
			hBr       = CreateSolidBrush(RGB(192, 0, 0));
			hOldBrush = (HBRUSH)SelectObject(hDC, hBr);
		}

		Ellipse(hDC, x + xPos, y + yPos, x + xPos + 20, y + yPos + 20);

		if(value == i)
		{
			SelectObject(hDC, hOldBrush);
			DeleteObject(hBr);
		}
	}
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			//
			// Register for joystick devices
			//

			RAWINPUTDEVICE rid[2];

			rid[0].usUsagePage = 1;
			rid[0].usUsage     = 4;	// Joystick
			rid[0].dwFlags     = RIDEV_INPUTSINK; // Recieve messages when in background.
			rid[0].hwndTarget  = hWnd;

			
			rid[1].usUsagePage = 1;
			rid[1].usUsage	   = 5;	// Gamepad - e.g. XBox 360 or XBox One controllers
			rid[1].dwFlags     = RIDEV_INPUTSINK; // Recieve messages when in background.
			rid[1].hwndTarget = hWnd;

			if(!RegisterRawInputDevices(&rid, 2, sizeof(RAWINPUTDEVICE)))
				return -1;
		}
		return 0;

	case WM_INPUT:
		{
			//
			// Get the pointer to the raw device data, process it and update the window
			//

			PRAWINPUT pRawInput;
			UINT      bufferSize;
			HANDLE    hHeap;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));

			hHeap     = GetProcessHeap();
			pRawInput = (PRAWINPUT)HeapAlloc(hHeap, 0, bufferSize);
			if(!pRawInput)
				return 0;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, pRawInput, &bufferSize, sizeof(RAWINPUTHEADER));
			ParseRawInput(pRawInput);

			HeapFree(hHeap, 0, pRawInput);

			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
		}
		return 0;

	case WM_PAINT:
		{
			//
			// Draw the buttons and axis-values
			//

			PAINTSTRUCT ps;
			HDC         hDC;
			int         i;

			//hDC = BeginPaint(hWnd, &ps);
            HDC hCompatibleDC = BeginPaint(hWnd, &ps); //CreateCompatibleDC(hDC);
            SetBkMode(hCompatibleDC, TRANSPARENT);

			for(i = 0; i < g_NumberOfButtons; i++)
				DrawButton(hCompatibleDC, i+1, 20 + i * 40, 20, bButtonStates[i]);
			DrawCrosshair(hCompatibleDC, 20, 100, lAxisX, lAxisY);
			DrawCrosshair(hCompatibleDC, 296, 100, rAxisX, rAxisY);
			DrawDPad(hCompatibleDC, 600, 140, lHat);

            RECT rect;
            GetClientRect(hWnd,&rect);

            //BitBlt(hDC,0,0, rect.right-rect.left,rect.bottom-rect.top,hCompatibleDC,0,0, SRCCOPY);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wcex;

	//
	// Register window class
	//

	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hbrBackground = NULL;
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hInstance     = hInstance;
	wcex.lpfnWndProc   = WindowProc;
	wcex.lpszClassName = WC_MAINFRAME;
	wcex.lpszMenuName  = NULL;
	wcex.style         = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClassEx(&wcex))
		return -1;

	//
	// Create window
	//

	hWnd = CreateWindow(WC_MAINFRAME, TEXT("Joystick using Raw Input API"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	//
	// Message loop
	//

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
