#pragma once
#include "PML/PML.h"
#define ClearFlag(cr3) ((ULONG64)cr3 & 0x000FFFFFFFFFF000)
#define NUM_1G			0x40000000
#define NUM_2M			0x200000
#define ENTRY_SIZE					0x8
#define PAGE_SIZE 0x1000
#define ROUND_DOWN(a,b) ((ULONG64)(a) & ~((ULONG64)(b) - 1))
#define ROUND_UP(a,b) (((ULONG64)(a) + ((ULONG64)(b) - 1)) & ~((ULONG64)(b) - 1))
typedef struct _MMPDPTE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
		/* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
		/* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
		/* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 reserved0 : 18; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 18; /* bit position: 30 */
		/* 0x0000 */ unsigned __int64 reserved1 : 4; /* bit position: 48 */
		/* 0x0000 */ unsigned __int64 SoftwareWsIndex : 11; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPDPTE, * PMMPDPTE; /* size: 0x0008 */
typedef struct _MMPDE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
		/* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
		/* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
		/* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 reserved0 : 9; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 27; /* bit position: 21 */
		/* 0x0000 */ unsigned __int64 reserved1 : 4; /* bit position: 48 */
		/* 0x0000 */ unsigned __int64 SoftwareWsIndex : 11; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPDE, * PMMPDE; /* size: 0x0008 */
typedef struct _MMPTE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
		/* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
		/* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
		/* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 36; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 reserved1 : 4; /* bit position: 48 */
		/* 0x0000 */ unsigned __int64 SoftwareWsIndex : 11; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPTE, * PMMPTE; /* size: 0x0008 */
typedef struct _MMVA
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Offset : 12;
		/* 0x0000 */ unsigned __int64 PT : 9;
		/* 0x0000 */ unsigned __int64 PDT : 9;
		/* 0x0000 */ unsigned __int64 PDPT : 9;
		/* 0x0000 */ unsigned __int64 PML4T : 9;
		/* 0x0000 */ unsigned __int64 Partition : 16; //User:0x0000 System:0xFFFF
	}; /* bitfield */
} MMVA, * PMMVA; /* size: 0x0008 */
typedef struct _MMVA_PDPTE_LARGE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Offset : 30;
		/* 0x0000 */ unsigned __int64 PDPT : 9;
		/* 0x0000 */ unsigned __int64 PML4T : 9;
		/* 0x0000 */ unsigned __int64 Partition : 16; //User:0x0000 System:0xFFFF
	}; /* bitfield */
} MMVA_PDPTE_LARGE, * PMMVA_PDPTE_LARGE; /* size: 0x0008 */
typedef struct _MMVA_PDE_LARGE
{
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Offset : 21;
		/* 0x0000 */ unsigned __int64 PDT : 9;
		/* 0x0000 */ unsigned __int64 PDPT : 9;
		/* 0x0000 */ unsigned __int64 PML4T : 9;
		/* 0x0000 */ unsigned __int64 Partition : 16; //User:0x0000 System:0xFFFF
	}; /* bitfield */
} MMVA_PDE_LARGE, * PMMVA_PDE_LARGE; /* size: 0x0008 */
static MAP_INFO MapedInfo{};
static PVOID KeGetSpecificAddressPhysicalByCR3(ULONG64 CR3, PVOID pVirtual)
{
	PMMVA		pAddressInfo = (PMMVA)&pVirtual;
	PVOID		pPML4T = (PVOID)ClearFlag(CR3);
	PMMPTE		pPML4E = NULL;
	PVOID		pPDPT = NULL;
	PMMPTE		pPDPTE = NULL;
	PVOID		pPDT = NULL;
	PMMPTE		pPDE = NULL;
	PVOID		pPT = NULL;
	PMMPTE		pPTE = NULL;
	PVOID		pPhysicalBase = NULL;
	PVOID		pPhysical = NULL;
	PVOID		pPhysicalR3 = NULL;

	pPML4E = (PMMPTE)((ULONG64)pPML4T + (pAddressInfo->PML4T * ENTRY_SIZE) + MapedInfo.MapedAddress);
	if (!pPML4E->Valid)
		return NULL;
	pPDPT = (PVOID)ClearFlag(*(PULONG64)pPML4E);

	pPDPTE = (PMMPTE)((ULONG64)pPDPT + pAddressInfo->PDPT * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPDPTE->Valid)
		return NULL;
	if (pPDPTE->LargePage)
		goto Lable_PDPTE_LargePage;
	pPDT = (PVOID)ClearFlag(*(PULONG64)pPDPTE);

	pPDE = (PMMPTE)((ULONG64)pPDT + pAddressInfo->PDT * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPDE->Valid)
		return NULL;
	if (pPDE->LargePage)
		goto Lable_PDE_LargePage;
	pPT = (PVOID)ClearFlag(*(PULONG64)pPDE);

	pPTE = (PMMPTE)((ULONG64)pPT + pAddressInfo->PT * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPTE->Valid)
		return NULL;
	pPhysicalBase = (PVOID)ClearFlag(*(PULONG64)pPTE);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + pAddressInfo->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + MapedInfo.MapedAddress);
	return pPhysicalR3;

Lable_PDPTE_LargePage:
	pPhysicalBase = (PVOID)(((PMMPDPTE)pPDPTE)->PageFrameNumber * NUM_1G);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + ((PMMVA_PDPTE_LARGE)pAddressInfo)->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + MapedInfo.MapedAddress);
	return pPhysicalR3;

Lable_PDE_LargePage:
	pPhysicalBase = (PVOID)(((PMMPDE)pPDE)->PageFrameNumber * NUM_2M);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + ((PMMVA_PDE_LARGE)pAddressInfo)->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + MapedInfo.MapedAddress);
	return pPhysicalR3;
}
static PVOID KAGetSpecificAddressPhysical(ULONG64 TargetProcessCr3,PVOID pVirtual)
{
	PMMVA		pAddressInfo = (PMMVA)&pVirtual;
	PVOID		pPML4T = (PVOID)ClearFlag(TargetProcessCr3);
	PMMPTE		pPML4E = NULL;
	PVOID		pPDPT = NULL;
	PMMPTE		pPDPTE = NULL;
	PVOID		pPDT = NULL;
	PMMPTE		pPDE = NULL;
	PVOID		pPT = NULL;
	PMMPTE		pPTE = NULL;
	PVOID		pPhysicalBase = NULL;
	PVOID		pPhysical = NULL;
	PVOID		pPhysicalR3 = NULL;

	pPML4E = (PMMPTE)((ULONG64)pPML4T + pAddressInfo->PML4T * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPML4E->Valid)
		return NULL;
	pPDPT = (PVOID)ClearFlag(*(PULONG64)pPML4E);

	pPDPTE = (PMMPTE)((ULONG64)pPDPT + pAddressInfo->PDPT * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPDPTE->Valid)
		return NULL;
	if (pPDPTE->LargePage)
		goto Lable_PDPTE_LargePage;
	pPDT = (PVOID)ClearFlag(*(PULONG64)pPDPTE);

	pPDE = (PMMPTE)((ULONG64)pPDT + pAddressInfo->PDT * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPDE->Valid)
		return NULL;
	if (pPDE->LargePage)
		goto Lable_PDE_LargePage;
	pPT = (PVOID)ClearFlag(*(PULONG64)pPDE);

	pPTE = (PMMPTE)((ULONG64)pPT + pAddressInfo->PT * ENTRY_SIZE + MapedInfo.MapedAddress);
	if (!pPTE->Valid)
		return NULL;
	pPhysicalBase = (PVOID)ClearFlag(*(PULONG64)pPTE);

	pPhysical = (PVOID)((ULONG64)pPhysicalBase + pAddressInfo->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + MapedInfo.MapedAddress);
	return pPhysicalR3;

Lable_PDPTE_LargePage:
	pPhysicalBase = (PVOID)(((PMMPDPTE)pPDPTE)->PageFrameNumber * NUM_1G);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + ((PMMVA_PDPTE_LARGE)pAddressInfo)->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + MapedInfo.MapedAddress);
	return pPhysicalR3;

Lable_PDE_LargePage:
	pPhysicalBase = (PVOID)(((PMMPDE)pPDE)->PageFrameNumber * NUM_2M);
	pPhysical = (PVOID)((ULONG64)pPhysicalBase + ((PMMVA_PDE_LARGE)pAddressInfo)->Offset);
	pPhysicalR3 = (PVOID)((ULONG64)pPhysical + MapedInfo.MapedAddress);
	return pPhysicalR3;
}
static BOOL KeReadPhysicalAddress(ULONG64 Cr3, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
	ULONG64 pCurr = (ULONG64)Address;
	ULONG64 pEnd = (ULONG64)Address + Size;
	ULONG64 pMappedPA = 0;
	SIZE_T CurrSize = 0;
	SIZE_T BytesCopied = 0;
	while (ROUND_DOWN(pCurr, PAGE_SIZE) < pEnd)
	{
		if (ROUND_DOWN(pCurr, PAGE_SIZE) + PAGE_SIZE < pEnd)
		{
			CurrSize = PAGE_SIZE - (SIZE_T)pCurr % PAGE_SIZE;
		}
		else
		{
			CurrSize = pEnd - pCurr;
		}

		pMappedPA = (ULONG64)KeGetSpecificAddressPhysicalByCR3(Cr3, (PVOID)pCurr);

		if (!pMappedPA)
		{
			return FALSE;
		}
		if (!IsBadReadPtr((PUCHAR)pMappedPA, CurrSize))
		{
			memcpy((PUCHAR)Buffer + BytesCopied, (PUCHAR)pMappedPA, CurrSize);
		}
		else
		{
			return FALSE;
		}

		BytesCopied += CurrSize;
		pCurr = ROUND_DOWN(pCurr, PAGE_SIZE) + PAGE_SIZE;
	}
	return TRUE;

}
static BOOL KeWritePhysicalAddress(ULONG64 Cr3, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{

	ULONG64 pCurr = (ULONG64)Address;
	ULONG64 pEnd = (ULONG64)Address + Size;
	ULONG64 pMappedPA = 0;
	SIZE_T CurrSize = 0;
	SIZE_T BytesCopied = 0;
	while (ROUND_DOWN(pCurr, PAGE_SIZE) < pEnd)
	{
		if (ROUND_DOWN(pCurr, PAGE_SIZE) + PAGE_SIZE < pEnd)
		{
			CurrSize = PAGE_SIZE - (SIZE_T)pCurr % PAGE_SIZE;
		}
		else
		{
			CurrSize = pEnd - pCurr;
		}

		pMappedPA = (ULONG64)KeGetSpecificAddressPhysicalByCR3(Cr3, (PVOID)pCurr);

		if (!pMappedPA)
		{
			return FALSE;
		}
		if (!IsBadReadPtr((PUCHAR)pMappedPA, CurrSize))
		{
			memcpy((PUCHAR)pMappedPA, (PUCHAR)Buffer + BytesCopied, CurrSize);
		}
		else
		{
			return FALSE;
		}

		BytesCopied += CurrSize;
		pCurr = ROUND_DOWN(pCurr, PAGE_SIZE) + PAGE_SIZE;
	}
	return TRUE;

}
static PVOID DecodeTableOne(ULONG64 BaseAddr, INT index1, INT index2, HANDLE dwmpid)
{
	INT i_id = 0;

	for (INT i = 0; i < 256; i++)
	{
		i_id = i * 4 + 1024 * index1 + 512 * index2 * 1024;

		if ((HANDLE)i_id == dwmpid)
		{
			return (PVOID)((ULONG64)BaseAddr + i * 0x10);
		}

	}

	return NULL;
}
static PVOID DecodeTableTwo(ULONG64 BaseAddr, INT index2, HANDLE pid)
{
	ULONG64 ul_baseAddr_1 = 0;

	PVOID TableTwo = NULL;

	for (SIZE_T i = 0; i < 512; i++)
	{
		KeReadPhysicalAddress(MapedInfo.SystemCr3, (BaseAddr + i * 8), &ul_baseAddr_1, sizeof(ULONG64));

		TableTwo = DecodeTableOne(ul_baseAddr_1, i, index2, pid);

		if (TableTwo != NULL)
		{
			return TableTwo;
		}
	}

	return NULL;
}
static PVOID DecodeTableThree(ULONG64 BaseAddr, HANDLE pid)
{
	PVOID TableThree = NULL;

	ULONG64 ul_baseAddr_2 = 0;

	for (INT i = 0; i < 512; i++)
	{
		KeReadPhysicalAddress(MapedInfo.SystemCr3, (BaseAddr + i * 8), &ul_baseAddr_2, sizeof(ULONG64));

		TableThree = DecodeTableTwo(ul_baseAddr_2, i, pid);

		if (TableThree != NULL)
		{
			return TableThree;
		}

	}

	return NULL;
}
static PVOID GetObjectAddrInHandleTable(IN PVOID pHandleTable, IN HANDLE Index)
{
	PVOID PspAddr = NULL;
	ULONG64 TableCode = NULL;
	KeReadPhysicalAddress(MapedInfo.SystemCr3, ((ULONG64)pHandleTable + 0x8), &TableCode, sizeof(ULONG64));
	INT List = TableCode & 3;
	ULONG64 BaseAddr = TableCode & (~3);
	if (List == 0)
	{
		PspAddr = DecodeTableOne(BaseAddr, 0, 0, Index);
		return PspAddr;
	}
	else if (List == 1)
	{
		PspAddr = DecodeTableTwo(BaseAddr, 0, Index);
		return PspAddr;
	}
	else if (List == 2)
	{
		PspAddr = DecodeTableThree(BaseAddr, Index);
		return PspAddr;
	}
	else
	{
		return NULL;
	}
}
static ULONG64 KeGetProcessSectionBase(HANDLE Pid)
{
	ULONG64 TargetProcessCr3 = NULL;

	ULONG64 TargetEprocess = NULL;

	ULONG64 TargetSectionBase = NULL;

	ULONG64 ObjectAddress = (ULONG64)GetObjectAddrInHandleTable((PVOID)MapedInfo.PspCidTable, Pid);

	KeReadPhysicalAddress(MapedInfo.SystemCr3, ObjectAddress, &TargetEprocess, sizeof(ULONG64));

	TargetEprocess = (TargetEprocess >> 16) | 0xFFFF000000000000;

	KeReadPhysicalAddress(MapedInfo.SystemCr3, (TargetEprocess + 0x28), &TargetProcessCr3, sizeof(ULONG64));

	KeReadPhysicalAddress(MapedInfo.SystemCr3, (TargetEprocess + MapedInfo.SectionBaseAddressOffset), &TargetSectionBase, sizeof(ULONG64));

	return TargetSectionBase;
}
static ULONG64 KeGetProcessCr3(HANDLE Pid)
{
	ULONG64 TargetProcessCr3 = NULL;
	
	ULONG64 TargetEprocess = NULL;

	ULONG64 TargetSectionBase = NULL;

	ULONG64 ObjectAddress = (ULONG64)GetObjectAddrInHandleTable((PVOID)MapedInfo.PspCidTable, Pid);

	KeReadPhysicalAddress(MapedInfo.SystemCr3, ObjectAddress, &TargetEprocess, sizeof(ULONG64));

	TargetEprocess = (TargetEprocess >> 16) | 0xFFFF000000000000;

	KeReadPhysicalAddress(MapedInfo.SystemCr3, (TargetEprocess + 0x28), &TargetProcessCr3, sizeof(ULONG64));

	return TargetProcessCr3;
}
static PEPROCESS KeGetEProcess(HANDLE Pid)
{
	ULONG64 TargetEprocess = NULL;
	ULONG64 TargetSectionBase = NULL;
	ULONG64 ObjectAddress = (ULONG64)GetObjectAddrInHandleTable((PVOID)MapedInfo.PspCidTable, Pid);
	if (!ObjectAddress) return NULL;
	KeReadPhysicalAddress(MapedInfo.SystemCr3, ObjectAddress, &TargetEprocess, sizeof(ULONG64));
	if (!TargetEprocess)  return NULL;
	TargetEprocess = (TargetEprocess >> 16) | 0xFFFF000000000000;
	return (PEPROCESS)KeGetSpecificAddressPhysicalByCR3(MapedInfo.SystemCr3, (PVOID)TargetEprocess);
}

template<typename T>
static T KeReadChain(ULONG64 Cr3, ULONG64 Address, std::initializer_list<ULONG64> offsets)
{
	T result = T();
	ULONG64 pCurr = Address;
	for (auto offset : offsets)
	{
		if (!KeReadPhysicalAddress(Cr3, pCurr + offset, &pCurr, sizeof(ULONG64)))
			return result;
	}
	KeReadPhysicalAddress(Cr3, pCurr, &result, sizeof(T));
	return result;
}