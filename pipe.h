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
    ACCOUNT_REQUEST = 1,
    ACCOUNT_RESPONSE = 2,
    TOKEN_REQUEST = 3,
    TOKEN_RESPONSE = 4,
    SERVER_LIST_REQUEST = 5,
    SERVER_LIST_RESPONSE = 6,
    LAUNCHER_GAME_EVENT_ENTERED_INTO_CINEMATIC = 1001,
    LAUNCHER_GAME_EVENT_ENTERED_SERVER_LIST = 1002,
    LAUNCHER_GAME_EVENT_ENTERING_LOBBY = 1003,
    LAUNCHER_GAME_EVENT_ENTERED_LOBBY = 1004,
    LAUNCHER_GAME_EVENT_ENTERING_CHARACTER_CREATION = 1005,
    LAUNCHER_GAME_EVENT_LEFT_LOBBY = 1006,
    LAUNCHER_GAME_EVENT_DELETED_CHARACTER = 1007,
    LAUNCHER_GAME_EVENT_CANCELED_CHARACTER_CREATION = 1008,
    LAUNCHER_GAME_EVENT_ENTERED_CHARACTER_CREATION = 1009,
    LAUNCHER_GAME_EVENT_CREATED_CHARACTER = 1010,
    LAUNCHER_GAME_EVENT_ENTERED_WORLD = 1011,
    LAUNCHER_GAME_EVENT_FINISHED_LOADING_SCREEN = 1012,
    LAUNCHER_GAME_EVENT_LEFT_WORLD = 1013,
    LAUNCHER_GAME_EVENT_MOUNTED_PEGASUS = 1014,
    LAUNCHER_GAME_EVENT_DISMOUNTED_PEGASUS = 1015,
    LAUNCHER_GAME_EVENT_CHANGED_CHANNEL = 1016,
    GAME_EXIT = 0x3fc,
    GAME_ERROR = 1021,
    GAME_START = 1000
};