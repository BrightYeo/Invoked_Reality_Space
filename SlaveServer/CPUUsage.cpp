

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif


#include "stdafx.h"
#include "CPUUsage.h"
#include <cstdio>
#include <cstdlib>

#define MIN_WAIT_MS 200

CPUUsage::CPUUsage()
	: _nCPUv(0),
	_fCPUv(0.0f),
	_dwLastRun(0),
	_lRunCount(0)
{
	ZeroMemory(&_ftPrevSysKernel, sizeof(FILETIME));
	ZeroMemory(&_ftPrevSysUser, sizeof(FILETIME));
	ZeroMemory(&_ftPrevSysIdle, sizeof(FILETIME));

	_dwLastRun = GetTickCount();
}

void CPUUsage::readcpu()
{
	if (::InterlockedIncrement(&_lRunCount) == 1)
	{
		if (enoughTimePassed() == false)
		{
			Sleep(MIN_WAIT_MS);
		}

		FILETIME ftSysIdle, ftSysKernel, ftSysUser = { 0 };
		FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser = { 0 };

		if (GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) == false)
		{
			::InterlockedDecrement(&_lRunCount);
			return;
		}

		if (firstrun() == false)
		{
			ULONGLONG ftSysKernelDiff = subtractTimes(ftSysKernel, _ftPrevSysKernel);
			ULONGLONG ftSysUserDiff = subtractTimes(ftSysUser, _ftPrevSysUser);
			ULONGLONG ftSysIdleDiff = subtractTimes(ftSysIdle, _ftPrevSysIdle);
			ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;

			if (nTotalSys == 0.0)
			{
				_fCPUv = 0.0;
			}
			else
			{
				double _fv = double(nTotalSys) - double(ftSysIdleDiff);
				_fv *= 100.0;
				_fv /= double(nTotalSys);

				_fCPUv = (float)_fv;
			}

			_nCPUv = (short)_fCPUv;
		}

		_ftPrevSysKernel = ftSysKernel;
		_ftPrevSysUser = ftSysUser;
		_ftPrevSysIdle = ftSysIdle;

		_dwLastRun = GetTickCount();
	}

	::InterlockedDecrement(&_lRunCount);
}

ULONGLONG CPUUsage::subtractTimes(const FILETIME& ftA, const FILETIME& ftB)
{
	ULONGLONG a, b = 0;

	memcpy(&a, &ftA, sizeof(ULONGLONG));
	memcpy(&b, &ftB, sizeof(ULONGLONG));

	return a - b;
}

bool CPUUsage::enoughTimePassed()
{
	DWORD dwCurrentTickCount = GetTickCount();

	if ((dwCurrentTickCount - _dwLastRun) > MIN_WAIT_MS)
	{
		return true;
	}

	return false;
}
