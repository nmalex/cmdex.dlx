#include "stdafx.h"

#include "cmdex.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <xutility>

namespace cmdex 
{
	const wchar_t* run(const wchar_t* cmd) {

		HANDLE hPipeRead, hPipeWrite;

		SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
		saAttr.bInheritHandle = TRUE;   //Pipe handles are inherited by child process.
		saAttr.lpSecurityDescriptor = NULL;

		// Create a pipe to get results from child's stdout.
		if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
			return nullptr;

		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.hStdOutput = hPipeWrite;
		si.hStdError = hPipeWrite;
		si.wShowWindow = SW_HIDE;       // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

		PROCESS_INFORMATION pi = { 0 };

		TCHAR windir[MAX_PATH];
		GetWindowsDirectory(windir, MAX_PATH);
		wchar_t cmdExe[1024];
		swprintf_s(cmdExe, L"%s\\system32\\cmd.exe", windir);

		wchar_t argsStr[8192];
		swprintf_s(argsStr, L"/c %s", cmd);

		BOOL fSuccess = CreateProcessW(cmdExe, argsStr, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
		if (!fSuccess)
		{
			int err = GetLastError();
			CloseHandle(hPipeWrite);
			CloseHandle(hPipeRead);
			return nullptr;
		}

		bool bProcessEnded = false;

		const int outputLen = 5 * 1024 * 1024; // 5Mb output buffer
		char* output = new char[outputLen];
		ZeroMemory(output, outputLen);

		for (; !bProcessEnded;)
		{
			// Give some timeslice (50ms), so we won't waste 100% cpu.
			bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

			// Even if process exited - we continue reading, if there is some data available over pipe.
			for (;;)
			{
				char buf[1024];
				DWORD dwRead = 0;
				DWORD dwAvail = 0;

				if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
					break;

				if (!dwAvail) // no data available, return
					break;

				if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
					// error, the child process might ended
					break;

				buf[dwRead] = 0;
				strcat_s(output, outputLen, buf);
			}
		} //for

		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		auto outputW = new wchar_t[outputLen];
		mbstowcs(outputW, output, outputLen);

		return outputW;
	}
}