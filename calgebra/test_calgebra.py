# -*- coding: utf-8 -*-
import subprocess

input="fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }\n"

i=15
for a in range(i,29):
	input += "fib(%d)\n" % (a)
p = subprocess.Popen(["calgebra", "--calculate", "--print-time"], shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

output=p.communicate(input)
for line in output[1].split('\n'):
	theLine = line.split('time:')
	if len(theLine)>1:
		#print str(i)+", "+theLine[1]
		print theLine[1]
		i+=1

