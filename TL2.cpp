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
#include <filesystem>


#define MAX_LOADSTRING 100

//#define NOARGS
//#define STANDALONE

#ifdef STANDALONE //useful for debugging
#define IP "169.254.77.105"
#define PORT 7801
//sent in C_LOGIN_ARBITER
#define ACCOUNTNAME L"2"
#define TICKET ""
#define DLANGUAGEEXT L"EUR"
#endif // STANDALONE
std::wstring ExePath();

WCHAR ACCOUNT[MAX_PATH];
WCHAR AUTH_TICKET[MAX_PATH];
DWORD TERAPID;
WCHAR DLL_PATH[MAX_PATH];
bool injectDLL = false;

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
    
#ifdef _DEBUG
    //create console
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout); //redirect stdio to console
#endif
	//Process Input Arguments
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
#ifndef NOARGS
    if (argc < 4) {
        MessageBox(NULL, L"Please specify a path to TERA.exe \nEx:\"TL2.exe [EXE_PATH] [LANGUAGEEXT] [PIPE_ID] [AttachDLL]", L"Error", MB_OK);
        return 0;
    }
	//Standard Arguments
    WCHAR TERA_EXE_PATH[MAX_PATH];
    wcscpy_s(TERA_EXE_PATH, argv[1]);
    WCHAR LANGUAGEEXT[20];
    wcscpy_s(LANGUAGEEXT, argv[2]);
    WCHAR PIPE_ID[20];
    wcscpy_s(PIPE_ID, argv[3]);
	//check if DLL path is provided
    if (argc > 4) {
        injectDLL = true;
        wcscpy_s(DLL_PATH, argv[4]);
    }
    
	//get TERA_EXE_PATH without the exe name
#else
    std::wstring PATH = std::filesystem::current_path();
    std::wstring TERA_EXE_PATH = PATH.append(L"\\Client\\Binaries\\TERA.exe");
    std::wstring LANGUAGEEXT = DLANGUAGEEXT;
#endif
	std::wstring TERA_PATH = TERA_EXE_PATH;
	std::wstring::size_type pos = TERA_PATH.find_last_of(L"\\/");
	if (pos != std::wstring::npos) {
		TERA_PATH = TERA_PATH.substr(0, pos);
	}
	//create command to start TERA (TERA_EXE_PATH -LANGUAGEEXT=LANGUAGEEXT)
	std::wstring TERA_CMD = TERA_EXE_PATH;
    TERA_CMD += L" -LANGUAGEEXT=";
    TERA_CMD += LANGUAGEEXT;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL,        // No module name (use command line)
        (TCHAR*)TERA_CMD.c_str(),   // Command line
        NULL,                       // Process handle not inheritable
        NULL,                       // Thread handle not inheritable
        FALSE,                      // Set handle inheritance to FALSE
        CREATE_SUSPENDED,           // create process suspended
        NULL,                       // Use parent's environment block
        (TCHAR*)TERA_PATH.c_str(),  // Use parent's starting directory 
        &si,                        // Pointer to STARTUPINFO structure
        &pi )                       // Pointer to PROCESS_INFORMATION structure
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
	TERAPID = pi.dwProcessId;
	
    ResumeThread(pi.hThread);

#ifndef STANDALONE
    // Connect to launcher pipe
	std::wstring LAUNCHER_PIPE_NAME = L"\\\\.\\pipe\\tera_launcher" + std::wstring(PIPE_ID);
	std::wcout << LAUNCHER_PIPE_NAME << std::endl;

	connectToLauncher(LAUNCHER_PIPE_NAME.c_str());
    if (hPipeLauncher == INVALID_HANDLE_VALUE) {
    	printf("Failed to connect to launcher pipe\n");
    	return FALSE;
    }
    else {
    	printf("Connected to launcher pipe\n");
    }
#endif // !STANDALONE

    

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
    std::cout << "Tera Client Connected to Pipe" << std::endl;
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
#ifdef _DEBUG
#define eventIdLog(eventEnum, eventStr) case eventEnum: {std::cout << eventStr << std::endl; break;}
void LogEvent(int eventId) {
    switch (eventId) {
        eventIdLog(ACCOUNT_REQUEST, "LAUNCHER_GAME_EVENT_ACCOUNT_REQUEST")
        eventIdLog(TOKEN_REQUEST, "LAUNCHER_GAME_EVENT_TOKEN_REQUEST")
        eventIdLog(SERVER_LIST_REQUEST, "LAUNCHER_GAME_EVENT_SERVER_LIST_REQUEST")
        eventIdLog(LAUNCHER_GAME_EVENT_ENTERED_CHARACTER_CREATION, "LAUNCHER_GAME_EVENT_ENTERED_CHARACTER_CREATION")
        eventIdLog(LAUNCHER_GAME_EVENT_ENTERED_SERVER_LIST, "LAUNCHER_GAME_EVENT_ENTERED_SERVER_LIST")
        eventIdLog(LAUNCHER_GAME_EVENT_ENTERING_LOBBY, "LAUNCHER_GAME_EVENT_ENTERING_LOBBY")
        eventIdLog(LAUNCHER_GAME_EVENT_ENTERED_LOBBY, "LAUNCHER_GAME_EVENT_ENTERED_LOBBY")
        eventIdLog(LAUNCHER_GAME_EVENT_ENTERING_CHARACTER_CREATION, "LAUNCHER_GAME_EVENT_ENTERING_CHARACTER_CREATION")
        eventIdLog(LAUNCHER_GAME_EVENT_LEFT_LOBBY, "LAUNCHER_GAME_EVENT_LEFT_LOBBY")
        eventIdLog(LAUNCHER_GAME_EVENT_DELETED_CHARACTER, "LAUNCHER_GAME_EVENT_DELETED_CHARACTER")
        eventIdLog(LAUNCHER_GAME_EVENT_CANCELED_CHARACTER_CREATION, "LAUNCHER_GAME_EVENT_CANCELED_CHARACTER_CREATION")
        eventIdLog(LAUNCHER_GAME_EVENT_CREATED_CHARACTER, "LAUNCHER_GAME_EVENT_CREATED_CHARACTER")
        eventIdLog(LAUNCHER_GAME_EVENT_ENTERED_WORLD, "LAUNCHER_GAME_EVENT_ENTERED_WORLD")
        eventIdLog(LAUNCHER_GAME_EVENT_FINISHED_LOADING_SCREEN, "LAUNCHER_GAME_EVENT_FINISHED_LOADING_SCREEN")
        eventIdLog(LAUNCHER_GAME_EVENT_LEFT_WORLD, "LAUNCHER_GAME_EVENT_LEFT_WORLD")
        eventIdLog(LAUNCHER_GAME_EVENT_MOUNTED_PEGASUS, "LAUNCHER_GAME_EVENT_MOUNTED_PEGASUS")
        eventIdLog(LAUNCHER_GAME_EVENT_DISMOUNTED_PEGASUS, "LAUNCHER_GAME_EVENT_DISMOUNTED_PEGASUS")
        eventIdLog(LAUNCHER_GAME_EVENT_CHANGED_CHANNEL, "LAUNCHER_GAME_EVENT_CHANGED_CHANNEL")
    }
}
#endif // _DEBUG



BOOL handleWmCopy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    COPYDATASTRUCT* pCDS = (COPYDATASTRUCT*)lParam;
#ifdef _DEBUG
    std::cout << "Event: " << std::to_string(pCDS->dwData) << std::endl;
    LogEvent(pCDS->dwData);
#endif
#define DefaultSendEventToLauncher int event = (int)pCDS->dwData; CopyMemory(sendBuff, &event, sizeof(event)); cbSend = sizeof(event); sendToLauncher();
    switch (pCDS->dwData) {
    case ACCOUNT_REQUEST:
    {
#ifndef STANDALONE
        int requestCode = ACCOUNT_REQUEST;
        CopyMemory(sendBuff, &requestCode, sizeof(requestCode));
        cbSend = sizeof(requestCode);
        std::cout << "Request Account" << std::endl;
        sendToLauncher();
        return waitForRecv(hWnd, message, wParam, lParam, ACCOUNT_REQUEST);
#else
        std::wstring* account = new std::wstring(ACCOUNTNAME);
        COPYDATASTRUCT cds = { 2, account->size() * sizeof(std::wstring::traits_type::char_type), (PVOID)account->data() };
        SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));
        delete account;
        break;
#endif // !STANDALONE
    }
    case TOKEN_REQUEST:
    {
#ifndef STANDALONE
        int requestCode = TOKEN_REQUEST;
        CopyMemory(sendBuff, &requestCode, sizeof(requestCode));
        cbSend = sizeof(requestCode);
        std::cout << "Request Token" << std::endl;
        sendToLauncher();
        return waitForRecv(hWnd, message, wParam, lParam, TOKEN_REQUEST);
#else
        std::string* token = new std::string(TICKET);
        COPYDATASTRUCT cds = { 4, token->size(), (PVOID)token->data() };
        //COPYDATASTRUCT cds = { 4, sizeof AUTH_TICKET, (PVOID)AUTH_TICKET };
        SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));
        delete token;
        break;
#endif // !STANDALONE
    }
    case SERVER_LIST_REQUEST:
    {
#ifndef STANDALONE
        int requestCode = SERVER_LIST_REQUEST;
        CopyMemory(sendBuff, &requestCode, sizeof(requestCode));
        cbSend = sizeof(requestCode);
        std::cout << "Request Server List" << std::endl;
        sendToLauncher(); //send the LauncherListSortCriteria
        return waitForRecv(hWnd, message, wParam, lParam, SERVER_LIST_REQUEST);
#else
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        TeraLauncher::ServerList* serverList = new TeraLauncher::ServerList();
        TeraLauncher::ServerList_Server* serverInfo = new TeraLauncher::ServerList_Server();

        serverInfo->set_id(2800);
        serverInfo->set_name(wstring_to_bytes(L"Valkyon"));
        serverInfo->set_category(wstring_to_bytes(L"PVE"));
        serverInfo->set_title(wstring_to_bytes(L"Valkyon"));
        serverInfo->set_queue(wstring_to_bytes(L"No"));
        serverInfo->set_population(wstring_to_bytes(L"Normal"));
        serverInfo->set_address(ipToInt(IP));
        serverInfo->set_port(PORT);
        serverInfo->set_available(1);
        serverInfo->set_popup(wstring_to_bytes(L"Unable to connect to server."));
        serverList->add_servers()->CopyFrom(*serverInfo);
        serverList->set_lastserverid(0);
        serverList->set_sortcriterion(0);

        int size = serverList->ByteSizeLong();
        char* data = new char[size];
        serverList->SerializeToArray(data, size);
        COPYDATASTRUCT cds{ 6, size, (PVOID)data };
        SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));
        delete serverList;
        delete serverInfo;
        delete[] data;
        std::cout << "ip: " << ipToInt(IP) << std::endl;
        break;
#endif // !STANDALONE
    }

	// message handlers with no breaks so that messages are passed to launcher by default
    case GAME_START:
    {
        std::cout << "Game Start" << std::endl;
        if (injectDLL) {
            injectDLL = false; //only inject once
            std::string dllpath =stdWtoS(DLL_PATH);
            InjectDLL(dllpath);
        }
#ifndef STANDALONE
        DefaultSendEventToLauncher
#endif
        break;
    }
    case GAME_EXIT: {
        // Process quit message (TODO: send message info to launcher)
        LauncherGameExitNotification* message = new LauncherGameExitNotification;
        memcpy_s(message, sizeof LauncherGameExitNotification, pCDS->lpData, pCDS->cbData);
        processExitMessage(message);
        delete[] message;
        DefaultSendEventToLauncher
        PostQuitMessage(0);
        break;
    }
    case GAME_ERROR: {
        std::wstring error((wchar_t*)pCDS->lpData, (int)pCDS->cbData);
        std::wcout << L"Error:" << std::endl << error << std::endl;
#ifndef STANDALONE
        DefaultSendEventToLauncher
#endif
        break;
    }
    default: {
#ifndef STANDALONE
        // send all other messages to launcher
        DefaultSendEventToLauncher
#endif // !STANDALONE
        break;
    }	 
    }
    return 1;
}

void processExitMessage(LauncherGameExitNotification *data)
{
    std::cout << "Game exited with error: ";
    switch (data->code)
    {
    case LAUNCHER_GAME_EXIT_REASON_INVALID_DATA_CENTER:
    {
        std::cout << "Invalid datacenter." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_CONNECTION_DROPPED:
    {
        std::cout << "Connection dropped." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_INVALID_AUTHENTICATION_INFO:
    {
        std::cout << "Invalid authentication info." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_OUT_OF_MEMORY:
    {
        std::cout << "Out of memory." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_SHADER_MODEL_3_UNAVAILABLE:
    {
        std::cout << "Shader model 3 unavailable." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_SPEED_HACK_DETECTED:
    {
        std::cout << "Speed hack detected." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_UNSUPPORTED_VERSION:
    {
        std::cout << "Unsupported version." << std::endl;
        break;
    }
    case LAUNCHER_GAME_EXIT_REASON_ALREADY_ONLINE:
    {
        std::cout << "Account already online." << std::endl;
        break;
    }
    default:
        std::cout << data->code << std::endl;
        break;
    }
}

std::wstring ExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}
void InjectDLL(std::string dllPath) {
	//get handle to process instead of using hTERA
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, TERAPID);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if (loc)
        {
            WriteProcessMemory(hProc, loc, dllPath.c_str(), dllPath.size() + 1, 0);
        }
        //print loc hex
        std::cout << "Dll load address: 0x" << std::setfill('0') << std::setw(4) << std::hex << loc << std::endl;
        std::cout << "LoadLibraryA: 0x" << std::setfill('0') << std::setw(4) << std::hex << LoadLibraryA << std::endl;

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        if (hThread)
        {
            CloseHandle(hThread);
        }
    }
}   

std::string stdWtoS(std::wstring wStr)
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, &str[0], bufferSize, nullptr, nullptr);
    str.resize(bufferSize - 1);
    return str;
}
std::wstring stdStoW(std::string str)
{
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], bufferSize);
    wstr.resize(bufferSize - 1);
    return wstr;
}