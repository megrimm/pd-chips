import os,pyext

def shell(*args):
	s = " ".join(map(str,args))
	f = os.popen(s)
	r = f.read().split()
	f.close()
	return r
	
class shellx(pyext._class):
    def _anything_1(self,*args):
        s = " ".join(map(str,args))
    	f = os.popen(s)
    	for fi in f.readlines():
    	    self._outlet(1,fi.split())
        f.close()
        self._outlet(2)
