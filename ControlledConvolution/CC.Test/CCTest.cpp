// CCTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include "Shlwapi.h"
#include <iostream>

using namespace std;

void main(void)
{
	// Valid file path name (file is there).
	char buffer_1[] = "C:\\img.raw";
	char *lpStr1;
	lpStr1 = buffer_1;

	// Invalid file path name (file is not there).
	char buffer_2[] = "C:\\TEST\\file.doc";
	char *lpStr2;
	lpStr2 = buffer_2;

	// Return value from "PathFileExists".
	int retval;
	// Search for the presence of a file with a true result.
	Wow64DisableWow64FsRedirection(NULL);
	LPCTSTR lpPath = TEXT("C:\\img.raw");
	retval = PathFileExists(lpPath);
	Wow64RevertWow64FsRedirection(NULL);

	if (retval == 1)
	{
		cout << "Search for the file path of : " << lpStr1 << endl;
		cout << "The file requested \"" << lpStr1 << "\" is a valid file" << endl;
		cout << "The return from function is : " << retval << endl;
	}

	else
	{
		cout << "\nThe file requested " << lpStr1 << " is not a valid file" << endl;
		cout << "The return from function is : " << retval << endl;
	}

	// Search for the presence of a file with a false result.
	retval = PathFileExists((LPTSTR)lpStr2);

	if (retval == 1)
	{
		cout << "\nThe file requested " << lpStr2 << "is a valid file" << endl;
		cout << "Search for the file path of : " << lpStr2 << endl;
		cout << "The return from function is : " << retval << endl;
	}
	else
	{
		cout << "\nThe file requested \"" << lpStr2 << "\" is not a valid file" << endl;
		cout << "The return from function is : " << retval << endl;
	}
}
