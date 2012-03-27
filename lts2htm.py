#!/usr/bin/python
# Convert an LTS log file to HTML
# Add color and all that pretty shit. 
# Repeat rate.. convert aaaa to a (x4) 


import os 
import sys 


# ----------------------------------------------------------
# Change these to suit your needs. 
# ----------------------------------------------------------
FormatHeader = True 
FormatSubHeaders = True 
ColorSpecialKeys = True 
OmitShiftKey = True 
FormatBackspaceKey = True 


SubColor = '#666666' 
Colors = { \
		'[Backspace]' : '#CC0000', \
		'[Tab]' : '#FF6633', \
		'[Enter]' : '#0066CC', \
		'[Right Shift]' : '#666666', \
		'[Shift]' : '#666666', \
		'[Ctrl]' : '#999999', \
		'[Alt]' : '#006699', \
		'[Right Alt]' : '#006699', \
		'[Esc]' : '#990000', \

		'[F1]' : '#009900', \
		'[F2]' : '#009900', \
		'[F3]' : '#009900', \
		'[F4]' : '#009900', \
		'[F5]' : '#009900', \
		'[F6]' : '#009900', \
		'[F7]' : '#009900', \
		'[F8]' : '#009900', \
		'[F9]' : '#009900', \
		'[F10]' : '#009900', \
		'[F11]' : '#009900', \
		'[F12]' : '#009900', \

		'[Up]' : '#666666', \
		'[Down]' : '#666666', \
		'[Left]' : '#666666', \
		'[Right]' : '#666666', \

		'[Caps Lock]' : '#00CC00', \
		'[Num Lock]' : '#00CC00', \
		'[Pause]' : '#FF9933', \
		'[Insert]' : '#FF9933', \
		'[Home]' : '#FF9933', \
		'[Page Up]' : '#FF9933', \
		'[Delete]' : '#FF3333', \
		'[End]' : '#FF9933', \
		'[Page Down]' : '#FF9933', \

		'[Left Windows]' : '#666666', \
		'[Right Windows]' : '#666666', \
		'[Application]' : '#666666', \

		'[Num 0]' : '#00CC00', \
		'[Num 1]' : '#00CC00', \
		'[Num 2]' : '#00CC00', \
		'[Num 3]' : '#00CC00', \
		'[Num 4]' : '#00CC00', \
		'[Num 5]' : '#00CC00', \
		'[Num 6]' : '#00CC00', \
		'[Num 7]' : '#00CC00', \
		'[Num 8]' : '#00CC00', \
		'[Num 9]' : '#00CC00', \
		'[Num Enter]' : '#00CC00' 
} 
# ----------------------------------------------------------


def FilterBackspaceKey(data): 
	return data 
#	while 1: 
#		index = data.find('[Backspace]') 
#		if -1==index: break 
#		if index==0: break 
#		data = data[0:index-1] + data[index+len('[Backspace]'):] 
#	return data 


def FilterShiftKey(data): 
	"""Remove [Shift], [Right Shift], etc..""" 
	data = data.replace('[Shift]', '') 
	data = data.replace('[Right Shift]', '') 
	data = data.replace('[Left Shift]', '') 
	return data 


def Colorize(data): 
	"""Color data using the Colors dictionary and return it.""" 
	colored = data 
	for key,color in Colors.items(): 
		colored = colored.replace(key, '<font color="%s">%s</font>' % (color, key)) 
	return colored 


if __name__=='__main__':
	# Get file names. 
	if 0: 
		if len(sys.argv) < 2: 
			print '*** ERROR: Missing parameter(s)' 
			print 'Usage: lts2htm.py bob_234.log' 
			sys.exit(1) 

		arg = sys.argv[1] 
		logfile=arg 
		out = arg + '.htm' 
	logfile = 'LOG2.log' 
	out = 'LOG.htm' 
	# Load data into memory. 
	log = file(logfile, 'r') 
	formatted = '' 
	line = '' 

	# Examine the log line by line. 
	while 1: 
		# Get the next line. 
		line = log.readline() 
		if not line: break 

		# Skip blank lines. 
		if '\n'==line[0]: continue; 

		# Remove the appending '\n' at the end of a line. 
		line = line[:-1] 

		# Subheaders start with '('. If user types '(', it would be '[Shift](' 
		if '('==line[0]: 
			formatted += '<b><font color="%s">%s</font></b><br />\n' % (SubColor, line) 
			continue 

		# See if this is the header. If so, format it. Else just add it to data. 
		if line.startswith('***Logging started @') and FormatHeader: 
			formatted += '<h3>%s</h3>\n' % line 
			continue 

		# The line contains key strokes. Optionally format it and then output it. 
#		if_tttpressSpecialKeys: fline =_tttpressSpecialKeys(fline) 
		if OmitShiftKey: line = FilterShiftKey(line) 
		if FormatBackspaceKey: line = FilterBackspaceKey(line) 

		# We do not want to scroll to read the text. So let's break this up into 
		# multiple lines, assuming it's line. 
		N = 130 
		A = line 
		line = '' 
		while len(A) > N: 
			# Append a new line after 100 chars 
			line += A[:N] + '<br />' 
			A = A[N:] 
		line = line + A 

		if ColorSpecialKeys: line = Colorize(line) 
		formatted += line + '<br /><br />\n\n' 
	log.close() 

	# Output data 
	htm = file(out, 'w') 
	htm.write('<!-- LogThatShit log to HTML converter\n -->\n\n') 
#	htm.write('<pre>') 
	htm.write(formatted) 
#	htm.write('</pre>') 
	htm.close() 
	print '"%s" has been generated.' % out 






