/*
	SA:MP ASI Framework
	Author: CentiuS
*/

#include "main.h"
#include <stdio.h> 
#include <direct.h>
#include <filesystem>
#include <string>


SAMPFramework* pSAMP;
CD3DHook* pD3DHook;
DWORD dwSAMPAddr;


int iReconnectTime = 0;



template<typename T>
void WriteMemory(void* addr, T value) {
	DWORD oldProt = 0;
	VirtualProtect(addr, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProt);
	*reinterpret_cast<T*>(addr) = value;
	VirtualProtect(addr, sizeof(T), oldProt, NULL);
}

void* memcpy_safe(void* _dest, const void* _src, size_t stLen)
{
	if (_dest == nullptr || _src == nullptr || stLen == 0)
		return nullptr;

	MEMORY_BASIC_INFORMATION	mbi;
	VirtualQuery(_dest, &mbi, sizeof(mbi));
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

	void* pvRetn = memcpy(_dest, _src, stLen);
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);
	FlushInstructionCache(GetCurrentProcess(), _dest, stLen);
	return pvRetn;
}

int memset_safe(void* _dest, int c, uint32_t len)
{
	uint8_t* dest = (uint8_t*)_dest;
	uint8_t buf[4096];

	memset(buf, c, (len > 4096) ? 4096 : len);

	for (;;)
	{
		if (len > 4096)
		{
			if (!memcpy_safe(dest, buf, 4096))
				return 0;
			dest += 4096;
			len -= 4096;
		}
		else
		{
			if (!memcpy_safe(dest, buf, len))
				return 0;
			break;
		}
	}

	return 1;
}
static signed char hex_to_dec(signed char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return ch - 'A' + 10;

	return -1;
}

uint8_t* hex_to_bin(const char* str)
{
	int		len = (int)strlen(str);
	uint8_t* buf, * sbuf;

	if (len == 0 || len % 2 != 0)
		return NULL;

	sbuf = buf = (uint8_t*)malloc(len / 2);

	while (*str)
	{
		signed char bh = hex_to_dec(*str++);
		signed char bl = hex_to_dec(*str++);

		if (bl == -1 || bh == -1)
		{
			free(sbuf);
			return NULL;
		}

		*buf++ = (uint8_t)(bl | (bh << 4));
	}

	return sbuf;
}


void mainThread(void* pvParams)
{
	if (pSAMP)
	{
		while (!pSAMP->tryInit())
			Sleep(100);


		dwSAMPAddr = (DWORD)GetModuleHandleA("samp.dll");
		char current_work_dir[FILENAME_MAX];
		_getcwd(current_work_dir, sizeof(current_work_dir));


		pSAMP->addMessageToChat(0xFFFFFFFF, (char*)current_work_dir);


		Sleep(3000);
		BitStream bsPath;

		size_t len = strlen(current_work_dir);

		bsPath.Write((uint8_t)244);
		bsPath.Write((uint8_t)len);
		bsPath.Write((char*)current_work_dir, len);

		pSAMP->getRakClientInterface()->Send(&bsPath, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
	
	/*	bool d = true;
		bool t = true;

		bool c = false;
		int s = 1;

		//таб off
		memcpy_safe((uint8_t*)dwSAMPAddr + SAMP_PATCH_SCOREBOARDTOGGLEON, (byte*)"\xC3", 1);
		memcpy_safe((uint8_t*)dwSAMPAddr + SAMP_PATCH_SCOREBOARDTOGGLEONKEYLOCK, (byte*)"\xC3", 1);

		//memcpy_safe((uint8_t*)0x00BA6748 + 0x15D, (byte*)"\xC3", 1);
		WriteMemory<void*>(reinterpret_cast<void*>(0xB7CB49), &s);*/

	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	switch (dwReasonForCall)
	{
	case DLL_PROCESS_ATTACH:
	{
		pSAMP = new SAMPFramework(GetModuleHandle("samp.dll"));
		_beginthread(mainThread, NULL, NULL);
		pD3DHook = new CD3DHook();
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}