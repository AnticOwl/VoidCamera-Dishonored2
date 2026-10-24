from pathlib import Path
import re, shutil
root=Path(__file__).resolve().parents[1]
src=root/'src'; src.mkdir(exist_ok=True)
lua=(root/'research/Dishonored2_Camera_v0.6.lua').read_text(encoding='utf-8-sig')
asm=lua.split('D2V06Mem+1000:\n',1)[1].split('\n]==]',1)[0]
asm=asm[:asm.index('  cmp byte ptr [Dishonored2.exe+39A529C],r12b')]
# CE AutoAssembler numbers are hexadecimal; MASM numbers need explicit suffixes.
asm=re.sub(r'(?<![A-Za-z_0-9])([0-9][0-9A-F]*|[A-F][0-9A-F]*)(?![A-Za-z_0-9])',lambda m:'0'+m[1]+'h',asm)
asm=re.sub(r'//.*','',asm).replace('D2V06Mem','cameraData')
asm=asm.replace('[cameraData+08h],rdi','qword ptr [cameraData+08h],rdi')
asm=asm.replace('[cameraData+02Ch],edx','dword ptr [cameraData+02Ch],edx')
asm=re.sub(r'(mov (?:e\w+),)(\[cameraData[^\]]*\])',r'\1dword ptr \2',asm)
asm=re.sub(r'(mov (?:r\w+),)(\[cameraData[^\]]*\])',r'\1qword ptr \2',asm)
asm=re.sub(r'mov (\[cameraData[^\]]*\]),(e\w+)',r'mov dword ptr \1,\2',asm)
asm=re.sub(r'mov (\[cameraData[^\]]*\]),(r\w+)',r'mov qword ptr \1,\2',asm)
asm=asm.replace('cmp ecx,[cameraData','cmp ecx,dword ptr [cameraData')
(src/'camera_hook.asm').write_text('option casemap:none\nEXTERN cameraData:BYTE\nEXTERN cameraOriginal:QWORD\n.code\ncameraHook PROC\n'+asm+'  jmp qword ptr [cameraOriginal]\ncameraHook ENDP\nEND\n')
for name in ('imgui','minhook'):
    dest=root/'third_party'/name
    if not dest.exists(): shutil.copytree(root.parent/'DXMD_Freecam/third_party'/name,dest)
