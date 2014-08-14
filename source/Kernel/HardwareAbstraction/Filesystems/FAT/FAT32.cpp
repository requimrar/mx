
// FAT32.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <String.hpp>
#include <List.hpp>
#include <math.h>

using namespace Library;
using namespace Library::StandardIO;
using namespace Kernel::HardwareAbstraction::Devices::Storage;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	struct DirectoryEntry
	{
		char name[8];
		char ext[3];
		uint8_t attrib;
		uint8_t userattrib;

		char undelete;
		uint16_t createtime;
		uint16_t createdate;
		uint16_t accessdate;
		uint16_t clusterhigh;

		uint16_t modifiedtime;
		uint16_t modifieddate;
		uint16_t clusterlow;
		uint32_t filesize;
	} __attribute__ ((packed));


	FAT32::FAT32(Partition* ParentP) : FSDriver(FSTypes::fat32)
	{
		using namespace Devices::Storage::ATA::PIO;
		using Devices::Storage::ATADrive;

		this->ParentPartition = ParentP;

		this->Type = FSTypes::fat32;

		// read the fields from LBA 0
		StorageDevice* atadev = ParentP->GetStorageDevice();

		uint64_t buf = MemoryManager::Physical::AllocateDMA(1);
		// atadev->Read(ParentP->GetStartLBA(), buf, 512);
		IO::Read(atadev, ParentP->GetStartLBA(), buf, 512);


		uint8_t* fat = (uint8_t*) buf;

		this->BytesPerSector		= *((uint16_t*)((uintptr_t) fat + 11));

		this->SectorsPerCluster		= *((uint8_t*)((uintptr_t) fat + 13));
		this->ReservedSectors		= *((uint16_t*)((uintptr_t) fat + 14));
		this->NumberOfFATs		= *((uint8_t*)((uintptr_t) fat + 16));
		this->NumberOfDirectories	= *((uint16_t*)((uintptr_t) fat + 17));


		if((uint16_t)(*((uint16_t*)(fat + 17))) > 0)
		{
			this->TotalSectors	= *((uint16_t*)((uintptr_t) fat + 17));
		}
		else
		{
			this->TotalSectors	= *((uint32_t*)((uintptr_t) fat + 32));
		}

		this->HiddenSectors		= *((uint32_t*)((uintptr_t) fat + 28));

		this->FATSectorSize		= *((uint32_t*)((uintptr_t) fat + 36));
		this->RootDirectoryCluster	= *((uint32_t*)((uintptr_t) fat + 44));
		this->FSInfoCluster		= *((uint16_t*)((uintptr_t) fat + 48));

		this->BackupBootCluster	= *((uint16_t*)((uintptr_t) fat + 50));
		this->FirstUsableCluster	= this->ParentPartition->GetStartLBA() + this->ReservedSectors + (this->NumberOfFATs * this->FATSectorSize);


		char* name = new char[256];
		name[0] = (char) fat[71];
		name[1] = (char) fat[72];
		name[2] = (char) fat[73];
		name[3] = (char) fat[74];
		name[4] = (char) fat[75];
		name[5] = (char) fat[76];
		name[6] = (char) fat[77];
		name[7] = (char) fat[78];
		name[8] = (char) fat[79];
		name[9] = (char) fat[80];
		name[10] = (char) fat[81];

		name = String::TrimWhitespace(name);
		this->rootfs = new VFS::Filesystem(name, this);
		MemoryManager::Physical::FreeDMA(buf, 1);
	}

	void FAT32::PrintInfo()
	{

	}

	VFS::Folder* FAT32::GetRootFolder()
	{
		return new VFS::Folder("/", this->RootDirectoryCluster, this->rootfs, 0);
	}





	Library::LinkedList<VFS::FSObject>* FAT32::GetFSObjects(VFS::Folder* start)
	{
		using Library::string;
		using Library::LinkedList;


		string* filename = new string();
		string* currentpath = new string();

		currentpath->Append(start->Path());
		LinkedList<string>* LFNEntries = new LinkedList<string>();
		LinkedList<VFS::FSObject>* ret = new LinkedList<VFS::FSObject>();


		uint64_t cluster = start->Cluster();
		uint64_t nc = cluster;

		bool DidParseLFN = false;
		bool IsLFN = false;

		uint64_t buf = MemoryManager::Physical::AllocateDMA((512 * this->SectorsPerCluster + 0xFFF) / 0x1000);

		do
		{
			cluster = nc;

			// get the next cluster.
			// this->ParentPartition->GetStorageDevice()->Read(this->ParentPartition->GetStartLBA() + this->ReservedSectors + (cluster * 4 / 512), buf, 512);

			IO::Read(this->ParentPartition->GetStorageDevice(), this->ParentPartition->GetStartLBA() + this->ReservedSectors + (cluster * 4 / 512), buf, 512);

			uint8_t* fat2 = (uint8_t*) buf;

			uint32_t offset = ((cluster * 4) % 512);
			nc = *((uint32_t*)(fat2 + offset));


			// read the contents of this cluster back into the buffer.
			// this->ParentPartition->GetStorageDevice()->Read(this->FirstUsableCluster + cluster * this->SectorsPerCluster - (this->RootDirectoryCluster * this->SectorsPerCluster), buf, 512 * this->SectorsPerCluster);

			IO::Read(this->ParentPartition->GetStorageDevice(), this->FirstUsableCluster + cluster * this->SectorsPerCluster - (this->RootDirectoryCluster * this->SectorsPerCluster), buf, 512 * this->SectorsPerCluster);

			for(uint64_t it = 0; it < (512 * this->SectorsPerCluster) / 32; it++)
			{
				// get a pointer.
				uint8_t* data = (uint8_t*)(buf + (it * 32));

				if(*data == '.' && *(data + 1) == ' ')
				{
					LFNEntries->Clear();
					continue;
				}

				else if(*data == '.' && *(data + 1) == '.')
				{
					LFNEntries->Clear();
					continue;
				}

				// make sure it's a valid entry
				if(*data != 0xE5 && *data != 0x05 && *data != 0x00 && !(*(data + 11) & (1 << 3)) && (*(data + 11) != 0x0F))
				{
					if(!DidParseLFN)
					{
						bool islowername = false;
						bool islowerextn = false;

						if(*(data + 0xC) & (1 << 3))
							islowername = true;

						if(*(data + 0xC) & (1 << 4))
							islowerextn = true;


						for(uint8_t t = 0; t < 8; t++)
						{
							uint8_t tc = *(data + t);
							filename->Append((char) tc + ((islowername && tc >= 'A' && tc <= 'Z') ? 32 : 0));
						}

						for(uint8_t t = 8; t < 11; t++)
						{
							uint8_t tc = *(data + t);
							filename->Append((char) tc + ((islowerextn && tc >= 'A' && tc <= 'Z') ? 32 : 0));
						}
					}
					else
					{
						DidParseLFN = false;
						IsLFN = true;
						uint64_t os = LFNEntries->Size();
						for(uint64_t d = 0; d < os; d++)
						{
							auto a = LFNEntries->RemoveFront();
							filename->Append(a);
						}
					}

					if(!IsLFN)
					{
						char* f = (*(data + 11) & (1 << 4)) ? GetFolderName(filename->CString()) : GetFileName(filename->CString());
						delete filename;

						filename = new string(f);
						delete[] f;
					}

					uint8_t attr = 0;
					if(*(data + 11) & 0x1)
						attr |= VFS::Attr_ReadOnly;

					if(*(data + 11) & 0x2)
						attr |= VFS::Attr_Hidden;


					if(*(data + 11) & (1 << 4))
					{
						string* tfn = new string();
						tfn->Append(currentpath);
						tfn->Append('/');
						tfn->Append(filename->CString());

						VFS::Folder* thef = new VFS::Folder(tfn->CString(), ((uint32_t)*((uint16_t*)(data + 0x14)) << 16) | (uint32_t)*((uint16_t*)(data + 0x1A)), this->rootfs, attr);

						ret->InsertBack(thef);
						delete tfn;
					}
					else
					{
						string* tfn = new string();
						tfn->Append(currentpath);
						tfn->Append('/');
						tfn->Append(filename->CString());

						ret->InsertBack(new VFS::File(tfn->CString(), *((uint32_t*)(data + 0x1C)), ((uint32_t)*((uint16_t*)(data + 0x14)) << 16) | (uint32_t)*((uint16_t*)(data + 0x1A)), this->rootfs, attr));

						delete tfn;
					}
				}

				else if(*data != 0xE5 && *data != 0x2E && *data != 0x05 && *data != 0x00 && *(data + 11) == 0x0F)
				{
					// handle a LFN entry.
					DidParseLFN = true;
					string* thisentry = new string();

					// get the first 5 characters.
					for(int d = 0; d < 5; d++)
					{
						uint8_t c = (uint8_t)(*((uint16_t*)(data + 1 + (d * 2))));
						if(c != 0xFF){ thisentry->Append((char) c); }
					}

					for(int d = 0; d < 6; d++)
					{
						uint8_t c = (uint8_t)(*((uint16_t*)(data + 14 + (d * 2))));
						if(c != 0xFF){ thisentry->Append((char) c); }
					}

					for(int d = 0; d < 2; d++)
					{
						uint8_t c = (uint8_t)(*((uint16_t*)(data + 28 + (d * 2))));
						if(c != 0xFF){ thisentry->Append((char) c); }
					}

					LFNEntries->InsertFront(thisentry);
				}

				IsLFN = false;
				filename->Clear();
			}

		} while((nc != 0) && ((nc & 0x0FFFFFFF) < 0x0FFFFFF8));

		delete LFNEntries;
		delete filename;

		MemoryManager::Physical::FreeDMA(buf, (this->SectorsPerCluster * 512 + 0xFFF) / 0x1000);
		return ret;
	}






	void FAT32::ReadFile(VFS::File* File, uint64_t Address, uint64_t length)
	{
		uint64_t BufferOffset = 0;
		using namespace HardwareAbstraction::Devices::Storage::ATA::PIO;
		uint32_t Cluster = (uint32_t) File->Cluster();
		uint32_t cchain = 0;

		uint64_t buf = MemoryManager::Physical::AllocateDMA((this->SectorsPerCluster * 512 + 0xFFF) / 0x1000);
		uint64_t BytesLeft = length;


		// read the cluster chain
		do
		{
			uint32_t FatSector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (Cluster * 4 / 512);
			uint32_t FatOffset = (Cluster * 4) % 512;

			// unfortunately we cannot read the entire FAT at once.
			// this->ParentPartition->GetStorageDevice()->Read(FatSector, buf, 512);
			IO::Read(this->ParentPartition->GetStorageDevice(), FatSector, buf, 512);

			uint8_t* clusterchain = (uint8_t*) buf;
			cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

			// cchain is the next cluster in the list.

			uint64_t bytespercluster = this->SectorsPerCluster * this->BytesPerSector;

			// this->ParentPartition->GetStorageDevice()->Read(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buf, bytespercluster);
			IO::Read(this->ParentPartition->GetStorageDevice(), this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buf, bytespercluster);

			uint8_t* contents = (uint8_t*) buf;

			if(!(math::min(BytesLeft, bytespercluster) % 8))
			{
				Memory::Copy64((void*)(Address + BufferOffset), contents, math::min(BytesLeft, bytespercluster) / 8);
			}
			else
			{
				Memory::Copy((void*)(Address + BufferOffset), contents, math::min(BytesLeft, bytespercluster));
			}

			BufferOffset += bytespercluster;

			if(bytespercluster > BytesLeft)
				break;

			BytesLeft -= bytespercluster;
			Cluster = cchain;

		} while((cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8) && BytesLeft > 0);

		MemoryManager::Physical::FreeDMA(buf, (this->SectorsPerCluster * 512 + 0xFFF) / 0x1000);
	}

	uint32_t FAT32::AllocateCluster(uint32_t PrevCluster)
	{
		uint32_t retclus = 0;
		uint32_t loopedclusters = 0;

		uint64_t buf = MemoryManager::Physical::AllocateDMA((this->SectorsPerCluster * 512 + 0xFFF) / 0x1000);

		// find the next free cluster.
		// loop through all clusters, start from 0
		uint32_t FatSector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + ((uint32_t) this->FirstUsableCluster * 4 / 512);

		// unfortunately we cannot read the entire FAT at once.
		// so, do it cluster by cluster.
		do
		{
			this->ParentPartition->GetStorageDevice()->Read(FatSector + (loopedclusters * 4) / 512, buf, this->SectorsPerCluster * 512);
			for(int c = 0; c < (this->SectorsPerCluster * 512) / 4; c++)
			{
				if(((uint32_t*) buf)[c] == 0x0)
				{
					retclus = (uint32_t) this->FirstUsableCluster + loopedclusters + c;
					break;
				}
			}

			if(retclus != 0)
				break;

			loopedclusters += (this->SectorsPerCluster * 512) / 4;

		} while(loopedclusters < (this->FATSectorSize * 512) / 4);

		if(retclus == 0)
		{
			HALT("Disk out of free clusters, cannot allocate space");
		}

		// free the buffer.
		MemoryManager::Physical::FreeDMA(buf, (this->SectorsPerCluster * 512 + 0xFFF) / 0x1000);


		// write an EndOfChain value (0x0FFFFFF8) to indicate a single length chain
		// (also to mark the cluster as allocated)
		{
			uint64_t newbuf = MemoryManager::Physical::AllocateDMA(1);
			uint32_t sector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (retclus * 4 / 512);
			uint32_t offset = (retclus * 4) % 512;

			// unfortunately we cannot read the entire FAT at once.
			this->ParentPartition->GetStorageDevice()->Read(sector, newbuf, 512);

			uint8_t* clusterchain = (uint8_t*) newbuf;
			*((uint32_t*)&clusterchain[offset])  = 0x0FFFFFF8;
			// write the change back to disk.
			this->ParentPartition->GetStorageDevice()->Write(sector, newbuf, 512);

			MemoryManager::Physical::FreeDMA(newbuf, 1);
		}

		if(PrevCluster != 0)
		{
			// we can't guarantee that the prevcluster lies in the buffer, so
			// might as well free it and allocate a new one.

			uint64_t newbuf = MemoryManager::Physical::AllocateDMA(1);
			uint32_t sector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (PrevCluster * 4 / 512);
			uint32_t offset = (PrevCluster * 4) % 512;

			// unfortunately we cannot read the entire FAT at once.
			this->ParentPartition->GetStorageDevice()->Read(sector, newbuf, 512);

			uint8_t* clusterchain = (uint8_t*) newbuf;
			*((uint32_t*)&clusterchain[offset])  = retclus & 0x0FFFFFFF;

			// write the change back to disk.
			this->ParentPartition->GetStorageDevice()->Write(sector, newbuf, 512);

			MemoryManager::Physical::FreeDMA(newbuf, 1);
		}

		return retclus;
	}

	void FAT32::WriteFile(VFS::File* file, uint64_t Address, uint64_t length)
	{
		using namespace HardwareAbstraction::Devices::Storage::ATA::PIO;

		uint64_t buffer = MemoryManager::Physical::AllocateDMA(1);
		uint64_t bc = this->SectorsPerCluster * 512;
		uint32_t Cluster = (uint32_t) file->Parent()->Cluster();
		uint32_t cchain = 0;
		DirectoryEntry* dirent = 0;

		this->ParentPartition->GetStorageDevice()->Read(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buffer, bc);
		do
		{
			// read the cluster
			// loop through dirents.

			uint8_t required = 1 + (uint8_t)(String::Length(file->Name()) + 12) / 13;
			uint64_t found = 0;
			for(uint64_t o = 0; o < this->SectorsPerCluster * 512; o += 32)
			{
				uint8_t firstchar = *((uint8_t*)(buffer + o));

				if(firstchar == 0x0 || firstchar == 0xE5)
				{
					found++;

					if(required == found)
					{
						dirent = (DirectoryEntry*)(buffer + o);
						Memory::Set(dirent, 0x0, 32);

						// make the name.
						for(uint64_t t = 0; t < (uint64_t) math::min(String::Length(file->Name()), 8); t++)
						{
							dirent->name[t] = file->Name()[t] & 0xDF;
						}


						dirent->ext[0] = 'L';
						dirent->ext[1] = 'F';
						dirent->ext[2] = 'N';

						// create a bunch of LFNs.

						uint64_t chars = 0;
						uint64_t off = o - 32;
						for(uint8_t m = 1; m < required; m++, off -= 32)
						{
							uint8_t* dat = (uint8_t*)(buffer + off);
							Memory::Set(dat, 0x0, 32);

							*dat = m | (m + 1 == required ? 0x40 : 0x0);

							// calc checksum
							uint8_t sum = 0;

							for (int i = 11; i > 0; i--)
								sum = (uint8_t)((sum & 1) << 7) + (sum >> 1) + ((uint8_t*) dirent)[11 - i];

							*(dat + 0xD) = sum;
							*(dat + 0xB) = 0x0F;

							*((uint16_t*)(dat + 1)) = file->Name()[chars + 0];
							*((uint16_t*)(dat + 3)) = file->Name()[chars + 1];
							*((uint16_t*)(dat + 5)) = file->Name()[chars + 2];
							*((uint16_t*)(dat + 7)) = file->Name()[chars + 3];
							*((uint16_t*)(dat + 9)) = file->Name()[chars + 4];


							*((uint16_t*)(dat + 0x0E)) = file->Name()[chars + 5];
							*((uint16_t*)(dat + 0x10)) = file->Name()[chars + 6];
							*((uint16_t*)(dat + 0x12)) = file->Name()[chars + 7];
							*((uint16_t*)(dat + 0x14)) = file->Name()[chars + 8];
							*((uint16_t*)(dat + 0x16)) = file->Name()[chars + 9];
							*((uint16_t*)(dat + 0x18)) = file->Name()[chars + 10];

							*((uint16_t*)(dat + 0x1A)) = 0;


							*((uint16_t*)(dat + 0x1C)) = file->Name()[chars + 11];
							*((uint16_t*)(dat + 0x1E)) = file->Name()[chars + 12];

							chars += 13;
						}

						break;
					}
				}
				else
				{
					found = 0;
				}
			}

			if(!dirent)
			{
				uint32_t FatSector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (Cluster * 4 / 512);
				uint32_t FatOffset = (Cluster * 4) % 512;

				// unfortunately we cannot read the entire FAT at once.
				this->ParentPartition->GetStorageDevice()->Read(FatSector, buffer, 512);

				uint8_t* clusterchain = (uint8_t*) buffer;
				cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

				this->ParentPartition->GetStorageDevice()->Read(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buffer, bc);
				Cluster = cchain;
			}

		} while(!dirent && (cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));


		if(!dirent)
		{
			// this is where we allocate a new cluster for the parent folder,
			// then call ourselves and return immediately after.
			// this lets the above code find the cluster, instead of duplicating code.

			AllocateCluster(Cluster);
			return this->WriteFile(file, Address, length);
		}




		// file exists now.
		// get a new cluster for the file.
		uint32_t firstclus = AllocateCluster();
		uint64_t clustersize = this->SectorsPerCluster * 512;
		uint64_t bytesleft = length;
		uint32_t prevcluster = firstclus;
		Log("Allocated cluster %d", firstclus);
		while(bytesleft > length)
		{
			prevcluster = AllocateCluster(prevcluster);
			Log("Allocated cluster %d", prevcluster);
			bytesleft -= math::min(clustersize, bytesleft);
		}

		// now we should have a cluster chain long enough to encompass the entire file's contents.
		// or at least the buffer length.

		// write the cluster to the file's dirent.
		{
			dirent->clusterlow = firstclus & 0xFFFF;
			dirent->clusterhigh = (firstclus >> 16) & 0xFFFF;
			dirent->filesize = (uint32_t) length;
			Log("%x, %x, %x, %x", Cluster, firstclus, buffer, bc);
			this->ParentPartition->GetStorageDevice()->Write(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buffer, bc);
		}




		cchain = 0;
		uint64_t buf = MemoryManager::Physical::AllocateDMA((length + 0xFFF) / 0x1000);
		uint64_t BufferOffset = 0;
		uint32_t curclus = firstclus;
		uint64_t left = length;
		Memory::Copy64((void*) buf, (void*) Address, length / 8);
		Memory::Copy((void*) (buf + length), (void*) (Address + length), length - (length / 8));

		uint64_t clusters = MemoryManager::Physical::AllocateDMA(1);
		do
		{
			uint32_t FatSector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (curclus * 4 / 512);
			uint32_t FatOffset = (curclus * 4) % 512;

			// unfortunately we cannot read the entire FAT at once.
			this->ParentPartition->GetStorageDevice()->Read(FatSector, clusters, 512);

			uint8_t* clusterchain = (uint8_t*) clusters;
			cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

			// cchain is the next cluster in the list.
			uint64_t bytespercluster = this->SectorsPerCluster * this->BytesPerSector;

			this->ParentPartition->GetStorageDevice()->Write(this->FirstUsableCluster + curclus * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buf + BufferOffset, math::min(bytespercluster, left));

			BufferOffset += bytespercluster;
			left -= bytespercluster;
			curclus = cchain;

		} while((cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));


		MemoryManager::Physical::FreeDMA(clusters, 1);
		MemoryManager::Physical::FreeDMA(buffer, 1);

		MemoryManager::Physical::FreeDMA(buf, (length + 0xFFF) / 0x1000);
	}


	void FAT32::AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset)
	{
		UNUSED(File);
		UNUSED(Address);
		UNUSED(length);
		UNUSED(offset);
	}










	uint16_t	FAT32::GetBytesPerSector(){ return this->BytesPerSector; }
	uint8_t		FAT32::GetSectorsPerCluster(){ return this->SectorsPerCluster; }
	uint16_t	FAT32::GetReservedSectors(){ return this->ReservedSectors; }
	uint8_t		FAT32::GetNumberOfFATS(){ return this->NumberOfFATs; }
	uint16_t	FAT32::GetNumberOfDirectories(){ return this->NumberOfDirectories; }

	uint32_t	FAT32::GetTotalSectors(){ return this->TotalSectors; }
	uint32_t	FAT32::GetHiddenSectors(){ return this->HiddenSectors; }
	uint32_t	FAT32::GetFATSectorSize(){ return this->FATSectorSize; }
	uint32_t	FAT32::GetRootDirectoryCluster(){ return this->RootDirectoryCluster; }

	uint16_t	FAT32::GetFSInfoCluster(){ return this->FSInfoCluster; }
	uint16_t	FAT32::GetBackupBootCluster(){ return this->BackupBootCluster; }
	uint32_t	FAT32::GetFirstUsableCluster(){ return (uint32_t) this->FirstUsableCluster; }



	char* FAT32::GetFileName(const char* filename)
	{
		// if this is a short filename...

		char* ext = new char[4];

		ext[0] = filename[8];
		ext[1] = filename[9];
		ext[2] = filename[10];
		ext[3] = filename[11];

		char* name = String::SubString(filename, 0, 8);

		String::TrimWhitespace((char*) name);
		String::TrimWhitespace(ext);
		if(String::Compare(ext, "   "))
		{
			delete[] ext;
			return (char*) name;
		}
		else
		{
			String::ConcatenateChar((char*) name, '.');
			String::Concatenate((char*) name, ext);

			delete[] ext;
			return (char*) name;
		}
	}

	char* FAT32::GetFolderName(const char* foldername)
	{
		char* d = new char[String::Length(foldername)];
		String::Copy(d, foldername);

		String::TrimWhitespace((char*) d);
		return d;
	}










}
}
}
