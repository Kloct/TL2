#include "pipe.h"
#include <stdio.h>
#include <iostream>

#define SendToClient(msgCode) char* buffer = new char[cbRecv];\
		CopyMemory(buffer, recvBuff, cbRecv);\
		COPYDATASTRUCT cds{ msgCode, cbRecv, buffer };\
		SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(&cds));\
		delete[] buffer;\
		return 1;\

DWORD cbRecv, cbSend, cbSent, dwMode;
HANDLE hPipeLauncher;
char recvBuff[BUFSIZE];
char sendBuff[BUFSIZE];

BOOL connectToLauncher(LPCTSTR lpszPipename) {
	hPipeLauncher = CreateFile(
		lpszPipename,   // pipe name
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hPipeLauncher == INVALID_HANDLE_VALUE)
		std::cout << "connectToLauncher failed with error: " << GetLastError() << std::endl;
	// pipe connected; change to message-read mode.
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(
		hPipeLauncher,		// pipe handle 
		NULL,				// new pipe mode 
		NULL,				// don't set maximum bytes 
		NULL);				// don't set maximum time
	if (!fSuccess)
		std::cout << "SetNamedPipeHandleState failed with error: " << GetLastError() << std::endl;
	return fSuccess;
}
// Send a message to the pipe server. 
BOOL sendToLauncher() {
	BOOL fSuccess = FALSE;
	std::cout << "Sending message to pipe" << std::endl;
	fSuccess = WriteFile(
		hPipeLauncher,          // pipe handle 
		sendBuff,				// message 
		cbSend,					// message length 
		&cbSent,				// bytes written 
		NULL);                  // not overlapped 
	if (!fSuccess)
		std::cout << "WriteFile to pipe failed with error: " << GetLastError() << std::endl;
	std::cout << "cbSent: " << cbSent << std::endl;
	return fSuccess;
}

std::string wstring_to_bytes(const std::wstring& str);
unsigned int ipToInt(std::string ipAddress);

BOOL waitForRecv(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT request) {
	// Read file will block until data is available
	BOOL fSuccess = FALSE;

	cbRecv = 0;
	fSuccess = ReadFile(
		hPipeLauncher,			// pipe handle 
		recvBuff,				// buffer to receive reply 
		BUFSIZE * sizeof(char), // size of buffer 
		&cbRecv,				// number of bytes read 
		NULL					// not overlapped 
	);
	if (!fSuccess) {
		std::cout << "ReadFile from pipe failed with error: " << GetLastError() << std::endl;
		return 0;
	}
	std::cout << "cbRecv: " << cbRecv << std::endl;
	if (request == TOKEN_REQUEST) {
		SendToClient(TOKEN_RESPONSE);
	}
	if (request == ACCOUNT_REQUEST) {
		SendToClient(ACCOUNT_RESPONSE);
	}
	if (request == SERVER_LIST_REQUEST) {
		SendToClient(SERVER_LIST_RESPONSE);
	}
}