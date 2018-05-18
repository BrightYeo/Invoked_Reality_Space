#pragma once

#ifndef __CPUU_H__
#define __CPUU_H__

#ifndef _WIN32
#error "This source code only runs under Windows platfrom."
#endif

#include "stdafx.h"
#include <windows.h>

class CPUUsage
{
public:
	CPUUsage();

public:
	short  usage()      { readcpu(); return _nCPUv; }
	float  usage_f()    { readcpu(); return _fCPUv; }

private:
	ULONGLONG subtractTimes(const FILETIME& ftA, const FILETIME& ftB);
	bool      enoughTimePassed();

protected:
	void readcpu();

private:
	inline
		bool firstrun() const { return (_dwLastRun == 0); }

private:
	FILETIME    _ftPrevSysKernel;
	FILETIME    _ftPrevSysUser;
	FILETIME    _ftPrevSysIdle;

	short       _nCPUv;
	float       _fCPUv;
	DWORD       _dwLastRun;

private:
	volatile
		LONG        _lRunCount;
};

#endif /// of __CPUU_H__
