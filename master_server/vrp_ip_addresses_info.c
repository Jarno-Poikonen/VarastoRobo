/*
	VarastoRobo master server version 1.0.0 2019-12-10 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#include "vrp_ip_addresses_info.h"
#include <stdio.h>

DWORD vrp_get_host_ip_address(uint32_t* host_ip_address, uint32_t* subnet_mask)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
	{
		*host_ip_address = INADDR_ANY;
		return ERROR_OUTOFMEMORY;
	}
	size_t adapter_info_size = 4096;
	IP_ADAPTER_ADDRESSES* adapter_info_base = (IP_ADAPTER_ADDRESSES*)HeapAlloc(heap, 0, adapter_info_size);
	DWORD error = ERROR_BUFFER_OVERFLOW;
	while (error == ERROR_BUFFER_OVERFLOW)
	{
		ULONG get_adapter_info_size = (ULONG)adapter_info_size;
		error = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_GATEWAYS, 0, adapter_info_base, &get_adapter_info_size);
		if (error == ERROR_BUFFER_OVERFLOW)
		{
			adapter_info_size = (size_t)get_adapter_info_size;
			IP_ADAPTER_ADDRESSES* new_adapter_info = (IP_ADAPTER_ADDRESSES*)HeapReAlloc(heap, 0, adapter_info_base, adapter_info_size);
			if (!new_adapter_info)
			{
				HeapFree(heap, 0, adapter_info_base);
				*host_ip_address = INADDR_ANY;
				return ERROR_OUTOFMEMORY;
			}
			adapter_info_base = new_adapter_info;
		}
		else if (error)
		{
			HeapFree(heap, 0, adapter_info_base);
			*host_ip_address = INADDR_ANY;
			return error;
		}
	}

	/*
	CHAR address_string[46];
	IP_ADAPTER_ADDRESSES* pCurrAddresses = adapter_info_base;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
	PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
	IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
	IP_ADAPTER_GATEWAY_ADDRESS_LH* pGateway = NULL;
	IP_ADAPTER_PREFIX *pPrefix = NULL;
	int i = 0;
	while (pCurrAddresses) {
		printf("\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n",
			pCurrAddresses->Length);
		printf("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
		printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);

		pUnicast = pCurrAddresses->FirstUnicastAddress;
		if (pUnicast != NULL) {
			for (i = 0; pUnicast != NULL; i++)
				pUnicast = pUnicast->Next;
			printf("\tNumber of Unicast Addresses: %d\n", i);
			pUnicast = pCurrAddresses->FirstUnicastAddress;
			for (i = 0; pUnicast != NULL; i++)
			{
				struct in_addr test;
				if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
					RtlIpv4AddressToStringA(&((struct sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr, address_string);
				else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
					RtlIpv6AddressToStringA(&((struct sockaddr_in6*)pUnicast->Address.lpSockaddr)->sin6_addr, address_string);
				else
					memcpy(address_string, "?", 2);
				printf("\t\t%s\n", address_string);
				pUnicast = pUnicast->Next;
			}
		}
		else
			printf("\tNo Unicast Addresses\n");

		pAnycast = pCurrAddresses->FirstAnycastAddress;
		if (pAnycast) {
			for (i = 0; pAnycast != NULL; i++)
				pAnycast = pAnycast->Next;
			printf("\tNumber of Anycast Addresses: %d\n", i);
			pAnycast = pCurrAddresses->FirstAnycastAddress;
			for (i = 0; pAnycast != NULL; i++)
			{
				if (pAnycast->Address.lpSockaddr->sa_family == AF_INET)
					RtlIpv4AddressToStringA(&((struct sockaddr_in*)pAnycast->Address.lpSockaddr)->sin_addr, address_string);
				else if (pAnycast->Address.lpSockaddr->sa_family == AF_INET6)
					RtlIpv6AddressToStringA(&((struct sockaddr_in6*)pAnycast->Address.lpSockaddr)->sin6_addr, address_string);
				else
					memcpy(address_string, "?", 2);
				printf("\t\t%s\n", address_string);
				pAnycast = pAnycast->Next;
			}
		}
		else
			printf("\tNo Anycast Addresses\n");

		pMulticast = pCurrAddresses->FirstMulticastAddress;
		if (pMulticast) {
			for (i = 0; pMulticast != NULL; i++)
				pMulticast = pMulticast->Next;
			printf("\tNumber of Multicast Addresses: %d\n", i);
			pMulticast = pCurrAddresses->FirstMulticastAddress;
			for (i = 0; pMulticast != NULL; i++)
			{
				if (pMulticast->Address.lpSockaddr->sa_family == AF_INET)
					RtlIpv4AddressToStringA(&((struct sockaddr_in*)pMulticast->Address.lpSockaddr)->sin_addr, address_string);
				else if (pMulticast->Address.lpSockaddr->sa_family == AF_INET6)
					RtlIpv6AddressToStringA(&((struct sockaddr_in6*)pMulticast->Address.lpSockaddr)->sin6_addr, address_string);
				else
					memcpy(address_string, "?", 2);
				printf("\t\t%s\n", address_string);
				pMulticast = pMulticast->Next;
			}
		}
		else
			printf("\tNo Multicast Addresses\n");

		pDnServer = pCurrAddresses->FirstDnsServerAddress;
		if (pDnServer) {
			for (i = 0; pDnServer != NULL; i++)
				pDnServer = pDnServer->Next;
			printf("\tNumber of DNS Server Addresses: %d\n", i);
			pDnServer = pCurrAddresses->FirstDnsServerAddress;
			for (i = 0; pDnServer != NULL; i++)
			{
				if (pDnServer->Address.lpSockaddr->sa_family == AF_INET)
					RtlIpv4AddressToStringA(&((struct sockaddr_in*)pDnServer->Address.lpSockaddr)->sin_addr, address_string);
				else if (pDnServer->Address.lpSockaddr->sa_family == AF_INET6)
					RtlIpv6AddressToStringA(&((struct sockaddr_in6*)pDnServer->Address.lpSockaddr)->sin6_addr, address_string);
				else
					memcpy(address_string, "?", 2);
				printf("\t\t%s\n", address_string);
				pDnServer = pDnServer->Next;
			}
		}
		else
			printf("\tNo DNS Server Addresses\n");

		pGateway = pCurrAddresses->FirstGatewayAddress;

		if (pGateway) {
			for (i = 0; pGateway != NULL; i++)
				pGateway = pGateway->Next;
			printf("\tNumber of Gateway Addresses: %d\n", i);
			pGateway = pCurrAddresses->FirstGatewayAddress;
			for (i = 0; pGateway != NULL; i++)
			{
				if (pGateway->Address.lpSockaddr->sa_family == AF_INET)
					RtlIpv4AddressToStringA(&((struct sockaddr_in*)pGateway->Address.lpSockaddr)->sin_addr, address_string);
				else if (pGateway->Address.lpSockaddr->sa_family == AF_INET6)
					RtlIpv6AddressToStringA(&((struct sockaddr_in6*)pGateway->Address.lpSockaddr)->sin6_addr, address_string);
				else
					memcpy(address_string, "?", 2);
				printf("\t\t%s\n", address_string);
				pGateway = pGateway->Next;
			}
		}
		else
			printf("\tNo Gateway Addresses\n");

		printf("\tDNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);
		printf("\tDescription: %wS\n", pCurrAddresses->Description);
		printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);

		if (pCurrAddresses->PhysicalAddressLength != 0) {
			printf("\tPhysical address: ");
			for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;
				i++) {
				if (i == (pCurrAddresses->PhysicalAddressLength - 1))
					printf("%.2X\n",
					(int)pCurrAddresses->PhysicalAddress[i]);
				else
					printf("%.2X-",
					(int)pCurrAddresses->PhysicalAddress[i]);
			}
		}
		printf("\tFlags: %ld\n", pCurrAddresses->Flags);
		printf("\tMtu: %lu\n", pCurrAddresses->Mtu);
		printf("\tIfType: %ld\n", pCurrAddresses->IfType);
		printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
		printf("\tIpv6IfIndex (IPv6 interface): %u\n",
			pCurrAddresses->Ipv6IfIndex);
		printf("\tZoneIndices (hex): ");
		for (i = 0; i < 16; i++)
			printf("%lx ", pCurrAddresses->ZoneIndices[i]);
		printf("\n");

		printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);
		printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);

		pPrefix = pCurrAddresses->FirstPrefix;
		if (pPrefix) {
			for (i = 0; pPrefix != NULL; i++)
				pPrefix = pPrefix->Next;
			printf("\tNumber of IP Adapter Prefix entries: %d\n", i);
		}
		else
			printf("\tNumber of IP Adapter Prefix entries: 0\n");

		printf("\n");

		pCurrAddresses = pCurrAddresses->Next;
	}
	*/

	int host_address_likelynes = 1;
	uint32_t likely_host_address = INADDR_ANY;
	uint32_t likely_subnet_mask = 0xFFFFFFFF;
	for (IP_ADAPTER_ADDRESSES* adapter_info = adapter_info_base; adapter_info; adapter_info = adapter_info->Next)
	{
		IP_ADAPTER_UNICAST_ADDRESS* unicast_address = adapter_info->FirstUnicastAddress;
		IP_ADAPTER_MULTICAST_ADDRESS* multicast_address = adapter_info->FirstMulticastAddress;
		IP_ADAPTER_GATEWAY_ADDRESS_LH* gateway_address = adapter_info->FirstGatewayAddress;
		IP_ADAPTER_DNS_SERVER_ADDRESS* dns_address = adapter_info->FirstDnsServerAddress;
		int likelynes = 0;
		while (unicast_address && unicast_address->Address.lpSockaddr->sa_family != AF_INET)
			unicast_address = unicast_address->Next;
		while (multicast_address && multicast_address->Address.lpSockaddr->sa_family != AF_INET)
			multicast_address = multicast_address->Next;
		while (gateway_address && gateway_address->Address.lpSockaddr->sa_family != AF_INET)
			gateway_address = gateway_address->Next;
		while (dns_address && dns_address->Address.lpSockaddr->sa_family != AF_INET)
			dns_address = dns_address->Next;
		if (adapter_info->DnsSuffix && (lstrlenW(adapter_info->DnsSuffix) > 0) && dns_address && gateway_address && multicast_address && unicast_address)
			likelynes = 6;
		else if (dns_address && gateway_address && multicast_address && unicast_address)
			likelynes = 5;
		else if (gateway_address && multicast_address && unicast_address)
			likelynes = 4;
		else if (multicast_address && unicast_address)
			likelynes = 3;
		else if (unicast_address)
			likelynes = 2;
		if (likelynes > host_address_likelynes)
		{
			likely_host_address = ((struct sockaddr_in*)unicast_address->Address.lpSockaddr)->sin_addr.s_addr;
			ConvertLengthToIpv4Mask(unicast_address->OnLinkPrefixLength, &likely_subnet_mask);
			host_address_likelynes = likelynes;
		}
	}
	HeapFree(heap, 0, adapter_info_base);
	*host_ip_address = likely_host_address;
	*subnet_mask = likely_subnet_mask;
	return 0;
}
