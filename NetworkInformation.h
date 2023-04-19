#pragma once

class NetworkInformation
{
public:
	NetworkInformation();
	~NetworkInformation();
	BOOL GetCurrentIpAddress(std::wstring* strIpAddress);

private:
	BOOL GetAllDns(std::vector<std::wstring>* pArrOutDns);
	BOOL GetCurrentIpAddress(std::wstring pstrInDnsAddress, USHORT port, std::wstring* pstrOutIpaddress);
};