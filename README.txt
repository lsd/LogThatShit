// -=[ LogThatShit v1.0 ]=- 
//	A stealthly keylogger designed on and for Windows XP 
//	By Isam M. <r0cket(NO_SPAM)jump@yahoo.com> 
//   http://biodegradablegeek.com


Table of contents (Use unique code to jump) 
----------------------------------------------------------------------------------------- 
0x01 About
0x02 Usage
0x03 FTP Information 
0x04 Config Variable List 
0x05 Gr33tz 
----------------------------------------------------------------------------------------- 


----------------------------------------------------------------------------------------- 
0x01 About 
----------------------------------------------------------------------------------------- 
	LogThatShit (LTS) is a stealthy key stroke logger for Windows 9X/NT/2000/XP/2K3. It 
is recommended that you read at least the usage part and skim the variable list before 
proceeding to output a TLS infection for your victim(s). 

Possible changes in the next version include: 
* Add filters, so you can only log key strokes typed into certain apps/windows. 
* Elimination of the hook DLL. A system-wide hook IS possible without a DLL. 
* Replace DLL with code injection. This will eliminate Injection.dll and reduce filesize. 
* If I feel energetic, maybe reduce filesize by not linking against the C runtime. 
* An interface to configure the output, instead of using an INI file :) 


----------------------------------------------------------------------------------------- 
0x02 Usage 
----------------------------------------------------------------------------------------- 
	Edit LogThatShit.ini and then run LogThatShit.exe, assuming no errors occured, you will 
get an output file (whatever you named it in LogThatShit.ini, default is Output.exe) Send 
the output file to your peers. Tell them it's Unreal Tournament 2008, or that it's the 
latest Puff Daddy MP3, who wouldn't download and execute that? If everything went well, 
your FTP server should start filling up with porn logins and boring emails. Sweet! 

	Optional: You may pack or compress the output to reduce the file size. I was able to get 
it down from ~103KB to ~40KB with ASPack. If you decide to pack the output, make sure to 
enable 'preserve extra data' in whatever packer you choose.


----------------------------------------------------------------------------------------- 
0x03 FTP Information 
----------------------------------------------------------------------------------------- 
	Make sure the FTP information in LogThatShit.ini is correct. The port is 21 and it 
can NOT be changed. I recommend using a static DNS for the FTP host. That way if that 
FTP changes or goes down, you can re-direct it to another FTP host (obviously the login 
and root dir would have to be the same.) It is highly recommended that you enable the 
'FTPTEST' variable under 'BUILD' in the INI file. 

	*PLEAE NOTE* The FTP test will only test out the connection. It will not 
upload/download/delete/rename anything. Therefore it doesn't check for write permission. 


----------------------------------------------------------------------------------------- 
0x04 Config Variable List (The default value is in paranthesis.) 
----------------------------------------------------------------------------------------- 
	[BUILD] 
		These values generally do NOT need to be changed. 

		* OUTPUT (Output.exe):Name of the generated output file. 
		* STUB (Stub.dll): Should point to the LTS stub file. 
		* INJECTION (Injection.dll): Should point to the LTS Injection.dll file. 
		* HOOK (Hook.dll): Should point to the LTS Hook.dll file. 
		* FTPTEST (1): If value is 1, FTP settings will be tested before building. 

	[GENERAL] 
		It is recommended that you change these! 

		* ALIAS (Stonecold): Alias to give the victim. Logfiles uploaded to the FTP will 
			contain this alias. Give each victim a different alias so that the files 
			won't collide on the FTP. Only alphanumeric characters here. No slashes! 
		* PATH (C:\Windows\System32): Path where the DLLs and logs will reside. The 
			files will be hidden. 
		* FILENAME (evil.exe): We will be copied to this name in PATH upon execution. 
		* LOGNAME (blog.log): Name the log file will go by inside PATH. Can be any extension. 
		* INJECTNAME (solution.dll): Name the injection DLL will go by inside PATH. 
		* HOOKNAME (hooker.dll): Name the hook DLL will go by inside PATH. 

	[REMOTE] 
			These files must be uploaded to the FTP root specified in [FTP]. LTS will look 
		for these files everytime it uploads a log. 

			If the file is handled successfully, it will be renamed to "name.ran" on the FTP, 
		in case the file is NOT handled correctly, it is corrupt or unable to be downloaded, 
		it will be renamed to "name.err" 

			*READ* If the FTP fails to upload the log file to the FTP dir in [FTP], the ROOT 
		FTP DIR IS USED. Here is an example. Assume the REMOVE file is called 'remove.plz' and 
		the FTP dir you put in [FTP] is /www/lost/dir/ 

			LTS uploads a log. If the upload was successful, it looks in the same dir 
		(/www/lost/dir) for remove.plz. If it finds it, it will uninstall itself.  Now, if the 
		log upload was NOT successful, it will attempt uploading the log to the root ftp dir 
		("/"). If THAT is successful, it will look in / for remove.plz  

		* EXEC (lts_exec.exe): File will be downloaded and execute on the victim's box. 
			Please note that the file will be executed normally, no special attributes 
			will be used. It will NOT be hidden. 
		* REMOVE (lts_remove.me): LTS will be uninstalled from the victim's box when 
			this file is found on the FTP. Can be any valid name. 
		* UPDATE (lts_update.exe): This has to be an LTS executable. If the file is valid, 
			it will replace the current LTS on the victim's box. Takes affect AFTER the 
			box reboots. 

	[LOG] 
			Usually only the first two variables need to be changed. The rest specify what 
		gets logged. Turning some/all off will be more efficient, but this is not an issue 
		on any modern day machine. 

		* MAXLOGSIZE (5000): Specify the max size - in bytes - that a log can grow to 
			before it is uploaded to the FTP. Default usually suffices, unless 
			the victim plays a first person shooter. 
		* LOGCHECKINT (30): Interval - in seconds - for checking the log file size to see 
			if it reached the above size. Do not set this too low. 30+ work well.
		* LOGTITLE (1): If set, log file will contain title of the current window that 
			the user is typing in. 
		* LOGTITLECHANGE (1): If set, an asterisk (*) will be appended to the title in the 
			log file indicating that the title for that window has changed. 
		* LOGPATH (1): Log path of the program that the user is currently typing in. 
		* LOGTIMESTAMP (1): Log time that the user started typing in a new window. 

	[FTP] 
			Be sure to read 'FTP Information' and it is recommended that you enable FTPTEST 
		under [BUILD]. 

		* ROOT (/): Directory on the FTP where the log files will be uploaded to. If 
			uploading to this directory fails, the root FTP dir will be tried ("/"). 
			This is also the directory where the REMOTE files will be looked for. 
		* HOST (ftp.dns.or.ip): The FTP host. It can be either a DNS name or an
			IP address. Do NOT append the port! (127.0.0.1:2121 is INVALID) 
		* USER (user): Your login username for the above FTP. 
		* PASS (secret): Your login password for the above FTP. 

	[MISC] 
			Upon execution, NOTHING will visibly happen. This can be very suspicious to 
		the victim. Enabling the fake error message below would probably be a good idea 
		unless you are binding this with something (a better idea.) 

		* FAKEERROR (0): If enabled, a fake error message box will be displayed when the 
			victim runs the file. The box will have an error icon. 
		* ERRMSG (...) : Message body for the error box. 
		* ERRCAP (...) : Caption of the error message box. 


----------------------------------------------------------------------------------------- 
0x05 Gr33tz 
----------------------------------------------------------------------------------------- 
	Thanks and Greets go out to Aadil, Sonikku`, p0ke, and Zatrix. I have to thank Ghirai 
for releasing Fearless Key Spy. I couldn't find the code to bring his logger into 
the 21st century, so I decided to just write my own from scratch. 

	Also thanks to the beta testers, Fransisco Brito and of course, all of you guys who 
beta tested unknowingly. ;) 

