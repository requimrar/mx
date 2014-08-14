// TCP.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <HardwareAbstraction/Devices.hpp>
#include <Hashmap.hpp>
#include <StandardIO.hpp>

using namespace Library::StandardIO;
using namespace Library;

// in ms
#define TCP_TIMEOUT		2000

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace TCP
{
	HashMap<SocketFullMappingv4, Socket::SocketObj>* tcpsocketmapv4 = 0;
	HashMap<SocketFullMappingv6, Socket::SocketObj>* tcpsocketmapv6 = 0;

	static LinkedObjList<uint16_t>* usedports = 0;
	static uint16_t nextfree = 49152;

	uint16_t AllocateEphemeralPort()
	{
		if(!usedports)
			usedports = new LinkedObjList<uint16_t>();

		usedports->InsertBack(nextfree);
		return nextfree++;
	}

	void ReleaseEphemeralPort(uint16_t port)
	{
		for(uint64_t i = 0, s = usedports->Size(); i < s; i++)
		{
			if(usedports->Get(i) == port)
				usedports->RemoveAt(i);
		}
	}


	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, IPv4Address source, IPv4Address destip)
	{
		TCPPacket* tcp = (TCPPacket*) packet;

		// validate checksum
		// setup a fake IPv4 header.
		IP::PseudoIPv4Header* pseudo = new IP::PseudoIPv4Header;
		pseudo->source = source;
		pseudo->dest = destip;

		pseudo->zeroes = 0;
		pseudo->protocol = (uint8_t) IP::ProtocolType::TCP;
		pseudo->length = SwapEndian16((uint16_t) length);

		// calculate the pseudo header's checksum separately.
		uint16_t checks[2];
		checks[0] = SwapEndian16(IP::CalculateIPChecksum(pseudo, sizeof(IP::PseudoIPv4Header)));

		uint16_t tcpcheck = tcp->Checksum;
		tcp->Checksum = 0xFFFF;

		// checksum the tcp packet.
		checks[1] = SwapEndian16(IP::CalculateIPChecksum(packet, length));

		uint16_t checksum = ~IP::CalculateIPChecksum(checks, sizeof(checks));

		if(checksum != SwapEndian16(tcpcheck))
		{
			Log(1, "Bad checksum on TCP packet from %d.%d.%d.%d, discarding (got %0.4x, expected %0.4x)", source.b1, source.b2, source.b3, source.b4,
				SwapEndian16(tcpcheck), checksum);
			return;
		}



		uint16_t sourceport = SwapEndian16(tcp->clientport);
		uint16_t destport = SwapEndian16(tcp->serverport);

		bool triedonce = false;

		retry:

		// check for full mapping.
		bool found = false;
		Socket::SocketObj* skt = tcpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { destip, destport }, IPv4PortAddress { source, sourceport } });
		if(skt)
			found = true;

		// multiphase search: check for sourceip + sourceport + destport
		if(!skt && !found)
		{
			skt = tcpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { destip, destport }, IPv4PortAddress { IPv4Address { 0 }, sourceport } });
			if(skt)
				found = true;
		}

		// check for sourceip + destport
		if(!skt && !found)
		{
			skt = tcpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { destip, 0 }, IPv4PortAddress { IPv4Address { 0}, sourceport } });
			if(skt)
				found = true;
		}

		// check for sourceport + destport
		if(!skt && !found)
		{
			skt = tcpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { IPv4Address { 0 }, sourceport }, IPv4PortAddress { IPv4Address { 0 }, sourceport } });
			if(skt)
				found = true;
		}

		// finally, only destport.
		if(!skt && !found)
		{
			skt = tcpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { IPv4Address { 0 }, 0 }, IPv4PortAddress { IPv4Address { 0xFFFFFFFF }, sourceport } });
			if(skt)
				found = true;
		}

		if(found)
		{
			// send into socket buffer.
			skt->tcpconnection->HandleIncoming((uint8_t*) packet, length, (tcp->HeaderLength & 0xF0) >> 4);
			return;
		}

		// if we got here, it didn't find.
		if(!triedonce)
		{
			triedonce = true;
			destip = IPv4Address { 0 };
			goto retry;
		}


		UNUSED(interface);
	}
}
}
}
}












