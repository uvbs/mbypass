#include <windows.h>
#include "environment.h"
#include "hs_aes.h"
#include "heartbeat.h"
#include "log.h"
#include "maple.h"

// main dispatch function called by the game client to do various things incl generating heartbeat
bool __stdcall ServiceDispatch(unsigned int service, void **param, unsigned int *errorCode)
{
	if (service == 13)	// heartbeat request
	{
		Log(L"Heartbeat....\n");
		Heartbeat beat;
		beat.Beat((unsigned char *)param[0], (int)param[1], (unsigned char *)param[2]);
	}

	*errorCode = 0;
	return true;
}

// implement maplestory-only stuff (currently only has support for EMS, check in maple.cpp to update the crc)
void DoMaplestory()
{
	Log(L"Identified process as Maplestory\n");

	Maplestory::DisableCrc();
	Maplestory::DisableInternalHackdetection();
	Maplestory::EnableMultiMS();
	Maplestory::DisableExceptionOnClose();
}

BOOL WINAPI DllMain(HINSTANCE, DWORD r, LPVOID)
{
	if (r == DLL_PROCESS_ATTACH)
	{
		AllocConsole();	// comment this line to remove the console from showing
		hsaes_relink();	// initialize the ripped code from ehsvc.dll originally

		// get the current working directory to determine what the app might be
		wchar_t cwd[MAX_PATH + 1];
		GetCurrentDirectory(MAX_PATH + 1, cwd);
		Log(L"cwd: %s\n", cwd);

		if (wcsstr(_wcslwr(cwd), L"maplestory") != 0)
			DoMaplestory();

		// read the data needed to generate heartbeat from hshield files
		Environment::ReadVersionHeaders(L"hshield\\3n.mhe", L"hshield\\hshield.dat");
		Environment::ReadInfoFromLog(L"hshield\\hshield.log");
	} else if (r == DLL_PROCESS_DETACH)
	{
		FreeConsole();
	}

	return TRUE;
}