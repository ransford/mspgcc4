// gcclauncher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <bzscmn/file.h>
#include <bzswin/process.h>

using namespace BazisLib;

int _tmain(int argc, _TCHAR* argv[])
{
	LPCTSTR lpCmdLine = GetCommandLine();
	LPCTSTR lpEnd = NULL;
	if (lpCmdLine[0] == '\"')
		lpEnd = _tcschr(lpCmdLine + 1, '\"');
	else
	{
		lpEnd = _tcschr(lpCmdLine, ' ');
		if (lpEnd)
			lpEnd--;
	}

	String cmdArgs = lpEnd ? lpEnd + 1 : _T("");

	TCHAR tszExeName[MAX_PATH];
	GetModuleFileName(NULL, tszExeName, __countof(tszExeName));
	
	FilePath fp(tszExeName);
	String exeFileName = fp.GetFileName();
	fp = fp.Parent().Parent().Parent().PlusPath(_T("bin"));
	fp.AppendString(_T("\\msp430-"));
	fp.AppendString(exeFileName);

	if (!File::Exists(fp))
	{
		_tprintf(_T("Error: %s does not exist!\n"), fp.c_str());
		return 1;
	}

	DWORD exitCode = -1;

	BazisLib::Win32::Process::RunCommandLineSynchronously((LPTSTR)(fp.ToString() + cmdArgs).c_str(), &exitCode);
	
	return exitCode;
}

