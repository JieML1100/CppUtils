#pragma once
#include <Windows.h>
#include <iostream>
#include <Shlobj.h>
#include <commdlg.h>
#include <filesystem>
#include <fstream>
#include <TlHelp32.h>
#include <winternl.h>
#include <vector>
#include "../Utils/Utils.h"

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif //NT_SUCCESS
typedef __int64 (*t_callkernel)(__int64, __int64, __int64, __int64);
static t_callkernel callkernel = NULL;
static HDC desktopDC = NULL;

typedef struct _KPROCESS* PEPROCESS;
typedef struct _KTHREAD* PETHREAD;

typedef struct _UNICODE_STRING64 {
	USHORT Length;
	USHORT MaximumLength;
	PWCH   Buffer;
} UNICODE_STRING64;
typedef struct _LDR_DATA_TABLE_ENTRY64
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING64 FullDllName;
	UNICODE_STRING64 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY64, * PLDR_DATA_TABLE_ENTRY64;
typedef struct _PEB_LDR_DATA32
{
	ULONG Length;
	BOOLEAN Initialized;
	ULONG SsHandle;
	LIST_ENTRY32 InLoadOrderModuleList;
	LIST_ENTRY32 InMemoryOrderModuleList;
	LIST_ENTRY32 InInitializationOrderModuleList;
	ULONG EntryInProgress;
} PEB_LDR_DATA32, * PPEB_LDR_DATA32;
typedef struct _STRING32 {
	USHORT   Length;
	USHORT   MaximumLength;
	ULONG  Buffer;
} STRING32;
typedef STRING32* PSTRING32;
typedef STRING32 UNICODE_STRING32;
typedef struct _LDR_DATA_TABLE_ENTRY32
{
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderModuleList;
	LIST_ENTRY32 InInitializationOrderModuleList;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union
	{
		LIST_ENTRY32 HashLinks;
		ULONG SectionPointer;
	};
	ULONG CheckSum;
	union
	{
		ULONG TimeDateStamp;
		ULONG LoadedImports;
	};
	ULONG EntryPointActivationContext;
	ULONG PatchInformation;
} LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;
enum class COMTYPE : ULONG64
{
    LOOKUP_PROCESS = 0xFFFFFFFF00000000,
    LOOKUP_THREAD,
    GET_PROCESS_EXIT_STATUS,
    GET_THREAD_EXIT_STATUS,
    GET_PROCESS_SECTION_BASE,

    READ_PROCESS_MEMORY,
    READ_PROCESS_MEMORY_DIR,
    SAFE_READ_PROCESS_MEMORY,
    READ_PROCESS_MEMORY_ATTACH,

    WRITE_PROCESS_MEMORY,
    WRITE_PROCESS_MEMORY_DIR,
    SAFE_WRITE_PROCESS_MEMORY,
    WRITE_PROCESS_MEMORY_ATTACH,

    READ_PHYSICAL_MEMORY,
    WRITE_PHYSICAL_MEMORY,
    VIRTUAL_QUERY,
    VIRTUAL_ALLOC,
    VIRTUAL_FREE,
    VIRTUAL_PROTECT,
    CREATE_THREAD,
    LOOKUP_PROCESS_MODULE,
    LOOKUP_PROCESS_MODULE_INDEX,
    MAP_ALL_PHYSICAL_MEMORY,
    DELETE_FILE,
    GET_CONTEXT_THREAD,
    SET_CONTEXT_THREAD,
    SUSPEND_PROCESS,
    RESUME_PROCESS,
    SUSPEND_THREAD,
    RESUME_THREAD,
    CALL_REMOTE_APC,
    SHUTDOWN_SYSTEM,
    READ_DIRTABLE,
};
typedef   enum   _SHUTDOWN_ACTION {
	ShutdownNoReboot,
	ShutdownReboot,
	ShutdownPowerOff
}SHUTDOWN_ACTION;
typedef struct _COMCLASS
{
    COMTYPE Type;
    PVOID InputData;
    PVOID RefData;
	LONG64 Status;
}COMCLASS, * PCOMCLASS;
typedef struct _COMLOOKUP
{
    HANDLE ProcessId;
    PEPROCESS Process;
}COMLOOKUP, * PCOMLOOKUP;
typedef struct _COM_READWRITE
{
    PEPROCESS Process;
    ULONG64 Address;
    SIZE_T Size;
}COM_READWRITE, * PCOM_READWRITE;
typedef struct _COM_VIRTUAL_PROTECT
{
	PEPROCESS Process;
	ULONG64 Address;
	SIZE_T Size;
	ULONG64 Protect;
}COM_VIRTUAL_PROTECT, * PCOM_VIRTUAL_PROTECT;
typedef struct _COM_ALLOC
{
	PEPROCESS Process;
	PVOID BaseAddress;
	SIZE_T Size;
	ULONG64 Type;
	ULONG64 Protect;
	PVOID Buffer;
}COM_ALLOC, * PCOM_ALLOC;
typedef struct _COM_FREE
{
	PEPROCESS Process;
	ULONG64 Address;
}COM_FREE, * PCOM_FREE;
typedef struct _LOOKUP_MODULE
{
	PEPROCESS Process;
	const wchar_t* ModuleName;
}LOOKUP_MODULE, * PLOOKUP_MODULE;
typedef struct _CREATE_THREAD
{
	PEPROCESS Process;
	PVOID Address;
	PVOID Parameter;
}CREATE_THREAD, * PCREATE_THREAD;
typedef struct _MAP_INFO
{
	ULONG64 PspCidTable;
	ULONG64 SystemCr3;
	ULONG64 SectionBaseAddressOffset;
	ULONG64 ExitStatusOffset;
    ULONG64 MapedAddress;
}MAP_INFO, * PMAP_INFO;
typedef struct _MEMORY_QUERT
{
	PEPROCESS Process;
	PVOID Address;
	ULONG64 MemoryInformationClass;
	PVOID Buffer;
	ULONG64 Length;
	PULONG64 ResultLength;
}MEMORY_QUERT, * PMEMORY_QUERT;
typedef struct _REMOTE_APC
{
    PEPROCESS Process;
    ULONG64 Address;
    ULONG64 args[9];
    BOOL AllThread;
}REMOTE_APC, * PREMOTE_APC;

BOOL InitIO();
static ULONG Peb64Offset()
{
    auto nt = NtBuildVersion();
    if (nt > 18363) {
        return 0x550;
    }
    else
    {
        if (nt > 9600) {
            return 0x3F8;
        }
        else {
            if (nt > 7601)
                return 0x3E8;
            else
                return 0x338;
        }
    }
}
static ULONG Peb32Offset()
{
    auto nt = NtBuildVersion();
    if (nt > 18363) {
        return 0x580;
    }
    else
    {
        if (nt > 9600) {
            return 0x428;
        }
        else {
            if (nt > 7601)
                return 0x418;
            else
                return 0x320;
        }
    }
}
static HANDLE GetProcessIdByName(const wchar_t* ProcName)
{
    HANDLE Pid = NULL;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W Entry;
    ZeroMemory(&Entry, sizeof(Entry));
    Entry.dwSize = sizeof(Entry);
    if (Process32FirstW(Snapshot, &Entry)){
        do{
            if (_wcsicmp(Entry.szExeFile, ProcName) == 0){
                Pid = (HANDLE)Entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(Snapshot, &Entry));
    }
    return Pid;
}
static NTSTATUS CallR0(COMTYPE type, PVOID inData, PVOID refData)
{
    if (callkernel == NULL)
    {
        InitIO();
    }
    COMCLASS com = {};
    com.Type = type;
    com.InputData = inData;
    com.RefData = refData;
    ULONG64 enc = (ULONG64)&com;
    enc += 0x1000000000000000;
    UINT64 state = (ULONG64)desktopDC;
    callkernel((__int64)&state, (__int64)&state, (__int64)&state, (__int64)enc);
    return com.Status;
}
static BOOL InitIO()
{
    if (!callkernel)
    {
        desktopDC = GetDC(NULL);
        auto win32u = LoadLibraryA("win32u.dll");
        if (win32u == NULL) return FALSE;
        callkernel = (t_callkernel)GetProcAddress(win32u, "NtTokenManagerOpenSectionAndEvents");

        //auto ntdll = LoadLibraryA("ntdll.dll");
        //if (ntdll == NULL) return FALSE;
        //callkernel = (t_callkernel)GetProcAddress(ntdll, "NtQueryAuxiliaryCounterFrequency");

        if (callkernel == NULL) return FALSE;
        auto tmp = VirtualAlloc((PVOID)NULL, 0x18, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (tmp == NULL) return FALSE;
        memcpy(tmp, callkernel, 0x18);
        callkernel = (t_callkernel)tmp;
    }
    PEPROCESS Process = NULL; 
    return (NT_SUCCESS(CallR0(COMTYPE::LOOKUP_PROCESS, (HANDLE)GetCurrentProcessId(), &Process))) & (Process != NULL);
}
static NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process)
{
    return CallR0(COMTYPE::LOOKUP_PROCESS, ProcessId, Process);
}
static NTSTATUS PsLookupThreadByThreadId(HANDLE ThreadId, PETHREAD* Thread)
{
    return CallR0(COMTYPE::LOOKUP_THREAD, ThreadId, Thread);
}
static LDR_DATA_TABLE_ENTRY64 PsLookProcessModule(PEPROCESS Process, const wchar_t* moduleName)
{
    LDR_DATA_TABLE_ENTRY64 result = {};
    LOOKUP_MODULE inData = {};
    inData.Process = Process;
    inData.ModuleName = moduleName;
    CallR0(COMTYPE::LOOKUP_PROCESS_MODULE, &inData, &result);
    return result;
}
static LDR_DATA_TABLE_ENTRY64 PsLookProcessModuleIndex(PEPROCESS Process, unsigned int moduleIndex)
{
    LDR_DATA_TABLE_ENTRY64 result = {};
    LOOKUP_MODULE inData = {};
    inData.Process = Process;
    inData.ModuleName = (const wchar_t*)moduleIndex;
    CallR0(COMTYPE::LOOKUP_PROCESS_MODULE_INDEX, &inData, &result);
    return result;
}
static PVOID MmVirtualAlloc(PEPROCESS Process, SIZE_T Size, ULONG64 Type = MEM_COMMIT, ULONG64 Protect = PAGE_EXECUTE_READWRITE, PVOID BaseAddress = NULL,PVOID buffer = NULL)
{
    COM_ALLOC inData = {};
    inData.Process = Process;
    inData.BaseAddress = BaseAddress;
    inData.Size = Size;
    inData.Type = Type;
    inData.Protect = Protect; 
    inData.Buffer = buffer;
    PVOID result = NULL;
    CallR0(COMTYPE::VIRTUAL_ALLOC, &inData, &result);
    return result;
}
static NTSTATUS VirtualFree(PEPROCESS Process, PVOID Address)
{
    COM_FREE inData = {};
    inData.Process = Process;
    inData.Address = (ULONG64)Address;
    return CallR0(COMTYPE::VIRTUAL_FREE, &inData, NULL);
}
static NTSTATUS PsGetProcessExitStatus(PEPROCESS Process)
{
    return CallR0(COMTYPE::GET_PROCESS_EXIT_STATUS, Process, NULL);
}
static NTSTATUS PsGetThreadExitStatus(PETHREAD Thread)
{
    return CallR0(COMTYPE::GET_THREAD_EXIT_STATUS, Thread, NULL);
}
static PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process)
{
    PVOID result = NULL;
    CallR0(COMTYPE::GET_PROCESS_SECTION_BASE, Process, &result);
    return result;
}
static NTSTATUS ReadProcessMemory(PEPROCESS Process, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::READ_PROCESS_MEMORY, &inData, Buffer);
}
static NTSTATUS MmReadProcessMemory(PEPROCESS Process, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::SAFE_READ_PROCESS_MEMORY, &inData, Buffer);
}
static NTSTATUS ReadProcessMemoryAttach(PEPROCESS Process, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::READ_PROCESS_MEMORY_ATTACH, &inData, Buffer);
}
static NTSTATUS WriteProcessMemory(PEPROCESS Process, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::WRITE_PROCESS_MEMORY, &inData, Buffer);
}
static NTSTATUS MmWriteProcessMemory(PEPROCESS Process, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::SAFE_WRITE_PROCESS_MEMORY, &inData, Buffer);
}
static NTSTATUS WriteProcessMemoryAttach(PEPROCESS Process, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::WRITE_PROCESS_MEMORY_ATTACH, &inData, Buffer);
}
static NTSTATUS ReadProcessMemoryDir(ULONG64* inTable, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = (PEPROCESS)inTable;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::READ_PROCESS_MEMORY_DIR, &inData, Buffer);
}
static NTSTATUS WriteProcessMemoryDir(ULONG64* inTable, ULONG64 Address, PVOID Buffer, SIZE_T Size)
{
    COM_READWRITE inData = {};
    inData.Process = (PEPROCESS)inTable;
    inData.Address = Address;
    inData.Size = Size;
    return CallR0(COMTYPE::WRITE_PROCESS_MEMORY_DIR, &inData, Buffer);
}
static NTSTATUS ReadProcessDirTable(PEPROCESS Process, ULONG64* outTable)
{
    COM_READWRITE inData = {};
    inData.Process = Process;
    return CallR0(COMTYPE::READ_DIRTABLE, &inData, outTable);
}
static NTSTATUS VirtualProtect(PEPROCESS Process, ULONG64 Address, SIZE_T Size,ULONG newProtect,PULONG oldProtect)
{
    COM_VIRTUAL_PROTECT inData = {};
    inData.Process = Process;
    inData.Address = Address;
    inData.Size = Size;
    inData.Protect = newProtect;
    return CallR0(COMTYPE::VIRTUAL_PROTECT, &inData, oldProtect);
}
static HANDLE RtlCreateThread(PEPROCESS Process, ULONG64 Address, PVOID Parameter)
{
    CREATE_THREAD inData = {};
    inData.Process = Process;
    inData.Address = (PVOID)Address;
    inData.Parameter = Parameter;
    HANDLE handle = NULL;
    CallR0(COMTYPE::CREATE_THREAD, &inData, &handle);
    return handle;
}
static NTSTATUS WriteProcessMemory(PEPROCESS Process, ULONG64 Address, std::initializer_list<uint8_t> value)
{
    return WriteProcessMemory(Process, Address, (void*)value.begin(), value.size());
}
EXTERN_C MAP_INFO MapedInfo;
static MAP_INFO MapAllPhysicalMemory(ULONG64 virtualBase)
{
    MAP_INFO result = {};
    CallR0(COMTYPE::MAP_ALL_PHYSICAL_MEMORY, (PVOID)virtualBase, &result);
    MapedInfo = result;
    return MapedInfo;
}
static NTSTATUS QueryVirtualMemory(PEPROCESS Process, PVOID BaseAddress,int MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength)
{
    MEMORY_QUERT inData = {};
    inData.Process = Process;
    inData.Address = (PVOID)BaseAddress;
    inData.MemoryInformationClass = MemoryInformationClass;
    inData.Buffer = MemoryInformation;
    inData.Length = MemoryInformationLength;
    inData.ResultLength = ReturnLength;
    return CallR0(COMTYPE::VIRTUAL_QUERY, &inData, NULL);
}
static NTSTATUS PsDeleteFile(std::wstring path)
{
    return CallR0(COMTYPE::DELETE_FILE, (PVOID)path.c_str(), NULL);
}
static NTSTATUS PsGetContextThread(IN PETHREAD Thread, IN OUT PCONTEXT ThreadContext)
{
    return CallR0(COMTYPE::GET_CONTEXT_THREAD, (PVOID)Thread, ThreadContext);
}
static NTSTATUS PsSetContextThread(IN PETHREAD Thread, IN OUT PCONTEXT ThreadContext)
{
    return CallR0(COMTYPE::SET_CONTEXT_THREAD, (PVOID)Thread, ThreadContext);
}
static NTSTATUS PsSuspendProcess(HANDLE ProcessId)
{
    return CallR0(COMTYPE::SUSPEND_PROCESS, (PVOID)ProcessId, NULL);
}
static NTSTATUS PsResumeProcess(HANDLE ProcessId)
{
    return CallR0(COMTYPE::RESUME_PROCESS, (PVOID)ProcessId, NULL);
}
static NTSTATUS PsSuspendThread(HANDLE ThreadHandle, PULONG PreviousSuspendCount)
{
    return CallR0(COMTYPE::SUSPEND_THREAD, (PVOID)ThreadHandle, PreviousSuspendCount);
}
static NTSTATUS PsResumeThread(HANDLE ThreadHandle, PULONG SuspendCount)
{
    return CallR0(COMTYPE::RESUME_THREAD, (PVOID)ThreadHandle, SuspendCount);
}
static NTSTATUS PsShutdownSystem(SHUTDOWN_ACTION sa)
{
    return CallR0(COMTYPE::SHUTDOWN_SYSTEM, (PVOID)sa, NULL);
}
static NTSTATUS QueryRemoteAPC(PEPROCESS process,PVOID func,
    PVOID arg1 = NULL,PVOID arg2 = NULL, PVOID arg3 = NULL, PVOID arg4 = NULL, PVOID arg5 = NULL, PVOID arg6 = NULL, PVOID arg7 = NULL, PVOID arg8 = NULL, PVOID arg9 = NULL)
{
	REMOTE_APC inData = {};
	inData.Process = process;
	inData.Address = (ULONG64)func;
	inData.args[0] = (ULONG64)arg1;
	inData.args[1] = (ULONG64)arg2;
	inData.args[2] = (ULONG64)arg3;
	inData.args[3] = (ULONG64)arg4;
    inData.args[4] = (ULONG64)arg5;
    inData.args[5] = (ULONG64)arg6;
    inData.args[6] = (ULONG64)arg7;
    inData.args[7] = (ULONG64)arg8;
    inData.args[8] = (ULONG64)arg9;
	inData.AllThread = FALSE;
	return CallR0(COMTYPE::CALL_REMOTE_APC, &inData, NULL);
}
static ULONG64 SerachProcess(PEPROCESS process, const wchar_t* _module, const char* key)
{
    LDR_DATA_TABLE_ENTRY64 moduleinfo = PsLookProcessModule(process, _module);
    SIZE_T dwsiz = 0;
    uint8_t* BUFFER = new uint8_t[moduleinfo.SizeOfImage];
    if (BUFFER)
    {
        ZeroMemory(BUFFER, moduleinfo.SizeOfImage);
        auto st = MmReadProcessMemory(process, (ULONG64)moduleinfo.DllBase, BUFFER, moduleinfo.SizeOfImage);
        if (NT_SUCCESS(st))
        {
            auto found = FindPattern(BUFFER, key, moduleinfo.SizeOfImage, 0);
            if (found)
            {
                uint8_t* pb = (uint8_t*)found;
                pb -= (ULONG64)BUFFER;
                pb += (ULONG64)moduleinfo.DllBase;
                return ULONG64(pb);
            }

        }
        delete[] BUFFER;
    }
    return NULL;
}
static ULONG64 SerachProcess(PEPROCESS process, const char* key)
{
    MEMORY_BASIC_INFORMATION info = { 0 };
    SIZE_T dwsiz = 0;
    PVOID CurrentAddress = NULL;
    while NT_SUCCESS(QueryVirtualMemory(process, CurrentAddress, 0, &info, sizeof(MEMORY_BASIC_INFORMATION), &dwsiz))
    {
        if (info.State == MEM_COMMIT)
        {
            uint8_t* BUFFER = new uint8_t[info.RegionSize];
            if (BUFFER)
            {
                ZeroMemory(BUFFER, info.RegionSize);
                if (NT_SUCCESS(MmReadProcessMemory(process, (ULONG64)info.BaseAddress, BUFFER, info.RegionSize)))
                {
                    auto found = FindAllPattern(BUFFER, key, info.RegionSize, 0);
                    for (auto p : found)
                    {
                        uint8_t* pb = (uint8_t*)p;
                        pb -= (ULONG64)BUFFER;
                        pb += (ULONG64)info.BaseAddress;
                        return ULONG64(pb);
                    }
                }
                delete[] BUFFER;
            }
        }
        CurrentAddress = (PVOID)((UINT64)info.BaseAddress + info.RegionSize);
    }
    return NULL;
}
static std::vector<ULONG64> SerachProcessEx(PEPROCESS process, const wchar_t* _module, const char* key)
{
    LDR_DATA_TABLE_ENTRY64 moduleinfo = PsLookProcessModule(process, _module);
    SIZE_T dwsiz = 0;
    std::vector<ULONG64> result = std::vector<ULONG64>();
    uint8_t* BUFFER = new uint8_t[moduleinfo.SizeOfImage];
    if (BUFFER)
    {
        ZeroMemory(BUFFER, moduleinfo.SizeOfImage);
        if (NT_SUCCESS(MmReadProcessMemory(process, (ULONG64)moduleinfo.DllBase, BUFFER, moduleinfo.SizeOfImage)))
        {
            auto found = FindAllPattern(BUFFER, key, moduleinfo.SizeOfImage, 0);
            for (auto p : found)
            {
                uint8_t* pb = (uint8_t*)p;
                pb -= (ULONG64)BUFFER;
                pb += (ULONG64)moduleinfo.DllBase;
                result.push_back(ULONG64(pb));
            }

        }
        else
            printf("ReadProcessMemory Failed\n");
        delete[] BUFFER;
    }
    return result;
}
static std::vector<ULONG64> SerachProcessEx(PEPROCESS process, const char* key)
{
    MEMORY_BASIC_INFORMATION info = { 0 };
    SIZE_T dwsiz = 0;
    PVOID CurrentAddress = NULL;
    std::vector<ULONG64> result = std::vector<ULONG64>();
    while NT_SUCCESS(QueryVirtualMemory(process, CurrentAddress, 0, &info, sizeof(MEMORY_BASIC_INFORMATION), &dwsiz))
    {
        if (info.State == MEM_COMMIT)
        {
            uint8_t* BUFFER = new uint8_t[info.RegionSize];
            if (BUFFER)
            {
                ZeroMemory(BUFFER, info.RegionSize);
                if (NT_SUCCESS(MmReadProcessMemory(process, (ULONG64)info.BaseAddress, BUFFER, info.RegionSize)))
                {
                    auto found = FindAllPattern(BUFFER, key, info.RegionSize, 0);
                    for (auto p : found)
                    {
                        uint8_t* pb = (uint8_t*)p;
                        pb -= (ULONG64)BUFFER;
                        pb += (ULONG64)info.BaseAddress;
                        result.push_back(ULONG64(pb));
                    }
                }
                delete[] BUFFER;
            }
        }
        CurrentAddress = (PVOID)((UINT64)info.BaseAddress + info.RegionSize);
    }
    return result;
}
static BOOL DumpPE(PEPROCESS Process, const wchar_t* moduleName, const wchar_t* szOutPe)
{
    LDR_DATA_TABLE_ENTRY64 info = PsLookProcessModule(Process, moduleName);
    ULONG64 lpAddress = (ULONG64)info.DllBase;
    CHAR bDosHeader[sizeof(IMAGE_DOS_HEADER)] = { 0 };
    if (!NT_SUCCESS(MmReadProcessMemory(Process, lpAddress, bDosHeader, sizeof(bDosHeader))))
        return FALSE;
    PIMAGE_DOS_HEADER lpDosHeader = (PIMAGE_DOS_HEADER)bDosHeader;
    CHAR bNtHeader[sizeof(IMAGE_NT_HEADERS)] = { 0 };
    if (!NT_SUCCESS(MmReadProcessMemory(Process, (ULONG64)((ULONGLONG)lpAddress + lpDosHeader->e_lfanew), bNtHeader, sizeof(bNtHeader))))
        return FALSE;
    PIMAGE_NT_HEADERS lpNtHeader = (PIMAGE_NT_HEADERS)bNtHeader;
    uint8_t* lpMappedImage = new uint8_t[lpNtHeader->OptionalHeader.SizeOfImage];
    if (!lpMappedImage)
        return FALSE;
    memset(lpMappedImage, 0, lpNtHeader->OptionalHeader.SizeOfImage);
    if (!NT_SUCCESS(MmReadProcessMemory(Process, lpAddress, lpMappedImage, lpNtHeader->OptionalHeader.SizeOfImage)))
        return FALSE;
    if (!DeleteFileW(szOutPe) && ERROR_FILE_NOT_FOUND != GetLastError())
        return FALSE;
    HANDLE hFile;
    if (!(hFile = CreateFileW(
        szOutPe,
        FILE_APPEND_DATA,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    )) || INVALID_HANDLE_VALUE == hFile)
        return FALSE;
    DWORD dwWrittenBytes;
    if (!WriteFile(
        hFile,
        lpMappedImage,
        lpNtHeader->OptionalHeader.SizeOfHeaders,
        &dwWrittenBytes,
        NULL
    ) || (lpNtHeader->OptionalHeader.SizeOfHeaders != dwWrittenBytes))
        return FALSE;
    IMAGE_SECTION_HEADER* lpSectionHeaderArray = (IMAGE_SECTION_HEADER*)((ULONG_PTR)lpMappedImage + lpDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

    for (DWORD dwSecIndex = 0; dwSecIndex < lpNtHeader->FileHeader.NumberOfSections; dwSecIndex++)
    {
        if (!WriteFile(
            hFile,
            (LPVOID)((ULONGLONG)lpMappedImage + lpSectionHeaderArray[dwSecIndex].VirtualAddress),
            lpSectionHeaderArray[dwSecIndex].SizeOfRawData,
            &dwWrittenBytes,
            NULL
        ) || (lpSectionHeaderArray[dwSecIndex].SizeOfRawData != dwWrittenBytes))
            return FALSE;
    };

    CloseHandle(hFile);
    return TRUE;
}
static BOOL DumpPEEx(PEPROCESS Process, ULONG64 lpAddress, const wchar_t* szOutPe)
{
    CHAR bDosHeader[sizeof(IMAGE_DOS_HEADER)] = { 0 };
    if (!NT_SUCCESS(MmReadProcessMemory(Process, lpAddress, bDosHeader, sizeof(bDosHeader))))
        return FALSE;
    PIMAGE_DOS_HEADER lpDosHeader = (PIMAGE_DOS_HEADER)bDosHeader;
    CHAR bNtHeader[sizeof(IMAGE_NT_HEADERS)] = { 0 };
    if (!NT_SUCCESS(MmReadProcessMemory(Process, (ULONG64)((ULONGLONG)lpAddress + lpDosHeader->e_lfanew), bNtHeader, sizeof(bNtHeader))))
        return FALSE;
    PIMAGE_NT_HEADERS lpNtHeader = (PIMAGE_NT_HEADERS)bNtHeader;
    uint8_t* lpMappedImage = new uint8_t[lpNtHeader->OptionalHeader.SizeOfImage];
    if (!lpMappedImage)
        return FALSE;
    memset(lpMappedImage, 0, lpNtHeader->OptionalHeader.SizeOfImage);
    if (!NT_SUCCESS(MmReadProcessMemory(Process, lpAddress, lpMappedImage, lpNtHeader->OptionalHeader.SizeOfImage)))
        return FALSE;
    if (!DeleteFileW(szOutPe) && ERROR_FILE_NOT_FOUND != GetLastError())
        return FALSE;
    HANDLE hFile;
    if (!(hFile = CreateFileW(
        szOutPe,
        FILE_APPEND_DATA,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    )) || INVALID_HANDLE_VALUE == hFile)
        return FALSE;
    DWORD dwWrittenBytes;
    if (!WriteFile(
        hFile,
        lpMappedImage,
        lpNtHeader->OptionalHeader.SizeOfHeaders,
        &dwWrittenBytes,
        NULL
    ) || (lpNtHeader->OptionalHeader.SizeOfHeaders != dwWrittenBytes))
        return FALSE;
    IMAGE_SECTION_HEADER* lpSectionHeaderArray = (IMAGE_SECTION_HEADER*)((ULONG_PTR)lpMappedImage + lpDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

    for (DWORD dwSecIndex = 0; dwSecIndex < lpNtHeader->FileHeader.NumberOfSections; dwSecIndex++)
    {
        if (!WriteFile(
            hFile,
            (LPVOID)((ULONGLONG)lpMappedImage + lpSectionHeaderArray[dwSecIndex].VirtualAddress),
            lpSectionHeaderArray[dwSecIndex].SizeOfRawData,
            &dwWrittenBytes,
            NULL
        ) || (lpSectionHeaderArray[dwSecIndex].SizeOfRawData != dwWrittenBytes))
            return FALSE;
    };

    CloseHandle(hFile);
    return TRUE;
}
template<typename T>
static T ReadProcessMemory(PEPROCESS Process, ULONG64 Address)
{
	T result = T();
	ReadProcessMemory(Process, Address, &result, sizeof(T));
	return result;
}
template<typename T>
static T ReadProcessMemoryDir(ULONG64* dirTable, ULONG64 Address)
{
	T result = T();
	ReadProcessMemory(dirTable, Address, &result, sizeof(T));
	return result;
}
template<typename T>
static T MmReadProcessMemory(PEPROCESS Process, ULONG64 Address)
{
	T result = T();
    MmReadProcessMemory(Process, Address, &result, sizeof(T));
	return result;
}
template<typename T>
static NTSTATUS WriteProcessMemory(PEPROCESS Process, ULONG64 Address, T value)
{
	return WriteProcessMemory(Process, Address, &value, sizeof(T));
}
template<typename T>
static NTSTATUS WriteProcessMemoryDir(ULONG64* dirTable, ULONG64 Address, T value)
{
	return WriteProcessMemoryDir(dirTable, Address, &value, sizeof(T));
}
template<typename T>
static NTSTATUS MmWriteProcessMemory(PEPROCESS Process, ULONG64 Address, T value)
{
	return SafeWriteProcessMemory(Process, Address, &value, sizeof(T));
}
template<typename T>
static T ReadProcessMemory(PEPROCESS Process, std::initializer_list<ULONG64> offsets)
{
	ULONG64 Address = ReadProcessMemory<ULONG64>(Process, offsets.begin()[0]);
	for (int i = 1; i < offsets.size() - 1; i++)
	{
		Address = ReadProcessMemory<ULONG64>(Process, Address + offsets.begin()[i]);
	}
	Address += offsets.begin()[offsets.size() - 1];
	T result = T();
	ReadProcessMemory(Process, Address, &result, sizeof(T));
	return result;
}
template<typename T>
static T MmReadProcessMemory(PEPROCESS Process, std::initializer_list<ULONG64> offsets)
{
	ULONG64 Address = MmReadProcessMemory<ULONG64>(Process, offsets.begin()[0]);
	for (int i = 1; i < offsets.size() - 1; i++)
	{
		Address = MmReadProcessMemory<ULONG64>(Process, Address + offsets.begin()[i]);
	}
	Address += offsets.begin()[offsets.size() - 1];
	T result = T();
	ReadProcessMemory(Process, Address, &result, sizeof(T));
	return result;
}
template<typename T>
static T ReadChain(PEPROCESS Process, ULONG64 Address, std::initializer_list<ULONG64> offsets)
{
    for (auto offset : offsets)
    {
        Address = ReadProcessMemory<ULONG64>(Process, Address + offset);
        if (!Address)
            return T();
    }
    T result = T();
    ReadProcessMemory(Process, Address, &result, sizeof(T));
    return result;
}
template<typename T>
static T MmReadChain(PEPROCESS Process, ULONG64 Address, std::initializer_list<ULONG64> offsets)
{
    for (auto offset : offsets)
    {
        Address = MmReadProcessMemory<ULONG64>(Process, Address + offset);
        if (!Address)
            return T();
    }
    T result = T();
    MmReadProcessMemory(Process, Address, &result, sizeof(T));
    return result;
}
