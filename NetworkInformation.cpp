#include "pch.h"
#include "NetworkInformation.h"
#include <set>

NetworkInformation::NetworkInformation()
{
}

NetworkInformation::~NetworkInformation()
{
}

BOOL NetworkInformation::GetCurrentIpAddress(std::wstring* strIpAddress)
{
	std::vector<std::wstring> arrDns;
	if (!GetAllDns(&arrDns))
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: GetAllDns() error.\n");
		return FALSE;
	}

	std::set<std::wstring> setIpAddress;

	for (const auto& dns : arrDns)
	{
		std::wstring strIpAddress1;
		if (!GetCurrentIpAddress(dns, 443, &strIpAddress1))
		{
			continue;
		}
		setIpAddress.insert(strIpAddress1);

		std::wstring strIpAddress2;
		if (!GetCurrentIpAddress(dns, 80, &strIpAddress2))
		{
			continue;
		}
		setIpAddress.insert(strIpAddress2);
	}

	if (setIpAddress.size() > 0)
	{
		*strIpAddress = *setIpAddress.begin();
	}

	return TRUE;
}

BOOL NetworkInformation::GetAllDns(std::vector<std::wstring>* pArrOutDns)
{
	ULONG ulBufLen = 0;

	if (pArrOutDns == nullptr)
	{
		wprintf(L"NetworkInformation::GetAllDns: nullptr error.\n");
		return FALSE;
	}

	if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &ulBufLen) != ERROR_BUFFER_OVERFLOW)
	{
		wprintf(L"NetworkInformation::GetAllDns: GetAdaptersAddresses() <1> error.\n");
		return FALSE;
	}

	std::unique_ptr<IP_ADAPTER_ADDRESSES[]> pAddresses = std::make_unique<IP_ADAPTER_ADDRESSES[]>(ulBufLen);
	if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses.get(), &ulBufLen) != NO_ERROR)
	{
		wprintf(L"NetworkInformation::GetAllDns: GetAdaptersAddresses() <2> error.\n");
		return FALSE;
	}

	for (auto pCurrAddresses = pAddresses.get(); pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next)
	{
		if (pCurrAddresses->OperStatus != IfOperStatusUp)
			continue;

		for (auto pDnsServer = pCurrAddresses->FirstDnsServerAddress; pDnsServer != nullptr; pDnsServer = pDnsServer->Next)
		{
			char szIp[INET_ADDRSTRLEN] = { 0, };
			inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(pDnsServer->Address.lpSockaddr)->sin_addr, szIp, INET_ADDRSTRLEN);
			pArrOutDns->push_back(std::wstring(szIp, szIp + strlen(szIp)));
		}
	}

	return TRUE;
}

BOOL NetworkInformation::GetCurrentIpAddress(std::wstring pstrInDnsAddress, USHORT port, std::wstring* pstrOutIpaddress)
{
	if (pstrOutIpaddress == nullptr)
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: nullptr error.\n");
		return FALSE;
	}

	SOCKET sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: socket() error.\n");
		return FALSE;
	}

	SOCKADDR_IN addrServer = { 0, };
	SOCKADDR_IN addrLocal = { 0, };
	int size = sizeof(SOCKADDR_IN);

	addrServer.sin_family = AF_INET;
	std::string strAddress(pstrInDnsAddress.begin(), pstrInDnsAddress.end());
	if (!inet_pton(AF_INET, strAddress.c_str(), &addrServer.sin_addr))
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: inet_pton() error.\n");
		return FALSE;
	}
	addrServer.sin_port = htons(port);

	if (connect(sock, reinterpret_cast<SOCKADDR*>(&addrServer), sizeof(SOCKADDR_IN)) != 0)
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: connect() error.\n");
		return FALSE;
	}

	if (getsockname(sock, reinterpret_cast<SOCKADDR*>(&addrLocal), &size) != 0)
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: getsockname() error.\n");
		return FALSE;
	}

	if (closesocket(sock) != 0)
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: closesocket() error.\n");
		return FALSE;
	}

	char szIpAdress[INET_ADDRSTRLEN] = { 0, };
	if (inet_ntop(AF_INET, &addrLocal.sin_addr, szIpAdress, INET_ADDRSTRLEN) == NULL)
	{
		wprintf(L"NetworkInformation::GetCurrentIpAddress: inet_ntop() error.\n");
		return FALSE;
	}

	std::wstring strIpAdress(szIpAdress, szIpAdress + strlen(szIpAdress));
	*pstrOutIpaddress = strIpAdress;

	return TRUE;
}