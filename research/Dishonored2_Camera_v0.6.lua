-- Dishonored 2 retail 1.77.9.0 -- Camera core v0.6 -- Cheat Engine 7.6 x64
-- Exact EXE SHA256: b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180
-- Direct render-view override. NO g_freeCam toggle; NO Ansel session;
-- NO resolution-scale writes; NO debugger/breakpoints; NO post-process.
-- v0.6: keep the last COMPLETE pose when Lua is publishing a new pose.
-- v0.5 returned to the game on an odd/changed sequence: visible player-view flashes.
-- The hook now reuses its committed cache instead of skipping the override.
-- Movement/Pitch/Yaw/Roll, hook location and view fields are unchanged from v0.5.
-- Camera-only CE prototype; not an ImGui/DLL release.
-- Hook instructions and view layout independently traced in the supplied EXE.
-- Corrected CVar addresses cross-checked with VoidPhotoMode's public source.
-- All original camera writes run first; we override only the selected view.
-- Disable restores the original instruction bytes; the small code allocation
-- stays mapped until process exit so an in-flight hook cannot jump into freed memory.
local C = {version='0.6', active=false, running=false, sequence=0, hotkeys={}}
for _,name in ipairs({'D2CameraV06','D2CameraV05','D2CameraV04'}) do
  local old=rawget(_G,name)
  if old and old.stop then
    if old.stop()==false then error('[D2 v06] Ancien hook non restaure; ne pas superposer une nouvelle version.',0) end
  end
end
-- Earlier probes used invalid addresses: remove their hotkeys WITHOUT calling
-- their stop() function, which would write back to those invalid addresses.
local oldProbe=_G.D2CamProbe
if oldProbe then
  if oldProbe.hotkeys then
    for _,h in pairs(oldProbe.hotkeys) do pcall(function() h.destroy() end) end
  end
  if oldProbe.timer then pcall(function() oldProbe.timer.destroy() end) end
  oldProbe.enabled=false
  _G.D2CamProbe=nil
end
local logs={}
local logfile=(os.getenv('TEMP') or os.getenv('TMP') or '.')..'\\Dishonored2_Camera_v06_log.txt'
local function emit(s)
  local t='[D2 v06] '..s
  print(t);logs[#logs+1]=t
  local f=io.open(logfile,'w')
  if f then f:write(table.concat(logs,'\n'));f:close() end
end
local function ensure(v,msg) if not v then error('[D2 v06] '..msg,0) end;return v end
local function bytes(a,n)
  local b=readBytes(a,n,true)
  ensure(b and #b==n,string.format('Lecture impossible @ %X',a))
  return b
end
local function equal(a,b)
  if not a or #a~=#b then return false end
  for i=1,#a do if a[i]~=b[i] then return false end end
  return true
end
local function expect(a,b,name)
  ensure(equal(bytes(a,#b),b),'Signature incorrecte: '..name..'. Aucun hook installe.')
end
local base=getAddressSafe('Dishonored2.exe')
if not base then
  pcall(openProcess,'Dishonored2.exe')
  if reinitializeSymbolhandler then pcall(reinitializeSymbolhandler) end
  base=getAddressSafe('Dishonored2.exe')
end
ensure(base,'Lance Dishonored2.exe, puis reactive la table.')
C.base=base;C.pid=getOpenedProcessID()
ensure(C.pid and C.pid~=0,'Attachement au jeu impossible.')
expect(base,{0x4D,0x5A},'MZ')
local pe=base+ensure(readInteger(base+0x3C),'PE absent')
expect(pe,{0x50,0x45,0,0},'PE')
ensure(readSmallInteger(pe+4)==0x8664,'Cette table exige le jeu x64.')
ensure(readInteger(pe+8)==0x67A50CDE,'Timestamp different du binaire fourni.')
ensure(readInteger(pe+24+56)==0x41CB000,'SizeOfImage different du binaire fourni.')
C.hook=base+0x375711
C.original={0x44,0x38,0x25,0x84,0xFB,0x62,0x03}
expect(C.hook,C.original,'vue camera +375711')
expect(base+0x3757B5,{0xF3,0x0F,0x10,0x87,0x70,0x3A,0,0},'FOV source')
expect(base+0x3758D6,{0xF3,0x0F,0x11,0x87,0x78,0x3A,0,0},'XYZ source')
-- Static idCVar objects: name pointer +30; cached int +28 and float +2C.
C.cvars={
 {name='g_freeCam',object=0x39A1790},
 {name='g_stopTime',object=0x39A0E10},
 {name='g_freezeTime',object=0x39A60E0},
}
for _,v in ipairs(C.cvars) do
  local a=base+v.object
  local np=readQword(a+0x30)
  ensure(np and np>=base and np<base+0x41CB000,'Nom CVar invalide: '..v.name)
  ensure(readString(np,#v.name+1,false)==v.name,'Identite CVar incorrecte: '..v.name)
  emit(string.format('%s : objet +%X / entier +%X = %s [lecture seule]',
       v.name,v.object,v.object+0x28,tostring(readInteger(a+0x28))))
end
ensure(readBytes(base+0x39A529C,1)==0,'Ferme la session Ansel/photo mode deja active avant ce test.')
local assembly=[==[
alloc(D2V06Mem,$4000,Dishonored2.exe+375711)
registersymbol(D2V06Mem)
label(D2V06Capture)
label(D2V06TryLock)
label(D2V06Locked)
label(D2V06UseCached)
label(D2V06Apply)
label(D2V06Unlock)
label(D2V06Finish)
D2V06Mem+1000:
  push rax
  push rcx
  push rdx
  push r8
  sub rsp,40
  cmp dword ptr [D2V06Mem],1
  jne D2V06Capture
  cmp [D2V06Mem+8],rdi
  jne D2V06Finish
D2V06Capture:
  inc dword ptr [D2V06Mem+20]
  mov [D2V06Mem+10],rdi
  mov [D2V06Mem+18],ebx
  mov [D2V06Mem+1C],esi
  mov eax,[rdi+3A6C]
  mov [D2V06Mem+40],eax
  mov eax,[rdi+3A70]
  mov [D2V06Mem+44],eax
  mov eax,[rdi+3A78]
  mov [D2V06Mem+48],eax
  mov eax,[rdi+3A7C]
  mov [D2V06Mem+4C],eax
  mov eax,[rdi+3A80]
  mov [D2V06Mem+50],eax
  mov eax,[rdi+3A9C]
  mov [D2V06Mem+54],eax
  mov eax,[rdi+3AA0]
  mov [D2V06Mem+58],eax
  mov eax,[rdi+3AA4]
  mov [D2V06Mem+5C],eax
  mov eax,[rdi+3AA8]
  mov [D2V06Mem+60],eax
  mov eax,[rdi+3AAC]
  mov [D2V06Mem+64],eax
  mov eax,[rdi+3AB0]
  mov [D2V06Mem+68],eax
  mov eax,[rdi+3AB4]
  mov [D2V06Mem+6C],eax
  mov eax,[rdi+3AB8]
  mov [D2V06Mem+70],eax
  mov eax,[rdi+3ABC]
  mov [D2V06Mem+74],eax
  inc dword ptr [D2V06Mem+24]
  inc dword ptr [D2V06Mem+20]
  cmp dword ptr [D2V06Mem],1
  jne D2V06Finish
  // Only engine hook invocations use this lock. Lua NEVER holds it.
  // No calls, sleeps, or waits for a CE update in the locked section.
D2V06TryLock:
  xor eax,eax
  mov edx,1
  lock cmpxchg [D2V06Mem+2C],edx
  je D2V06Locked
  pause
  cmp dword ptr [D2V06Mem],1
  jne D2V06Finish
  jmp D2V06TryLock
D2V06Locked:
  cmp dword ptr [D2V06Mem],1
  jne D2V06Unlock
  cmp [D2V06Mem+8],rdi
  jne D2V06Unlock
  mov ecx,[D2V06Mem+4]
  test ecx,1
  jne D2V06UseCached
  mov rax,[D2V06Mem+80]
  mov [rsp+0],rax
  mov rax,[D2V06Mem+88]
  mov [rsp+8],rax
  mov rax,[D2V06Mem+90]
  mov [rsp+10],rax
  mov rax,[D2V06Mem+98]
  mov [rsp+18],rax
  mov rax,[D2V06Mem+A0]
  mov [rsp+20],rax
  mov rax,[D2V06Mem+A8]
  mov [rsp+28],rax
  mov rax,[D2V06Mem+B0]
  mov [rsp+30],rax
  cmp ecx,[D2V06Mem+4]
  jne D2V06UseCached
  // The staged pose was coherent: commit it before using it for rendering.
  mov rax,[rsp+0]
  mov [D2V06Mem+200],rax
  mov rax,[rsp+8]
  mov [D2V06Mem+208],rax
  mov rax,[rsp+10]
  mov [D2V06Mem+210],rax
  mov rax,[rsp+18]
  mov [D2V06Mem+218],rax
  mov rax,[rsp+20]
  mov [D2V06Mem+220],rax
  mov rax,[rsp+28]
  mov [D2V06Mem+228],rax
  mov rax,[rsp+30]
  mov [D2V06Mem+230],rax
  mov [D2V06Mem+38],ecx
  inc dword ptr [D2V06Mem+34]
  jmp D2V06Apply
D2V06UseCached:
  // Publication was busy or changed mid-copy. Keep the last complete pose.
  // Cache is seeded BEFORE enabling the freecam, so it is valid on frame one.
  inc dword ptr [D2V06Mem+30]
  mov rax,[D2V06Mem+200]
  mov [rsp+0],rax
  mov rax,[D2V06Mem+208]
  mov [rsp+8],rax
  mov rax,[D2V06Mem+210]
  mov [rsp+10],rax
  mov rax,[D2V06Mem+218]
  mov [rsp+18],rax
  mov rax,[D2V06Mem+220]
  mov [rsp+20],rax
  mov rax,[D2V06Mem+228]
  mov [rsp+28],rax
  mov rax,[D2V06Mem+230]
  mov [rsp+30],rax
D2V06Apply:
  cmp dword ptr [D2V06Mem],1
  jne D2V06Unlock
  mov eax,[rsp+0]
  mov [rdi+3A6C],eax
  mov [rdi+9C],eax
  mov eax,[rsp+4]
  mov [rdi+3A70],eax
  mov [rdi+A0],eax
  mov eax,[rsp+8]
  mov [rdi+3A78],eax
  mov [rdi+A8],eax
  mov eax,[rsp+C]
  mov [rdi+3A7C],eax
  mov [rdi+AC],eax
  mov eax,[rsp+10]
  mov [rdi+3A80],eax
  mov [rdi+B0],eax
  mov eax,[rsp+14]
  mov [rdi+3A9C],eax
  mov [rdi+CC],eax
  mov eax,[rsp+18]
  mov [rdi+3AA0],eax
  mov [rdi+D0],eax
  mov eax,[rsp+1C]
  mov [rdi+3AA4],eax
  mov [rdi+D4],eax
  mov eax,[rsp+20]
  mov [rdi+3AA8],eax
  mov [rdi+D8],eax
  mov eax,[rsp+24]
  mov [rdi+3AAC],eax
  mov [rdi+DC],eax
  mov eax,[rsp+28]
  mov [rdi+3AB0],eax
  mov [rdi+E0],eax
  mov eax,[rsp+2C]
  mov [rdi+3AB4],eax
  mov [rdi+E4],eax
  mov eax,[rsp+30]
  mov [rdi+3AB8],eax
  mov [rdi+E8],eax
  mov eax,[rsp+34]
  mov [rdi+3ABC],eax
  mov [rdi+EC],eax
  inc dword ptr [D2V06Mem+28]
D2V06Unlock:
  mov dword ptr [D2V06Mem+2C],0
D2V06Finish:
  add rsp,40
  pop r8
  pop rdx
  pop rcx
  pop rax
  cmp byte ptr [Dishonored2.exe+39A529C],r12b
  jmp Dishonored2.exe+375718

]==]
local ok,detail=autoAssemble(assembly)
ensure(ok,'Assemblage refuse: '..tostring(detail))
C.mem=ensure(getAddressSafe('D2V06Mem'),'Allocation D2V06Mem introuvable.')
local m=C.mem
local blank={};for i=1,0x300 do blank[i]=0 end
ensure(writeBytes(m,blank),'Initialisation memoire impossible.')
local disp=(m+0x1000)-(C.hook+5)
ensure(disp>=-0x80000000 and disp<=0x7FFFFFFF,'Allocation trop loin du hook; aucun patch pose.')
local u=disp%0x100000000
C.patch={0xE9,u%256,math.floor(u/256)%256,math.floor(u/65536)%256,math.floor(u/16777216)%256,0x90,0x90}
local function putFloat(a,v) ensure(writeFloat(a,v),'Ecriture float refusee.') end
local function finite(x) return type(x)=='number' and x==x and math.abs(x)<math.huge end
local function clamp(x,a,b) return math.max(a,math.min(b,x)) end
local pi=math.pi
local function rad(d) return d*pi/180 end
local function deg(r) return r*180/pi end
local function qnorm(q)
  local n=math.sqrt(q[1]^2+q[2]^2+q[3]^2+q[4]^2)
  ensure(n>0.000001,'Quaternion nul.')
  return {q[1]/n,q[2]/n,q[3]/n,q[4]/n}
end
local function qmul(a,b)
  local x,y,z,w=a[1],a[2],a[3],a[4]
  local X,Y,Z,W=b[1],b[2],b[3],b[4]
  return {w*X+x*W+y*Z-z*Y,w*Y-x*Z+y*W+z*X,w*Z+x*Y-y*X+z*W,w*W-x*X-y*Y-z*Z}
end
local function axis(which,r)
  local t={0,0,0,math.cos(r/2)};t[which]=math.sin(r/2);return t
end
local function matrix(q)
  local x,y,z,w=q[1],q[2],q[3],q[4]
  return {1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w),
          2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w),
          2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)}
end
-- In the captured native block, each consecutive triple is a world-space
-- camera axis: Forward, Left, Up. matrix(q) below uses these axes as COLUMNS.
-- The EXE transposes the +111410 quaternion-matrix output at +375930..+375977.
-- Keep one camera-to-world quaternion internally; convert only at the boundary.
local function transpose3(r)
  return {r[1],r[4],r[7],r[2],r[5],r[8],r[3],r[6],r[9]}
end
local function fromMatrix(r)
  local t=r[1]+r[5]+r[9];local q
  if t>0 then
    local s=math.sqrt(t+1)*2;q={(r[8]-r[6])/s,(r[3]-r[7])/s,(r[4]-r[2])/s,s/4}
  elseif r[1]>r[5] and r[1]>r[9] then
    local s=math.sqrt(1+r[1]-r[5]-r[9])*2;q={s/4,(r[2]+r[4])/s,(r[3]+r[7])/s,(r[8]-r[6])/s}
  elseif r[5]>r[9] then
    local s=math.sqrt(1+r[5]-r[1]-r[9])*2;q={(r[2]+r[4])/s,s/4,(r[6]+r[8])/s,(r[3]-r[7])/s}
  else
    local s=math.sqrt(1+r[9]-r[1]-r[5])*2;q={(r[3]+r[7])/s,(r[6]+r[8])/s,s/4,(r[4]-r[2])/s}
  end
  return qnorm(q)
end
local function fromNativeMatrix(r)
  return fromMatrix(transpose3(r))
end
local function toNativeMatrix(q)
  return transpose3(matrix(q))
end
local function eulers(q)
  local r=matrix(q);local p=math.asin(clamp(-r[7],-1,1));local y,roll
  if math.abs(math.cos(p))>0.00001 then
    y=math.atan(r[4],r[1]);roll=math.atan(r[8],r[9])
  else y=math.atan(-r[2],r[5]);roll=0 end
  return deg(p),deg(y),deg(roll)
end
local function fromEuler(p,y,r)
  return qnorm(qmul(qmul(axis(3,rad(y)),axis(2,rad(p))),axis(1,rad(r))))
end
local function valid(s)
  if not s or not s.ptr or s.ptr==0 or s.width<16 or s.height<16 or s.width>65536 or s.height>65536 then return false end
  for i=1,14 do if not finite(s.f[i]) then return false end end
  local f=s.f
  if f[1]<=0.01 or f[1]>=3.14 or f[2]<=0.01 or f[2]>=3.14 then return false end
  for i=3,5 do if math.abs(f[i])>1e7 then return false end end
  for row=0,2 do
    local o=6+row*3;local n=f[o]^2+f[o+1]^2+f[o+2]^2
    if n<0.85 or n>1.15 then return false end
  end
  local a,b,c,d,e,g,h,i,j=table.unpack(f,6,14)
  local det=a*(e*j-g*i)-b*(d*j-g*h)+c*(d*i-e*h)
  return det>0.8 and det<1.2
end
local function snapshot()
  for attempt=1,3 do
    local seq=readInteger(m+0x20)
    if seq and seq%2==0 then
      local s={f={},ptr=readQword(m+0x10),width=readInteger(m+0x18) or 0,height=readInteger(m+0x1C) or 0}
      for i=1,14 do s.f[i]=readFloat(m+0x40+(i-1)*4) end
      if readInteger(m+0x20)==seq and valid(s) then return s end
    end
  end
end
local function readUI()
  local v={};for i=1,8 do v[i]=readFloat(m+0x100+(i-1)*4) end
  for i=1,8 do ensure(finite(v[i]),'Champ camera non numerique.') end
  return v
end
local function writeUI()
  local p,y,r=eulers(C.q)
  local v={C.pos[1],C.pos[2],C.pos[3],p,y,r,C.fov,C.speed}
  for i=1,8 do putFloat(m+0x100+(i-1)*4,v[i]) end
  C.lastEuler={readFloat(m+0x10C),readFloat(m+0x110),readFloat(m+0x114)}
end
local function publish(seedCache)
  local r=toNativeMatrix(C.q)
  local vf=rad(C.fov);local hf=2*math.atan(math.tan(vf/2)*C.aspect)
  local f={hf,vf,C.pos[1],C.pos[2],C.pos[3]}
  for i=1,9 do f[#f+1]=r[i] end
  -- Do not open a publication window when the pose did not change.
  if not seedCache and C.lastPayload then
    local same=true
    for i=1,14 do if f[i]~=C.lastPayload[i] then same=false;break end end
    if same then return end
  end
  C.sequence=(C.sequence+2)%0x7FFFFFFE
  ensure(writeInteger(m+4,C.sequence+1),'Verrou payload impossible.')
  for i=1,14 do putFloat(m+0x80+(i-1)*4,f[i]) end
  ensure(writeInteger(m+4,C.sequence),'Publication payload impossible.')
  if seedCache then
    -- Called only with active flag OFF and the engine-side cache lock free.
    for i=1,14 do putFloat(m+0x200+(i-1)*4,f[i]) end
    ensure(writeInteger(m+0x38,C.sequence),'Initialisation cache impossible.')
  end
  C.lastPayload=f
end
local function isFocused() return getForegroundProcess()==C.pid end
function C.status()
  if C.q then
    local p,y,r=eulers(C.q)
    emit(string.format('%s | vue=%X | XYZ %.5f %.5f %.5f | PYR %.3f %.3f %.3f | FOVv %.3f | speed %.3f | captures=%s applied=%s cached=%s',
      C.active and 'ON' or 'OFF',C.target or 0,C.pos[1],C.pos[2],C.pos[3],p,y,r,C.fov,C.speed,
      tostring(readInteger(m+0x24)),tostring(readInteger(m+0x28)),tostring(readInteger(m+0x30))))
  else emit('En attente de la vue du jeu. Charge une sauvegarde et reviens dans le jeu.') end
end
function C.disable()
  if C.active and getOpenedProcessID()==C.pid then C.status() end
  if getOpenedProcessID()==C.pid then writeInteger(m,0);writeQword(m+8,0) end
  C.active=false
  emit('Camera OFF : la vue du jeu reprend le controle.')
end
function C.toggle()
  if not C.running or not isFocused() then return end
  if C.active then C.disable();return end
  if readInteger(m+0x2C)~=0 then emit('Hook encore occupe. Retente Insert.');return end
  if readBytes(base+0x39A529C,1)~=0 then emit('Session Ansel active : ferme-la avant Insert.');return end
  local s=snapshot()
  if not s then emit('Vue pas encore valide. Charge une sauvegarde, ferme les menus puis retente Insert.');return end
  local r={};for i=6,14 do r[#r+1]=s.f[i] end
  C.q=fromNativeMatrix(r);C.pos={s.f[3],s.f[4],s.f[5]};C.fov=deg(s.f[2])
  C.speed=clamp(readFloat(m+0x11C) or 2,0.001,500)
  C.aspect=s.width/s.height;C.target=s.ptr
  writeUI();publish(true)
  ensure(writeQword(m+8,C.target),'Cible camera impossible.')
  ensure(writeInteger(m,1),'Activation camera impossible.')
  C.active=true;C.lastTick=getTickCount();C.staleSince=nil
  C.status()
end
function C.stop()
  C.running=false
  if C.timer then pcall(function() C.timer.destroy() end);C.timer=nil end
  for _,h in ipairs(C.hotkeys) do pcall(function() h.destroy() end) end
  C.hotkeys={}
  if getOpenedProcessID()==C.pid then
    pcall(writeInteger,m,0)
    local current=readBytes(C.hook,#C.patch,true)
    if equal(current,C.patch) then
      local restored=writeBytes(C.hook,C.original)
      if not restored or not equal(readBytes(C.hook,#C.original,true),C.original) then
        emit('ATTENTION: restauration du hook non confirmee. Ne libere pas sa memoire.');return false
      end
    elseif not equal(current,C.original) then
      emit('Hook modifie par un autre outil: restauration refusee pour ne pas ecraser son code.');return false
    end
  end
  pcall(unregisterSymbol,'D2V06Mem')
  C.active=false
  if _G.D2CameraV06==C then _G.D2CameraV06=nil end
  emit('Arrete. Octets du hook restaures lorsque le processus est encore attache. Allocation conservee jusqu a la fermeture du jeu.')
  return true
end
local function key(k) return isKeyPressed(k) and 1 or 0 end
local wasReset=false
local function tick()
  if not C.running then return end
  if getOpenedProcessID()~=C.pid then C.stop();return end
  local now=getTickCount();local dt=clamp((now-(C.lastTick or now))/1000,0,0.05);C.lastTick=now
  if not C.active then
    local s=snapshot()
    if s then
      local r={};for i=6,14 do r[#r+1]=s.f[i] end
      C.q=fromNativeMatrix(r);C.pos={s.f[3],s.f[4],s.f[5]};C.fov=deg(s.f[2]);C.target=s.ptr
      C.speed=clamp(readFloat(m+0x11C) or 2,0.001,500);C.aspect=s.width/s.height
      writeUI()
    end
    return
  end
  if readBytes(base+0x39A529C,1)~=0 then C.disable();emit('Autre session camera detectee; controle rendu au jeu.');return end
  local hits=readInteger(m+0x24)
  if hits~=C.lastHits then C.lastHits=hits;C.staleSince=now
  elseif isFocused() and C.staleSince and now-C.staleSince>3000 then
    C.disable();emit('Vue inactive/changement de niveau. Reactive Insert dans le gameplay.');return
  elseif not isFocused() then C.staleSince=now end
  local v=readUI()
  for i=1,3 do ensure(math.abs(v[i])<1e7,'Position hors limites.') end
  C.pos={v[1],v[2],v[3]};C.fov=clamp(v[7],5,150);C.speed=clamp(v[8],0.001,500)
  if math.abs(v[4]-C.lastEuler[1])>0.0005 or math.abs(v[5]-C.lastEuler[2])>0.0005 or math.abs(v[6]-C.lastEuler[3])>0.0005 then
    C.q=fromEuler(v[4],v[5],v[6])
  end
  local w,h=readInteger(m+0x18),readInteger(m+0x1C)
  if w and h and w>=16 and h>=16 and w<=65536 and h<=65536 then C.aspect=w/h end
  if isFocused() then
    local factor=key(0x12)==1 and 0.01 or (key(0x11)==1 and 0.1 or (key(0x10)==1 and 5 or 1))
    local ang=rad(45*dt*factor)
    local pitch=key(0x28)-key(0x26);local yaw=key(0x27)-key(0x25);local roll=key(0x61)-key(0x63)
    -- Camera-to-world: yaw about world Z; pitch and roll about camera-local axes.
    -- Up -> look up, Right -> turn right. Keep v04's roll key direction.
    C.q=qnorm(qmul(qmul(axis(3,-yaw*ang),C.q),qmul(axis(2,pitch*ang),axis(1,roll*ang))))
    local reset=key(0x62)==1
    if reset and not wasReset then local p,y=eulers(C.q);C.q=fromEuler(p,y,0) end
    wasReset=reset
    local f=key(0x68)-key(0x65);local right=key(0x66)-key(0x64);local up=key(0x69)-key(0x67)
    local len=math.sqrt(f*f+right*right+up*up)
    if len>0 then
      -- Use exactly the same native axes as publish(): first row Forward,
      -- second row Left (negated for Right). NP9/7 remains world-vertical.
      local r=toNativeMatrix(C.q);local step=C.speed*factor*dt/len
      C.pos[1]=C.pos[1]+(r[1]*f-r[4]*right)*step
      C.pos[2]=C.pos[2]+(r[2]*f-r[5]*right)*step
      C.pos[3]=C.pos[3]+(r[3]*f-r[6]*right+up)*step
    end
    C.fov=clamp(C.fov+(key(0x6B)-key(0x6D))*20*dt*factor,5,150)
  else wasReset=false end
  publish();writeUI()
end
-- Code and data are ready before the original instruction is changed.
putFloat(m+0x11C,2)
expect(C.hook,C.original,'verification finale du hook')
ensure(writeBytes(C.hook,C.patch),'Patch refuse; original non remplace.')
if not equal(readBytes(C.hook,#C.patch,true),C.patch) then
  pcall(writeBytes,C.hook,C.original);error('[D2 v06] Verification du patch echouee.',0)
end
C.running=true;_G.D2CameraV06=C
local setup,why=pcall(function()
  C.hotkeys[1]=createHotkey(function()
    local good,e=pcall(C.toggle)
    if not good then C.disable();emit('ERREUR: '..tostring(e)) end
  end,0x2D)
  C.timer=createTimer(nil,false);C.timer.Interval=16
  C.timer.OnTimer=function()
    local good,e=pcall(tick)
    if not good then
      pcall(C.disable);emit('ERREUR TIMER: '..tostring(e));C.stop()
    end
  end
  C.timer.Enabled=true
end)
if not setup then C.stop();error(tostring(why),0) end
emit('Installe. INSERT=freecam. Numpad 8/5/4/6=deplacement, 9/7=haut/bas. Fleches=Pitch/Yaw.')
emit('NP1/3=roll; NP2=reset roll; NP+/-=FOV; SHIFT/CTRL/ALT=vitesses. NumLock ON.')
emit('Ce prototype ne modifie ni g_freeCam, ni la pause, ni le HUD, ni les rs_*; aucun appel Ansel.')
emit('Anti-flicker: une publication incomplete reutilise la pose precedente; controles v05 conserves.')
emit('Desactiver avant changement de carte. D2CameraV06.status() / D2CameraV06.stop()')
emit('LOG='..logfile)
