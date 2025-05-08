#pragma once

#include "resource.h"
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>

enum LauncherGameExitReason
{
    LAUNCHER_GAME_EXIT_REASON_SUCCESS = 0x0,
    LAUNCHER_GAME_EXIT_REASON_INVALID_DATA_CENTER = 0x6,
    LAUNCHER_GAME_EXIT_REASON_CONNECTION_DROPPED = 0x8,
    LAUNCHER_GAME_EXIT_REASON_INVALID_AUTHENTICATION_INFO = 0x9,
    LAUNCHER_GAME_EXIT_REASON_OUT_OF_MEMORY = 0xa,
    LAUNCHER_GAME_EXIT_REASON_SHADER_MODEL_3_UNAVAILABLE = 0xc,
    LAUNCHER_GAME_EXIT_REASON_SPEED_HACK_DETECTED = 0x10,
    LAUNCHER_GAME_EXIT_REASON_UNSUPPORTED_VERSION = 0x13,
    LAUNCHER_GAME_EXIT_REASON_ALREADY_ONLINE = 0x106,
};
struct LauncherGameExitNotification
{
	unsigned int length;
    unsigned int code;
	LauncherGameExitReason reason;
};

void pipe_thread_function(HANDLE hPipe, DWORD Process_Id);
unsigned int ipToInt(std::string ipAddress);
std::string wstring_to_bytes(const std::wstring& str);

void processExitMessage(LauncherGameExitNotification *data);
void InjectDLL(std::string dllPath);

std::string stdWtoS(std::wstring wStr);
std::wstring stdStoW(std::string str);