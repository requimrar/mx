// Functions.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../syscall.h"

namespace Library {
namespace SystemCall
{
	/*
		// misc things, page 0+
		.quad	ExitProc			// 0000
		.quad	InstallIRQ			// 0001
		.quad	InstallIRQNoRegs		// 0002

		// process related things, page 4000+
		.quad	CreateThread			// 4000
		.quad	SpawnProcess			// 4001
		.quad	SendSignalToProcess		// 4002
		.quad	SendSignalToThread		// 4003
		.quad	SendMessage			// 4004
		.quad	ReceiveMessage		// 4005
		.quad	Sleep				// 4006
		.quad	Yield				// 4007
		.quad	Block				// 4008
		.quad	InstallSigHandler		// 4009
		.quad	GetPID				// 4010
		.quad	GetParentPID			// 4011

		// file io things, page 8000+
		.quad	OpenFile			// 8000
		.quad	OpenIPCSocket		// 8001
		.quad	OpenAnyFD			// 8002
		.quad	CloseAnyFD			// 8003
		.quad	ReadAnyFD			// 8004
		.quad	WriteAnyFD			// 8005
		.quad	MemoryMapAnonymous	// 8006
		.quad	MemoryMapFile		// 8007
	*/


	void ExitProc()
	{
		Syscall0Param(0);
	}

	void InstallIRQHandler(uint64_t irq, uint64_t handleraddr)
	{
		Syscall2Param(1, irq, handleraddr);
	}

	void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr)
	{
		Syscall2Param(2, irq, handleraddr);
	}



	void CreateThread(void (*thr)())
	{
		Syscall1Param(4000, (uint64_t) thr);
	}

	void SpawnProcess(const char* path, const char* name)
	{
		Syscall2Param(4001, (uint64_t) path, (uint64_t) name);
	}

	void SignalProcess(pid_t pid, int signum)
	{
		Syscall2Param(4002, pid, signum);
	}

	void SignalThread(pid_t tid, int signum)
	{
		Syscall2Param(4003, tid, signum);
	}

	int SendMessage(key_t key, void* msg, size_t size, uint64_t flags)
	{
		return (int) Syscall4Param(4004, key, (uint64_t) msg, size, flags);
	}

	ssize_t ReceiveMessage(key_t key, void* msg, size_t size, uint64_t type, uint64_t flags)
	{
		return (int) Syscall5Param(4005, key, (uint64_t) msg, size, type, flags);
	}

	void Sleep(uint64_t ms)
	{
		Syscall1Param(4006, ms);
	}

	void Yield()
	{
		Syscall0Param(4007);
	}

	void Block()
	{
		Syscall0Param(4008);
	}

	sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler)
	{
		return (sighandler_t) Syscall2Param(4009, signum, (uint64_t) handler);
	}

	uint64_t GetPID()
	{
		return Syscall0Param(4010);
	}

	uint64_t GetParentPID()
	{
		return Syscall0Param(4011);
	}

	// 8000, 8001

	uint64_t Open(const char* path, uint64_t flags)
	{
		return Syscall2Param(8002, (uint64_t) path, flags);
	}

	void Close(uint64_t fd)
	{
		Syscall1Param(8003, fd);
	}

	uint64_t Read(uint64_t sd, void* buffer, uint64_t length)
	{
		return Syscall3Param(8004, sd, (uint64_t) buffer, length);
	}

	uint64_t Write(uint64_t sd, const void* buffer, uint64_t length)
	{
		return Syscall3Param(8005, sd, (uint64_t) buffer, length);
	}

	uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
	{
		return Syscall4Param(8006, addr, size, prot, flags);
	}
}
}
