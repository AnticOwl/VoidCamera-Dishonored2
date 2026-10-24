option casemap:none
EXTERN cameraData:BYTE
EXTERN cameraOriginal:QWORD
.code
cameraHook PROC
  push rax
  push rcx
  push rdx
  push r8
  sub rsp,040h
  cmp dword ptr [cameraData],01h
  jne D2V06Capture
  cmp qword ptr [cameraData+08h],rdi
  jne D2V06Finish
D2V06Capture:
  inc dword ptr [cameraData+020h]
  mov qword ptr [cameraData+010h],rdi
  mov dword ptr [cameraData+018h],ebx
  mov dword ptr [cameraData+01Ch],esi
  mov eax,[rdi+03A6Ch]
  mov dword ptr [cameraData+040h],eax
  mov eax,[rdi+03A70h]
  mov dword ptr [cameraData+044h],eax
  mov eax,[rdi+03A78h]
  mov dword ptr [cameraData+048h],eax
  mov eax,[rdi+03A7Ch]
  mov dword ptr [cameraData+04Ch],eax
  mov eax,[rdi+03A80h]
  mov dword ptr [cameraData+050h],eax
  mov eax,[rdi+03A9Ch]
  mov dword ptr [cameraData+054h],eax
  mov eax,[rdi+03AA0h]
  mov dword ptr [cameraData+058h],eax
  mov eax,[rdi+03AA4h]
  mov dword ptr [cameraData+05Ch],eax
  mov eax,[rdi+03AA8h]
  mov dword ptr [cameraData+060h],eax
  mov eax,[rdi+03AACh]
  mov dword ptr [cameraData+064h],eax
  mov eax,[rdi+03AB0h]
  mov dword ptr [cameraData+068h],eax
  mov eax,[rdi+03AB4h]
  mov dword ptr [cameraData+06Ch],eax
  mov eax,[rdi+03AB8h]
  mov dword ptr [cameraData+070h],eax
  mov eax,[rdi+03ABCh]
  mov dword ptr [cameraData+074h],eax
  inc dword ptr [cameraData+024h]
  inc dword ptr [cameraData+020h]
  cmp dword ptr [cameraData],01h
  jne D2V06Finish
  
  
D2V06TryLock:
  xor eax,eax
  mov edx,01h
  lock cmpxchg dword ptr [cameraData+02Ch],edx
  je D2V06Locked
  pause
  cmp dword ptr [cameraData],01h
  jne D2V06Finish
  jmp D2V06TryLock
D2V06Locked:
  cmp dword ptr [cameraData],01h
  jne D2V06Unlock
  cmp qword ptr [cameraData+08h],rdi
  jne D2V06Unlock
  mov ecx,dword ptr [cameraData+04h]
  test ecx,01h
  jne D2V06UseCached
  mov rax,qword ptr [cameraData+080h]
  mov [rsp+00h],rax
  mov rax,qword ptr [cameraData+088h]
  mov [rsp+08h],rax
  mov rax,qword ptr [cameraData+090h]
  mov [rsp+010h],rax
  mov rax,qword ptr [cameraData+098h]
  mov [rsp+018h],rax
  mov rax,qword ptr [cameraData+0A0h]
  mov [rsp+020h],rax
  mov rax,qword ptr [cameraData+0A8h]
  mov [rsp+028h],rax
  mov rax,qword ptr [cameraData+0B0h]
  mov [rsp+030h],rax
  cmp ecx,dword ptr [cameraData+04h]
  jne D2V06UseCached
  
  mov rax,[rsp+00h]
  mov qword ptr [cameraData+0200h],rax
  mov rax,[rsp+08h]
  mov qword ptr [cameraData+0208h],rax
  mov rax,[rsp+010h]
  mov qword ptr [cameraData+0210h],rax
  mov rax,[rsp+018h]
  mov qword ptr [cameraData+0218h],rax
  mov rax,[rsp+020h]
  mov qword ptr [cameraData+0220h],rax
  mov rax,[rsp+028h]
  mov qword ptr [cameraData+0228h],rax
  mov rax,[rsp+030h]
  mov qword ptr [cameraData+0230h],rax
  mov dword ptr [cameraData+038h],ecx
  inc dword ptr [cameraData+034h]
  jmp D2V06Apply
D2V06UseCached:
  
  
  inc dword ptr [cameraData+030h]
  mov rax,qword ptr [cameraData+0200h]
  mov [rsp+00h],rax
  mov rax,qword ptr [cameraData+0208h]
  mov [rsp+08h],rax
  mov rax,qword ptr [cameraData+0210h]
  mov [rsp+010h],rax
  mov rax,qword ptr [cameraData+0218h]
  mov [rsp+018h],rax
  mov rax,qword ptr [cameraData+0220h]
  mov [rsp+020h],rax
  mov rax,qword ptr [cameraData+0228h]
  mov [rsp+028h],rax
  mov rax,qword ptr [cameraData+0230h]
  mov [rsp+030h],rax
D2V06Apply:
  cmp dword ptr [cameraData],01h
  jne D2V06Unlock
  mov eax,[rsp+00h]
  mov [rdi+03A6Ch],eax
  mov [rdi+09Ch],eax
  mov eax,[rsp+04h]
  mov [rdi+03A70h],eax
  mov [rdi+0A0h],eax
  mov eax,[rsp+08h]
  mov [rdi+03A78h],eax
  mov [rdi+0A8h],eax
  mov eax,[rsp+0Ch]
  mov [rdi+03A7Ch],eax
  mov [rdi+0ACh],eax
  mov eax,[rsp+010h]
  mov [rdi+03A80h],eax
  mov [rdi+0B0h],eax
  mov eax,[rsp+014h]
  mov [rdi+03A9Ch],eax
  mov [rdi+0CCh],eax
  mov eax,[rsp+018h]
  mov [rdi+03AA0h],eax
  mov [rdi+0D0h],eax
  mov eax,[rsp+01Ch]
  mov [rdi+03AA4h],eax
  mov [rdi+0D4h],eax
  mov eax,[rsp+020h]
  mov [rdi+03AA8h],eax
  mov [rdi+0D8h],eax
  mov eax,[rsp+024h]
  mov [rdi+03AACh],eax
  mov [rdi+0DCh],eax
  mov eax,[rsp+028h]
  mov [rdi+03AB0h],eax
  mov [rdi+0E0h],eax
  mov eax,[rsp+02Ch]
  mov [rdi+03AB4h],eax
  mov [rdi+0E4h],eax
  mov eax,[rsp+030h]
  mov [rdi+03AB8h],eax
  mov [rdi+0E8h],eax
  mov eax,[rsp+034h]
  mov [rdi+03ABCh],eax
  mov [rdi+0ECh],eax
  inc dword ptr [cameraData+028h]
D2V06Unlock:
  mov dword ptr [cameraData+02Ch],00h
D2V06Finish:
  add rsp,040h
  pop r8
  pop rdx
  pop rcx
  pop rax
  jmp qword ptr [cameraOriginal]
cameraHook ENDP
END
