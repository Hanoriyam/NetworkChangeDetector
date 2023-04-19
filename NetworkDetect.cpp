#include "pch.h"
#include "NetworkInformation.h"

BOOL InitWinsock()
{
	WSADATA wsaData = { 0, };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return FALSE;
	}

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return FALSE;
	}

	return TRUE;
}

BOOL DetectNetworkChange()
{
	OVERLAPPED overlap = { 0, };
	overlap.hEvent = WSACreateEvent();
	if (overlap.hEvent == WSA_INVALID_EVENT)
	{
		wprintf(L"DetectNetworkChange: WSACreateEvent() error.\n");
		return FALSE;
	}

	DWORD dwRet = ::NotifyAddrChange(NULL, &overlap);
	if (dwRet != NO_ERROR && dwRet != ERROR_IO_PENDING)
	{
		wprintf(L"DetectNetworkChange: NotifyAddrChange() error\n");
		return FALSE;
	}

	switch (WaitForSingleObject(overlap.hEvent, INFINITE))
	{
		case WAIT_FAILED:
		{
			wprintf(L"DetectNetworkChange: WAIT_FAILED() error. (%d)\n", GetLastError());
			return FALSE;
		}

		case WAIT_OBJECT_0:
		{
			break;
		}

		default:
		{
			return FALSE;
		}
	}
	return TRUE;
}

int wmain()
{
	if (!InitWinsock())
	{
		wprintf(L"wmain: InitWinsock fail.\n");
		return 0;
	}

	NetworkInformation network;
	std::wstring strIpAddress;
	if (!network.GetCurrentIpAddress(&strIpAddress))
	{
		wprintf(L"wmain: GetCurrentIpAddress() fail.\n");
		return 0;
	}
	wprintf(L"wmain: Current IP: %s\n", strIpAddress.c_str());

	while (true)
	{
		if (DetectNetworkChange())
		{
			wprintf(L"wmain: NetworkChangeEvent raised.\n");

			NetworkInformation network;
			std::wstring strIpAddress;
			if (!network.GetCurrentIpAddress(&strIpAddress))
			{
				wprintf(L"wmain: GetCurrentIpAddress() fail.\n");
				continue;
			}
			wprintf(L"DetectNetworkChange: Current IP: %s\n", strIpAddress.c_str());
		}
	}

	return 0;
}