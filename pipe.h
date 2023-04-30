#pragma once
#include "framework.h"
#include <string>
#include <iostream>

#define BUFSIZE 4096

extern DWORD cbRecv, cbSend, cbSent, dwMode;
extern HANDLE hPipeLauncher;
extern char recvBuff[BUFSIZE];
extern char sendBuff[BUFSIZE];

BOOL connectToLauncher(LPCTSTR lpszPipename);
BOOL sendToLauncher();
BOOL waitForRecv(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT request);


enum ClientRequests {
	ACCOUNT = 1,
	ACCOUNT_RESPONSE = 2,
	TOKEN = 3,
	TOKEN_RESPONSE = 4,
	SERVER_LIST = 5,
	SERVER_LIST_RESPONSE = 6,
};