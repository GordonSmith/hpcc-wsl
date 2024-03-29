#include "resource.h"

// wsl -u root -e /etc/init.d/hpcc-init status
// wsl -u root -e /etc/init.d/hpcc-init status
// wsl -u root -e /etc/init.d/hpcc-init status
// wsl -u root -e /etc/init.d/dafilesrv stop

/*variables*/
UINT WM_TASKBAR = 0;
HWND Hwnd;
HMENU Hmenu;
NOTIFYICONDATA notifyIconData;
TCHAR szTIP[64] = TEXT("hpcc-wsl");
char szClassName[] = "hpcc-wsl";

/*procedures  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void minimize();
void restore();
void InitNotifyIconData();

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nCmdShow)
{
    /* This is the handle for our window */
    MSG messages;     /* Here messages to the application are saved */
    WNDCLASSEX wincl; /* Data structure for the windowclass */
    WM_TASKBAR = RegisterWindowMessageA("TaskbarCreated");
    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure; /* This function is called by windows */
    wincl.style = CS_DBLCLKS;            /* Catch double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
    wincl.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL; /* No menu */
    wincl.cbClsExtra = 0;      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;      /* structure or the window instance */
    wincl.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx(&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    Hwnd = CreateWindowEx(
        0,                   /* Extended possibilites for variation */
        szClassName,         /* Classname */
        szClassName,         /* Title Text */
        WS_OVERLAPPEDWINDOW, /* default window */
        CW_USEDEFAULT,       /* Windows decides the position */
        CW_USEDEFAULT,       /* where the window ends up on the screen */
        544,                 /* The programs width */
        375,                 /* and height in pixels */
        HWND_DESKTOP,        /* The window is a child-window to desktop */
        NULL,                /* No menu */
        hThisInstance,       /* Program Instance handler */
        NULL                 /* No Window Creation data */
    );
    /*Initialize the NOTIFYICONDATA structure only once*/
    InitNotifyIconData();
    /* Make the window visible on the screen */
    ShowWindow(Hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    return static_cast<int>(messages.wParam);
}

void blockingShellExec(const char *cmd, const char *params, bool show = true)
{
    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = cmd;
    ShExecInfo.lpParameters = params;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = show ? SW_SHOW : SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    CloseHandle(ShExecInfo.hProcess);
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    if (message == WM_TASKBAR && !IsWindowVisible(Hwnd))
    {
        minimize();
        return 0;
    }

    switch (message) /* handle the messages */
    {
    case WM_USER + 100:
        ShellExecute(0, "open", "cmd.exe", "/C wsl -u root -e /etc/init.d/hpcc-init start", 0, SW_SHOW);
        break;

    case WM_USER + 101:
        blockingShellExec("wsl.exe", "-u root -e /etc/init.d/hpcc-init status");
        DestroyWindow(Hwnd);
        break;

    case WM_ACTIVATE:
        Shell_NotifyIcon(NIM_ADD, &notifyIconData);
        break;

    case WM_CREATE:
        ShowWindow(Hwnd, SW_HIDE);
        Hmenu = CreatePopupMenu();
        AppendMenu(Hmenu, MF_STRING, ID_TRAY_START, TEXT("&Start"));
        AppendMenu(Hmenu, MF_STRING, ID_TRAY_STOP, TEXT("S&top"));
        AppendMenu(Hmenu, MF_SEPARATOR, ID_TRAY_SEP, TEXT(""));
        AppendMenu(Hmenu, MF_STRING, ID_TRAY_STATUS, TEXT("St&atus"));
        AppendMenu(Hmenu, MF_SEPARATOR, ID_TRAY_SEP, TEXT(""));
        AppendMenu(Hmenu, MF_STRING, ID_TRAY_EXIT, TEXT("E&xit"));

        PostMessage(hwnd, WM_USER + 100, 0, 0);
        break;

    case WM_SYSCOMMAND:
        /*In WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter 
		are used internally by the system. To obtain the correct result when testing the value of wParam, 
		an application must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator.*/

        switch (wParam & 0xFFF0)
        {
        case SC_MINIMIZE:
            minimize();
            return 0;

        case SC_CLOSE:
            break;
        }
        break;

        // Our user defined WM_SYSICON message.
    case WM_SYSICON:
    {
        switch (wParam)
        {
        case ID_TRAY_APP_ICON:
            SetForegroundWindow(Hwnd);
            break;
        }

        if (lParam == WM_LBUTTONUP)
        {
            restore();
        }
        else if (lParam == WM_RBUTTONDOWN)
        {
            // Get current mouse position.
            POINT curPoint;
            GetCursorPos(&curPoint);
            SetForegroundWindow(Hwnd);

            // TrackPopupMenu blocks the app until TrackPopupMenu returns

            UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

            SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
            switch (clicked)
            {
            case ID_TRAY_START:
                ShellExecute(0, "open", "cmd.exe", "/C wsl -u root -e /etc/init.d/hpcc-init start", 0, SW_SHOW);
                break;
            case ID_TRAY_STOP:
                ShellExecute(0, "open", "cmd.exe", "/C wsl -u root -e /etc/init.d/hpcc-init stop", 0, SW_SHOW);
                break;
            case ID_TRAY_STATUS:
                ShellExecute(0, "open", "cmd.exe", "/C wsl -u root -e /etc/init.d/hpcc-init status && pause", 0, SW_SHOW);
                break;
            case ID_TRAY_EXIT:
                DestroyWindow(Hwnd);
                return 0;
            }
        }
    }
    break;

    // intercept the hittest message..
    case WM_NCHITTEST:
    {
        LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
        if (uHitTest == HTCLIENT)
            return HTCAPTION;
        else
            return uHitTest;
    }

    case WM_ENDSESSION:
        blockingShellExec("wsl.exe", "-u root -e /etc/init.d/hpcc-init stop");
        break;

    case WM_CLOSE:
        break;

    case WM_DESTROY:
        blockingShellExec("wsl.exe", "-u root -e /etc/init.d/hpcc-init stop");
        Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void minimize()
{
    ShowWindow(Hwnd, SW_HIDE);
}

void restore()
{
    ShowWindow(Hwnd, SW_SHOW);
}

void InitNotifyIconData()
{
    memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

    notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    notifyIconData.hWnd = Hwnd;
    notifyIconData.uID = ID_TRAY_APP_ICON;
    notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    notifyIconData.uCallbackMessage = WM_SYSICON;
    notifyIconData.hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
    strncpy_s(notifyIconData.szTip, szTIP, sizeof(szTIP));
}
