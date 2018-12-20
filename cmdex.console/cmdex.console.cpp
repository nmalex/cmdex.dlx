// cmdex.console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cmdex.h"

int main()
{
	auto res = cmdex::run(L"xcopy /?");
	wprintf(res);
	delete[] res;

    return 0;
}

