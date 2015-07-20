// Virtual.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <Synchro.hpp>
#include <rdestl/rdestl.h>

namespace Kernel {

class Mutex;

namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	struct PageMapStructure
	{
		uint64_t Entry[512];
	};

	struct AddressLengthPair
	{
		AddressLengthPair() : start(0), length(0) { }
		AddressLengthPair(uint64_t s, uint64_t l) : start(s), length(l) { }

		bool operator==(const AddressLengthPair& rhs)
		{
			return this->start == rhs.start && this->length == rhs.length;
		}

		uint64_t start;
		uint64_t length;
	};

	struct ALPTuple
	{
		ALPTuple() : start(0), length(0), phys(0) { }
		ALPTuple(uint64_t s, uint64_t l, uint64_t p) : start(s), length(l), phys(p) { }

		bool operator==(const ALPTuple& rhs)
		{
			return this->start == rhs.start && this->length == rhs.length && this->phys == rhs.phys;
		}

		uint64_t start;
		uint64_t length;
		uint64_t phys;
	};

	struct VirtualAddressSpace
	{
		VirtualAddressSpace(PageMapStructure* pml4) : used(64)
		{
			this->PML4 = pml4;
		}

		// book-keeping for allocations.
		rde::vector<AddressLengthPair> pairs;
		rde::vector<ALPTuple> used;

		// store the actual address of the pml4.
		PageMapStructure* PML4;
		Mutex* mtx;
	};


	#define I_Present		0x01
	#define I_ReadWrite		0x02
	#define I_UserAccess	0x04
	#define I_AlignMask		0xFFFFFFFFFFFFF000
	#define I_NoExecute		0
	#define I_CopyOnWrite	0x800	// bit 11
	#define I_SwappedPage	0x400	// bit 10



	uint64_t GetRawCR3();
	void ChangeRawCR3(uint64_t newval);
	void invlpg(PageMapStructure* p);
	void Initialise();
	void SwitchPML4T(PageMapStructure* PML4T);
	void ChangeAddressSpace(PageMapStructure* pms);
	PageMapStructure* GetCurrentPML4T();

	VirtualAddressSpace* SetupVAS(VirtualAddressSpace* vas);
	void DestroyVAS(VirtualAddressSpace* vas);
	uint64_t AllocateVirtual(uint64_t size = 1, uint64_t addr = 0, VirtualAddressSpace* vas = 0, uint64_t phys = 0);
	void FreeVirtual(uint64_t addr, uint64_t size = 1, VirtualAddressSpace* vas = 0);
	uint64_t GetVirtualPhysical(uint64_t virt, VirtualAddressSpace* vas = 0);
	void ForceInsertALPTuple(uint64_t addr, size_t sizeInPages, uint64_t phys, VirtualAddressSpace* vas = 0);

	uint64_t AllocatePage(uint64_t size = 1, uint64_t addr = 0, uint64_t flags = 0x7);
	void FreePage(uint64_t addr, uint64_t size = 1);


	void MarkCOW(uint64_t VirtAddr, VirtualAddressSpace* vas = 0);
	void UnmarkCOW(uint64_t VirtAddr, VirtualAddressSpace* vas = 0);

	void MarkCOW(uint64_t VirtAddr, PageMapStructure* pml);
	void UnmarkCOW(uint64_t VirtAddr, PageMapStructure* pml);

	VirtualAddressSpace* CopyVAS(VirtualAddressSpace* src, VirtualAddressSpace* dest);
	bool HandlePageFault(uint64_t cr2, uint64_t cr3, uint64_t errorcode);

	void CopyBetweenAddressSpaces(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from, VirtualAddressSpace* to);
	void CopyFromKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* to);
	void CopyToKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from);



	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4, bool DoNotUnmap);
	void UnmapAddress(uint64_t VirtAddr, PageMapStructure* PML4, bool DoNotUnmap);

	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags);
	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4);
	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, bool DoNotUnmap);
	void UnmapAddress(uint64_t VirtAddr);
	void UnmapAddress(uint64_t VirtAddr, PageMapStructure* PML4);
	void UnmapAddress(uint64_t VirtAddr, bool DoNotUnmap);



	void MapRegion(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t LengthInPages, uint64_t Flags, PageMapStructure* PML4 = 0);
	void UnmapRegion(uint64_t VirtAddr, uint64_t LengthInPages, PageMapStructure* PML4 = 0);

	uint64_t CreateVAS();
	uint64_t GetMapping(uint64_t VirtAddr, PageMapStructure* VAS);
}
}
}
}
