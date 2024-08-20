#ifndef MEMANAGER_H_
#define MEMANAGER_H_

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

class MemoryManager
{
private:
    DWORD m_processId = 0;
    HANDLE m_processHandle = nullptr;

	DWORD getProcessID(const wchar_t *process_name)
	{
		PROCESSENTRY32W procEntry;
		procEntry.dwSize = sizeof(procEntry);

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (hSnap == INVALID_HANDLE_VALUE)
			return 0;

		if (Process32FirstW(hSnap, &procEntry))
		{
			do
			{
				if (std::wcscmp(procEntry.szExeFile, process_name) == 0)
				{
					CloseHandle(hSnap);
					return procEntry.th32ProcessID;
				}
			}
			while (Process32NextW(hSnap, &procEntry));
		}

		CloseHandle(hSnap);
		return 0;
	}
public:
	MemoryManager(const wchar_t *process_name)
	{
		this->m_processId = this->getProcessID(process_name);
		this->m_processHandle = OpenProcess(PROCESS_ALL_ACCESS, true, this->m_processId);
	}

	~MemoryManager()
	{
		CloseHandle(this->m_processHandle);
	}

	DWORD ProcessId()
	{
		return this->m_processId;
	}

	HANDLE ProcessHandle()
	{
		return this->m_processHandle;
	}

	uintptr_t GetModuleBaseAddress(const wchar_t *module_name)
	{
		uintptr_t modBaseAddr = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->m_processId);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);
			if (Module32First(hSnap, &modEntry))
			{
				do
				{
					if (!_wcsicmp(modEntry.szModule, module_name))
					{
						modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
		}
		CloseHandle(hSnap);
		return modBaseAddr;
	}

	template<typename T>
	T ReadMemory(const uintptr_t address_ptr)
	{
		if (this->m_processHandle == nullptr)
			return T();

		T buffer;

		if (!ReadProcessMemory(this->m_processHandle, (LPCVOID)address_ptr, &buffer, sizeof(buffer), NULL))
		{
			std::cout << "ERROR (ReadProcessMemory): " << GetLastError() << std::endl;
			return T();
		}

		return buffer;
	}

	template <typename T>
	bool WriteMemory(const uintptr_t address_ptr, const T& buffer)
	{
		if (this->m_processHandle == nullptr)
			return false;

		if (!WriteProcessMemory(this->m_processHandle, (LPVOID)address_ptr, &buffer, sizeof(buffer), NULL))
		{
			std::cout << "ERROR (WriteProcessMemory): " << GetLastError() << std::endl;
			return false;
		}

		return true;
	}
};

#endif // MEMANAGER_H_
