// TL2.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TL2.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <sstream>
#include "serverlist.pb.h"
#include "pipe.h"
#include <shellapi.h>


#define MAX_LOADSTRING 100

#define DEBUG true

// Global Variables:
HINSTANCE hInst;                                                    // current instance
WCHAR szTitle[MAX_LOADSTRING] = L"LAUNCHER_WINDOW";                 // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING] = L"LAUNCHER_CLASS";            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL handleWmCopy(HWND, UINT, WPARAM, LPARAM);

void pipe_thread_function(HANDLE hPipe, DWORD Process_Id);
unsigned int ipToInt(std::string ipAddress);
std::string wstring_to_bytes(const std::wstring& str);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    
#ifdef DEBUG
    //create console
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout); //redirect stdio to console
#endif

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < 2) {
        MessageBox(NULL, L"Please specify a path to TERA.exe", L"Error", MB_OK);
        return 0;
    }
    WCHAR TERA_EXE[MAX_PATH];
    wcscpy_s(TERA_EXE, argv[1]);
    WCHAR TERA_PATH[MAX_PATH];
    wcscpy_s(TERA_PATH, argv[2]);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (!CreateProcess(NULL,  // No module name (use command line)
        TERA_EXE,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_SUSPENDED,              // create process suspended
        NULL,           // Use parent's environment block
        TERA_PATH,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
		printf("CreateProcess failed (%d).\n", GetLastError());
		return FALSE;
	}
    
    //create named pipe in new thread
    HANDLE hPipe;
    std::thread pipe_thread(pipe_thread_function, hPipe, pi.dwProcessId);
    // resume process after creating pipe
    pipe_thread.detach();
    ResumeThread(pi.hThread);


    // Connect to launcher pipe
    //connectToLauncher(TEXT("\\\\.\\pipe\\tera_launcher"));
    //if (hPipeLauncher == INVALID_HANDLE_VALUE) {
	//	printf("Failed to connect to launcher pipe\n");
	//	return FALSE;
	//}
    //else {
	//	printf("Connected to launcher pipe\n");
	//}

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TL2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
void pipe_thread_function(HANDLE hPipe, DWORD Process_Id) {
    //create named pipe
    std::wstring PIPE_NAME = L"\\\\.\\pipe\\" + std::to_wstring(Process_Id) + L"cout";
    std::wcout << PIPE_NAME << std::endl;
    hPipe = CreateNamedPipe(
        PIPE_NAME.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024 * 16,
        1024 * 16,
        0,
        NULL
    );
    ConnectNamedPipe(hPipe, NULL);

    //CloseHandle(hPipe);
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TL2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TL2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_COPYDATA:
    {
        return handleWmCopy(hWnd, message, wParam, lParam);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

unsigned int ipToInt(std::string ipAddress) {
    std::stringstream ss(ipAddress);
    unsigned int a, b, c, d;
    char ch;
    ss >> a >> ch >> b >> ch >> c >> ch >> d;
    return (a << 24) + (b << 16) + (c << 8) + d;
}

std::string wstring_to_bytes(const std::wstring& str) {
    return std::string(reinterpret_cast<const char*>(str.data()), str.size() * sizeof(wchar_t));
}

BOOL handleWmCopy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    COPYDATASTRUCT* pCDS = (COPYDATASTRUCT*)lParam;
    std::cout << "Event: " << std::to_string(pCDS->dwData) << std::endl;
    switch (pCDS->dwData) {
    case ACCOUNT:
    {
        //int requestCode = ACCOUNT;
        //CopyMemory(sendBuff, &requestCode, sizeof(requestCode));
        //cbSend = sizeof(requestCode);
        std::cout << "Request Account" << std::endl;
        //sendToLauncher();
        //return waitForRecv(hWnd, message, wParam, lParam, ACCOUNT);
        std::wstring* account = new std::wstring(L"2");
        COPYDATASTRUCT cds = {2, account->size()*sizeof(std::wstring::traits_type::char_type), (PVOID)account->data()};
        SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));
        delete account;
        break;
    }
    case TOKEN:
    {
        //int requestCode = TOKEN;
        //CopyMemory(sendBuff, &requestCode, sizeof(requestCode));
        //cbSend = sizeof(requestCode);
        std::cout << "Request Token" << std::endl;
        //sendToLauncher();
        //return waitForRecv(hWnd, message, wParam, lParam, TOKEN);
        std::string* token = new std::string("40ec99b2-cb51-4f02-b5b3-153514f87bf1");
        COPYDATASTRUCT cds = {4, token->size(), (PVOID)token->data()};
        SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));
        delete token;
        break;

    }
    case SERVER_LIST:
    {
        //int requestCode = SERVER_LIST;
        //CopyMemory(sendBuff, &requestCode, sizeof(requestCode));
        //cbSend = sizeof(requestCode);
        std::cout << "Request Server List" << std::endl;
        //sendToLauncher();
        //return waitForRecv(hWnd, message, wParam, lParam, SERVER_LIST);

        TeraLauncher::ServerList* serverList = new TeraLauncher::ServerList();
        TeraLauncher::ServerList::Server* server = new TeraLauncher::ServerList::Server();

        server->set_id(2800);
        server->set_rawname(wstring_to_bytes(L"Test Server"));
        server->set_category(wstring_to_bytes(L"PVE"));
        server->set_name(wstring_to_bytes(L"Test Server"));
        server->set_crowdness(wstring_to_bytes(L"No"));
        server->set_open(wstring_to_bytes(L"Normal"));
        server->set_ip(ipToInt("192.168.0.168"));
        server->set_port(7801);
        server->set_lang(6);
        server->set_popup(wstring_to_bytes(L"Unable to connect to server."));
        serverList->add_servers()->CopyFrom(*server);
        serverList->set_lastplayedid(0);
        serverList->set_unknwn2(0);

        int size = serverList->ByteSizeLong();
        char* data = new char[size];
        serverList->SerializeToArray(data, size);
        COPYDATASTRUCT cds {6, size, (PVOID)data};
        SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));
        delete serverList;
        delete server;
        delete [] data;
        std::cout << "ip: " << ipToInt("192.168.0.168") << std::endl;
        break;
    }
    case 1020: {
        // on game close close launcher process
        //PostQuitMessage(0);
        break;
    }
    }
    return 1;
}