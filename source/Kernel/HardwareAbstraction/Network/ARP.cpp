// ARP.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons 3.0 Unported.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <StandardIO.hpp>
#include <rdestl/hash_map.h>

using namespace Library::StandardIO;
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace ARP
{
	// ARP
	struct ARPPacket
	{
		uint16_t HardwareType;
		uint16_t ProtocolType;

		uint8_t HardwareAddressLength;
		uint8_t ProtocolAddressLength;

		uint16_t Operation;

		EUI48Address SenderMacAddress;
		Library::IPv4Address SenderIPv4;

		EUI48Address TargetMacAddress;
		Library::IPv4Address TargetIPv4;

		ARPPacket();

	} __attribute__((packed));


	static rde::hash_map<IPv4Address, EUI48Address>* ARPTable = 0;
	static EUI48Address GatewayMAC;

	EUI48Address GetGatewayMAC()
	{
		return GatewayMAC;
	}

	void SetGatewayMAC(EUI48Address addr)
	{
		GatewayMAC = addr;
	}

	void Initialise()
	{
		ARPTable = new rde::hash_map<IPv4Address, EUI48Address>();
	}

	ARPPacket::ARPPacket()
	{
		this->HardwareType			= SwapEndian16(0x0001);	// ethernet
		this->ProtocolType			= SwapEndian16(0x0800);	// ipv4
		this->HardwareAddressLength	= 6;				// 6 byte EUI48 addr
		this->ProtocolAddressLength	= 4;				// 4 byte ipv4 addr
		this->Operation				= SwapEndian16(0x0001);	// request = 1, reply = 2
	}

	static bool IsLocal(IPv4Address addr)
	{
		IPv4Address mask = IP::GetSubnetMask();
		IPv4Address thisip = IP::GetIPv4Address();

		for(int i = 0; i < 4; i++)
		{
			if((thisip.bytes[i] & mask.bytes[i]) != (addr.bytes[i] & mask.bytes[i]))
				return false;
		}

		return true;
	}


	void SendPacket(Devices::NIC::GenericNIC* interface, IPv4Address ip)
	{
		ARPPacket* packet = new ARPPacket;
		packet->SenderIPv4.raw = 0;
		for(int i = 0; i < 6; i++)
			packet->SenderMacAddress.mac[i] = interface->GetMAC()[i];

		for(int i = 0; i < 6; i++)
			packet->TargetMacAddress.mac[i] = 0xFF;

		packet->TargetIPv4.raw = ip.raw;
		Ethernet::SendPacket(interface, packet, sizeof(ARPPacket), Ethernet::EtherType::ARP, packet->TargetMacAddress);
	}

	void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length)
	{
		UNUSED(interface);
		UNUSED(length);

		ARPPacket* arp = (ARPPacket*) packet;
		(*ARPTable)[arp->SenderIPv4] = arp->SenderMacAddress;

		Log("Added translation from IPv4 %d.%d.%d.%d to EUI48 (MAC) %#02x:%#02x:%#02x:%#02x:%#02x:%#02x", arp->SenderIPv4.bytes[0], arp->SenderIPv4.bytes[1], arp->SenderIPv4.bytes[2], arp->SenderIPv4.bytes[3], arp->SenderMacAddress.mac[0], arp->SenderMacAddress.mac[1], arp->SenderMacAddress.mac[2], arp->SenderMacAddress.mac[3], arp->SenderMacAddress.mac[4], arp->SenderMacAddress.mac[5]);
	}


	EUI48Address SendQuery(Devices::NIC::GenericNIC* interface, Library::IPv4Address addr)
	{
		// override behaviour: if we're broadcasting IP, then we should broadcast MAC as well.
		if(addr.raw == 0xFFFFFFFF)
		{
			EUI48Address ret;
			ret.mac[0] = 0xFF;
			ret.mac[1] = 0xFF;
			ret.mac[2] = 0xFF;
			ret.mac[3] = 0xFF;
			ret.mac[4] = 0xFF;
			ret.mac[5] = 0xFF;
			return ret;
		}

		if(!IsLocal(addr))
			return EUI48Address();

		if((*ARPTable)[addr].isZero())
		{
			ARP::SendPacket(interface, addr);

			// 500 ms
			volatile uint64_t timeout = Time::Now() + 500;
			while(timeout > Time::Now())
			{
				EUI48Address ret;
				if(!(ret = (*ARPTable)[addr]).isZero())
					return ret;
			}

			// Log("ARP reply not received within timeout");
			return EUI48Address();
		}
		else
		{
			return (*ARPTable)[addr];
		}
	}
}
}
}
}
