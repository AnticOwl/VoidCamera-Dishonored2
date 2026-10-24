"""Read-only, targeted inspection of the documented camera sites."""
import ctypes as c, ctypes.wintypes as w, json
k=c.WinDLL('kernel32',use_last_error=True)
class PE(c.Structure):
    _fields_=[('size',w.DWORD),('use',w.DWORD),('pid',w.DWORD),('heap',c.c_size_t),('module',w.DWORD),('threads',w.DWORD),('parent',w.DWORD),('priority',w.LONG),('flags',w.DWORD),('exe',w.WCHAR*260)]
class ME(c.Structure):
    _fields_=[('size',w.DWORD),('module',w.DWORD),('pid',w.DWORD),('globalUse',w.DWORD),('use',w.DWORD),('base',c.c_void_p),('bytes',w.DWORD),('hmod',w.HMODULE),('name',w.WCHAR*256),('path',w.WCHAR*260)]
k.CreateToolhelp32Snapshot.restype=w.HANDLE
k.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD];k.OpenProcess.restype=w.HANDLE
k.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)]
k.CloseHandle.argtypes=[w.HANDLE]
for fn,typ in [('Process32FirstW',PE),('Process32NextW',PE),('Module32FirstW',ME),('Module32NextW',ME)]:getattr(k,fn).argtypes=[w.HANDLE,c.POINTER(typ)]
snap=k.CreateToolhelp32Snapshot(2,0);e=PE();e.size=c.sizeof(e);ok=k.Process32FirstW(snap,c.byref(e));pids=[]
while ok:
    if e.exe.lower()=='dishonored2.exe':pids.append(e.pid)
    ok=k.Process32NextW(snap,c.byref(e))
k.CloseHandle(snap)
assert len(pids)==1,pids
pid=pids[0];snap=k.CreateToolhelp32Snapshot(8,pid);m=ME();m.size=c.sizeof(m);ok=k.Module32FirstW(snap,c.byref(m));mods=[];base=None
while ok:
    if m.name.lower()=='dishonored2.exe':base=m.base
    if any(t in m.name.lower() for t in ['camera','photo','igcs','dinput','reshade']):mods.append(m.path)
    ok=k.Module32NextW(snap,c.byref(m))
k.CloseHandle(snap);assert base
p=k.OpenProcess(0x1010,False,pid);assert p,c.get_last_error()
def read(a,n):
    buf=c.create_string_buffer(n);done=c.c_size_t()
    assert k.ReadProcessMemory(p,a,buf,n,c.byref(done)) and done.value==n,c.get_last_error()
    return buf.raw
out={'pid':pid,'base':hex(base),'modules':mods}
for rva,n in [(0x375711,7),(0x3757b5,8),(0x3758d6,8),(0x39a529c,1)]:out[hex(rva)]=read(base+rva,n).hex(' ')
b=read(base+0x375711,7)
if b[0]==0xe9:
    dest=base+0x375716+int.from_bytes(b[1:5],'little',signed=True)
    out['existing_hook_target']=hex(dest);out['hook_head']=read(dest,32).hex(' ')
print(json.dumps(out,indent=2));k.CloseHandle(p)
