#include "camera.h"
#include <MinHook.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cwchar>
#include <Psapi.h>
#include <Xinput.h>
extern "C" { __declspec(align(64)) unsigned char cameraData[0x300]{}; void* cameraOriginal=nullptr; void cameraHook(); }
std::recursive_mutex cameraMutex;
Pose pose;
bool cameraReady=false,cameraEnabled=false;
bool timeStopReady=false,timeStopped=false;
bool hudReady=false,hudHidden=false;
bool focusLossReady=false,focusLossEnabled=false;
bool cameraLocked=false;
bool gamepadConnected=false;
int controlMode=0; // 0: Keyboard, 1: Gamepad
Status* status=nullptr;
namespace {
uintptr_t base;
volatile LONG* stopTimeValue=nullptr;
volatile LONG* freezeTimeValue=nullptr;
volatile unsigned char* pauseOnFocusLossValue=nullptr;
LONG originalStopTime=0,originalFreezeTime=0;
unsigned char originalPauseOnFocusLoss=1;
WCHAR settingsPath[MAX_PATH]{};
struct HudCvar { uintptr_t objectRva; const char* name; volatile LONG* integer; volatile float* real; LONG originalInteger; float originalReal; };
HudCvar hudCvars[]={
 {0x39A3610,"g_showHud"},{0x39A3810,"g_showGuis"},{0x398EBB0,"ark_show_ui"},
 {0x397F2A0,"ark_hud_enable_keyhole"},{0x398A000,"ark_ui_crosshair_opacity"},
 {0x3985BC0,"ark_ui_show_awareness_markers"},{0x397D380,"ark_ui_show_contextual_icons"},
 {0x3995FC0,"ark_ui_show_firelock_markers"},{0x397F220,"ark_ui_show_grenade_cooking_gauge"},
 {0x397F1A0,"ark_ui_show_grenade_markers"},{0x3989C20,"ark_ui_show_heart_markers"},
 {0x397D0A0,"ark_ui_show_information_messages"},{0x397D300,"ark_ui_show_interactions"},
 {0x3989840,"ark_ui_show_location_discovery"},{0x398EC30,"ark_ui_show_master_hud"},
 {0x397F410,"ark_ui_show_notifications"},{0x3989BA0,"ark_ui_show_objective_markers"},
 {0x3989AA0,"ark_ui_show_objective_notifications"},{0x398A080,"ark_ui_show_pickup_log"},
 {0x397F680,"ark_ui_show_shortcuts"},{0x397D410,"ark_ui_show_snap_reaction_gauge"},
 {0x3989B20,"ark_ui_show_target_notifications"},{0x3981ED0,"ark_ui_show_tutorial_messages"}
};
struct RenderCvar { uintptr_t objectRva; const char* name; bool floating; bool inverse; volatile LONG* integer=nullptr; volatile float* real=nullptr; volatile char* text=nullptr; LONG originalInteger=0; float originalReal=0; char originalText[16]{}; bool ready=false; };
RenderCvar renderCvars[]={
 {0x32F7D40,"r_skipAutoExposure",false,true},
 {0x32F7DC0,"r_skipAutoExposureTemporalAdapatation",false,false},
 {0x32F7F40,"r_skipBloom",false,true},
 {0x32F7FC0,"r_skipLensFlares",false,true},
 {0x32F8040,"r_skipColorGrading",false,true},
 {0x32F3D50,"r_skipDOF",false,true},
 {0x39A4220,"r_skipCameraMotionBlur",false,true},
 {0x32F7EC0,"r_skipToneMapping",false,true},
 {0x32F82C0,"r_postProcessCustomEV",true,false},
 {0x32F44D0,"r_DofFocalDist",true,false},
 {0x32F4550,"r_DofFocusRange",true,false},
 {0x32F47D0,"r_DofCOC",true,false},
 {0x397AC50,"r_skipFog",false,true},
 {0x397AD50,"r_skipGodRays",false,true},
 {0x397ACD0,"r_skipLightScattering",false,true},
 {0x397AF50,"r_skipSky",false,true},
 {0x32FBD20,"r_areaLights_FactorDiffuse",true,false},
 {0x32FBDA0,"r_areaLights_FactorSpecular",true,false},
 {0x32FBE20,"r_areaLights_ShadowMin",true,false},
 {0x32FBEA0,"r_areaLights_ShadowMax",true,false},
 {0x32F86C0,"r_postProcessDebugAreaLights",false,false},
 {0x3937E60,"r_showLights",false,false},
 {0x32F8970,"r_shadowCullDistance",true,false},
 {0x3982E70,"r_viewDistPreset",false,false},
 {0x3982CF0,"r_shadowQualityPreset",false,false},
 {0x3982DF0,"r_modelsQualityPreset",false,false},
 {0x32FBF20,"r_lodZoomMultiplicator",true,false},
 {0x3988BB0,"lod_forcevisibledistance",true,false},
 {0x3988C30,"lod_dist_level0",true,false},
 {0x3988CB0,"lod_dist_level1",true,false},
 {0x3988D30,"lod_dist_level2",true,false},
 {0x3988DB0,"lod_dist_level3",true,false},
 {0x3988E30,"lod_dist_far",true,false},
 {0x3982F70,"r_textureDetailsPreset",false,false},
 {0x32EA470,"image_lodbias",true,false},
 {0x3937860,"r_forceFirstMipMap",false,false},
 {0x32EFD90,"r_addAllMips",false,false},
 {0x32F0090,"r_textureStreamerMemoryBudgetMB",false,false},
 {0x32F8B70,"r_shadowParallelPartialSliceUpdate",false,false},
 {0x397A250,"r_shadowParallelSliceSize",false,false}
};
static_assert(sizeof(renderCvars)/sizeof(renderCvars[0])==static_cast<unsigned>(RenderControl::Count),"RenderControl enum and retail CVar bindings must stay aligned");
volatile char* shadowCascadeText=nullptr;
char originalShadowCascadeText[16]{};
bool shadowCascadeReadyFlag=false;
float aspect=1;
uint64_t target=0,lastHitTime=0;
unsigned lastHits=0;
struct Q { float x=0,y=0,z=0,w=1; } q;
bool igcsSessionActive=false;
Pose igcsSessionStart{};
Q igcsSessionStartOrientation{};
void native(Q a,float* r);
void publish(bool seed);
struct ConnectorVec3 { float values[3]; };
struct ConnectorVec4 { float values[4]; };
struct ConnectorCameraData {
 unsigned char cameraEnabled, cameraMovementLocked, reserved1, reserved2;
 float fov;
 ConnectorVec3 coordinates;
 ConnectorVec4 lookQuaternion;
 ConnectorVec3 rotationMatrixUpVector;
 ConnectorVec3 rotationMatrixRightVector;
 ConnectorVec3 rotationMatrixForwardVector;
 float pitch, yaw, roll;
};
static_assert(offsetof(ConnectorCameraData, fov)==4 && offsetof(ConnectorCameraData, coordinates)==8 && offsetof(ConnectorCameraData, lookQuaternion)==20 && offsetof(ConnectorCameraData, rotationMatrixUpVector)==36 && offsetof(ConnectorCameraData, pitch)==72 && sizeof(ConnectorCameraData)==84,"IGCS Connector CameraToolsData ABI mismatch");
using ConnectorConnect = bool(*)();
using ConnectorGetBuffer = unsigned char*(*)();
static ConnectorCameraData* connectorData=nullptr;
static uint64_t connectorRetryAt=0;
constexpr float pi=3.14159265358979323846f;
float rad(float x){return x*pi/180;} float deg(float x){return x*180/pi;}
void updateIgcsConnector(bool enabled, bool locked, const Pose& cameraPose, Q orientation, const float* rotation){
 if(!connectorData && GetTickCount64()>=connectorRetryAt){
  connectorRetryAt=GetTickCount64()+1000;
  HMODULE modules[512]{};DWORD needed=0;
  if(EnumProcessModules(GetCurrentProcess(),modules,sizeof modules,&needed)){
   for(DWORD i=0;i<std::min<DWORD>(needed/sizeof(HMODULE),512);++i){
    auto connect=reinterpret_cast<ConnectorConnect>(GetProcAddress(modules[i],"connectFromCameraTools"));
    auto getBuffer=reinterpret_cast<ConnectorGetBuffer>(GetProcAddress(modules[i],"getDataFromCameraToolsBuffer"));
    if(connect&&getBuffer&&connect()){
     connectorData=reinterpret_cast<ConnectorCameraData*>(getBuffer());
     if(connectorData)break;
    }
   }
  }
 }
 if(!connectorData)return;
 connectorData->cameraEnabled=enabled?1:0;
 connectorData->cameraMovementLocked=locked?1:0;
 connectorData->reserved1=connectorData->reserved2=0;
 connectorData->fov=cameraPose.fov;
 memcpy(connectorData->coordinates.values,cameraPose.xyz,sizeof cameraPose.xyz);
 connectorData->lookQuaternion.values[0]=orientation.x;connectorData->lookQuaternion.values[1]=orientation.y;connectorData->lookQuaternion.values[2]=orientation.z;connectorData->lookQuaternion.values[3]=orientation.w;
 connectorData->rotationMatrixForwardVector.values[0]=rotation[0];connectorData->rotationMatrixForwardVector.values[1]=rotation[1];connectorData->rotationMatrixForwardVector.values[2]=rotation[2];
 connectorData->rotationMatrixRightVector.values[0]=-rotation[3];connectorData->rotationMatrixRightVector.values[1]=-rotation[4];connectorData->rotationMatrixRightVector.values[2]=-rotation[5];
 memcpy(connectorData->rotationMatrixUpVector.values,rotation+6,sizeof(float)*3);
 connectorData->pitch=rad(cameraPose.pyr[0]);connectorData->yaw=rad(cameraPose.pyr[1]);connectorData->roll=rad(cameraPose.pyr[2]);
}
extern "C" __declspec(dllexport) int __cdecl IGCS_StartScreenshotSession(unsigned char type){
 std::lock_guard<std::recursive_mutex> g(cameraMutex);
 constexpr int allOk=0,errorCameraNotEnabled=1,errorAlreadyActive=3,errorFeatureUnavailable=4;
 if(type!=1)return errorFeatureUnavailable; // IGCS ScreenshotType::MultiShot
 if(igcsSessionActive)return errorAlreadyActive;
 if(!cameraEnabled||!cameraReady)return errorCameraNotEnabled;
 igcsSessionStart=pose;
 igcsSessionStartOrientation=q;
 igcsSessionActive=true;
 // Publish the locked state immediately so Connector shaders and its UI see it.
 float rotation[9];native(q,rotation);
 updateIgcsConnector(true,true,pose,q,rotation);
 return allOk;
}
extern "C" __declspec(dllexport) void __cdecl IGCS_EndScreenshotSession(){
 std::lock_guard<std::recursive_mutex> g(cameraMutex);
 if(!igcsSessionActive)return;
 pose=igcsSessionStart;
 q=igcsSessionStartOrientation;
 igcsSessionActive=false;
 if(cameraEnabled)publish(false);
 float rotation[9];native(q,rotation);
 updateIgcsConnector(cameraEnabled,cameraLocked,pose,q,rotation);
}
extern "C" __declspec(dllexport) void IGCS_MoveCameraPanorama(float stepAngle){(void)stepAngle;}
extern "C" __declspec(dllexport) void __cdecl IGCS_MoveCameraMultishot(float stepLeftRight,float stepUpDown,float fovDegrees,bool fromStartPosition){
 std::lock_guard<std::recursive_mutex> g(cameraMutex);
 if(!igcsSessionActive||!cameraEnabled||!std::isfinite(stepLeftRight)||!std::isfinite(stepUpDown))return;
 Pose basePose=fromStartPosition?igcsSessionStart:pose;
 Q baseOrientation=fromStartPosition?igcsSessionStartOrientation:q;
 float rotation[9];native(baseOrientation,rotation);
 // Match the IGCS camera-data basis published above: right=-row 1, up=row 2.
 for(int i=0;i<3;++i)
  pose.xyz[i]=basePose.xyz[i]-rotation[3+i]*stepLeftRight+rotation[6+i]*stepUpDown;
 pose.pyr[0]=basePose.pyr[0];pose.pyr[1]=basePose.pyr[1];pose.pyr[2]=basePose.pyr[2];
 q=baseOrientation;
 if(fovDegrees>0.0f&&std::isfinite(fovDegrees))pose.fov=std::clamp(fovDegrees,5.0f,150.0f);
 else pose.fov=basePose.fov;
 publish(false);
 native(q,rotation);
 updateIgcsConnector(true,true,pose,q,rotation);
}
template<class T>T read(size_t n){return *reinterpret_cast<volatile T*>(cameraData+n);}
template<class T>void write(size_t n,T v){*reinterpret_cast<volatile T*>(cameraData+n)=v;}
Q mul(Q a,Q b){return {a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z};}
Q norm(Q a){float n=sqrtf(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);return {a.x/n,a.y/n,a.z/n,a.w/n};}
Q axis(int i,float a){Q r; r.w=cosf(a/2);if(i==0)r.x=sinf(a/2);if(i==1)r.y=sinf(a/2);if(i==2)r.z=sinf(a/2);return r;}
Q fromEuler(){return norm(mul(mul(axis(2,rad(pose.pyr[1])),axis(1,rad(pose.pyr[0]))),axis(0,rad(pose.pyr[2]))));}
void native(Q a,float* r){auto [x,y,z,w]=a;r[0]=1-2*(y*y+z*z);r[1]=2*(x*y+z*w);r[2]=2*(x*z-y*w);r[3]=2*(x*y-z*w);r[4]=1-2*(x*x+z*z);r[5]=2*(y*z+x*w);r[6]=2*(x*z+y*w);r[7]=2*(y*z-x*w);r[8]=1-2*(x*x+y*y);}
void eulers(const float* r){float p=asinf(std::clamp(-r[2],-1.f,1.f));pose.pyr[0]=deg(p);if(fabsf(cosf(p))>0.00001f){pose.pyr[1]=deg(atan2f(r[1],r[0]));pose.pyr[2]=deg(atan2f(r[5],r[8]));}else{pose.pyr[1]=deg(atan2f(-r[3],r[4]));pose.pyr[2]=0;}}
struct Capture{float f[14]; uint64_t ptr; unsigned w,h;};
bool snapshot(Capture& s){for(int attempt=0;attempt<3;++attempt){auto seq=read<unsigned>(0x20);if(seq&1)continue;s.ptr=read<uint64_t>(0x10);s.w=read<unsigned>(0x18);s.h=read<unsigned>(0x1c);for(int i=0;i<14;++i)s.f[i]=read<float>(0x40+i*4);MemoryBarrier();if(seq!=read<unsigned>(0x20))continue;if(!s.ptr||s.w<16||s.h<16||s.w>65536||s.h>65536)return false;for(auto v:s.f)if(!std::isfinite(v))return false;if(s.f[0]<.01||s.f[0]>3.14||s.f[1]<.01||s.f[1]>3.14)return false;for(int i=2;i<5;++i)if(fabsf(s.f[i])>1e7)return false;for(int i=5;i<14;i+=3){float n=s.f[i]*s.f[i]+s.f[i+1]*s.f[i+1]+s.f[i+2]*s.f[i+2];if(n<.85||n>1.15)return false;}auto r=s.f+5;float det=r[0]*(r[4]*r[8]-r[5]*r[7])-r[1]*(r[3]*r[8]-r[5]*r[6])+r[2]*(r[3]*r[7]-r[4]*r[6]);return det>.8&&det<1.2;}return false;}
void publish(bool seed){float f[14];f[1]=rad(pose.fov);f[0]=2*atanf(tanf(f[1]/2)*aspect);memcpy(f+2,pose.xyz,12);native(q,f+5);InterlockedIncrement(reinterpret_cast<LONG*>(cameraData+4));for(int i=0;i<14;++i)write<float>(0x80+i*4,f[i]);MemoryBarrier();InterlockedIncrement(reinterpret_cast<LONG*>(cameraData+4));if(seed)memcpy(cameraData+0x200,f,sizeof f);}
void disable(){write<unsigned>(0,0);MemoryBarrier();write<uint64_t>(8,0);cameraEnabled=false;}
bool key(int k){return (GetAsyncKeyState(k)&0x8000)!=0;}
bool edge(int k){static bool down[256]{};bool d=key(k),r=d&&!down[k];down[k]=d;return r;}
float stick(SHORT value,SHORT deadzone){float v=float(value)/32767.f;float a=fabsf(v);return a<=float(deadzone)/32767.f?0.f:std::copysign((a-float(deadzone)/32767.f)/(1.f-float(deadzone)/32767.f),v);}
bool resolveCvar(uintptr_t objectRva,const char* expected,volatile LONG** value,volatile float** floatValue=nullptr){auto object=base+objectRva;auto name=*reinterpret_cast<volatile uint64_t*>(object+0x30);size_t length=strlen(expected);if(name<base||name>=base+0x41CB000||length+1>base+0x41CB000-name)return false;if(memcmp((const void*)name,expected,length+1))return false;*value=reinterpret_cast<volatile LONG*>(object+0x28);if(floatValue)*floatValue=reinterpret_cast<volatile float*>(object+0x2c);return true;}
void resolveRenderCvars(){for(auto& c:renderCvars){c.ready=resolveCvar(c.objectRva,c.name,&c.integer,&c.real);if(c.ready){c.text=reinterpret_cast<volatile char*>(base+c.objectRva+0x10);bool terminated=false;for(int i=0;i<16;++i){c.originalText[i]=c.text[i];if(!c.originalText[i]){terminated=true;break;}}if(!terminated){c.ready=false;c.text=nullptr;continue;}c.originalInteger=*c.integer;c.originalReal=*c.real;}}}
void resolveShadowCascade(){volatile LONG* unused=nullptr;if(!resolveCvar(0x397A350,"r_shadowCascadeFars",&unused))return;shadowCascadeText=reinterpret_cast<volatile char*>(base+0x397A360);char value[16]{};for(int i=0;i<15;++i){value[i]=shadowCascadeText[i];if(!value[i])break;}int a,b,c,d;if(sscanf_s(value,"%d %d %d %d",&a,&b,&c,&d)!=4)return;strncpy_s(originalShadowCascadeText,value,_TRUNCATE);shadowCascadeReadyFlag=true;}
void loadControlMode(){DWORD length=GetEnvironmentVariableW(L"LOCALAPPDATA",settingsPath,MAX_PATH);if(!length||length>=MAX_PATH-32){settingsPath[0]=0;controlMode=0;return;}wcscat_s(settingsPath,L"\\VoidCamera");CreateDirectoryW(settingsPath,nullptr);wcscat_s(settingsPath,L"\\settings.ini");controlMode=static_cast<int>(GetPrivateProfileIntW(L"Input",L"ControlMode",0,settingsPath));if(controlMode<0||controlMode>1)controlMode=0;}
void saveControlModeInternal(){if(!settingsPath[0])return;wchar_t value[2]{static_cast<wchar_t>(L'0'+controlMode),0};WritePrivateProfileStringW(L"Input",L"ControlMode",value,settingsPath);}
bool resolveFocusLossValue(){
 const unsigned char p[]={0x83,0x3D,0,0,0,0,0,0x74,0x4E,0x48,0x8B,0x0D,0,0,0,0,0x48,0x8B,0x01};
 auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);auto section=IMAGE_FIRST_SECTION(nt);uintptr_t match=0;
 for(unsigned n=0;n<nt->FileHeader.NumberOfSections;++n,++section){if((section->Characteristics&(IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_MEM_READ))!=(IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_MEM_READ))continue;uintptr_t start=base+section->VirtualAddress;size_t size=std::min<size_t>(section->Misc.VirtualSize,nt->OptionalHeader.SizeOfImage-section->VirtualAddress);for(uintptr_t i=start;i+sizeof(p)<=start+size;++i){if(*(const unsigned char*)i!=p[0]||memcmp((const void*)i,p,2)||memcmp((const void*)(i+6),p+6,6)||memcmp((const void*)(i+16),p+16,3))continue;if(match)return false;match=i;}}
 if(!match)return false;int32_t displacement;memcpy(&displacement,(const void*)(match+2),sizeof(displacement));uintptr_t target=match+7+displacement;if(target!=base+0x3139408)return false;MEMORY_BASIC_INFORMATION mbi{};if(!VirtualQuery((const void*)target,&mbi,sizeof(mbi))||mbi.State!=MEM_COMMIT||(mbi.Protect&(PAGE_READWRITE|PAGE_WRITECOPY|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY))==0)return false;pauseOnFocusLossValue=reinterpret_cast<volatile unsigned char*>(target);return *pauseOnFocusLossValue<=1;
}
bool verify(){base=reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Dishonored2.exe"));if(!base)return false;auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);if(nt->FileHeader.TimeDateStamp!=0x67A50CDE||nt->OptionalHeader.SizeOfImage!=0x41CB000)return false;const unsigned char sig[]={0x44,0x38,0x25,0x84,0xfb,0x62,0x03},fov[]={0xf3,0x0f,0x10,0x87,0x70,0x3a,0,0},xyz[]={0xf3,0x0f,0x11,0x87,0x78,0x3a,0,0};return !memcmp((void*)(base+0x375711),sig,7)&&!memcmp((void*)(base+0x3757b5),fov,8)&&!memcmp((void*)(base+0x3758d6),xyz,8);}
DWORD WINAPI run(void*){wchar_t name[80];swprintf_s(name,L"Local\\VoidCamera_%lu",GetCurrentProcessId());HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Status),name);if(!mapping)return 1;status=(Status*)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(Status));if(!status)return 1;new(status)Status;
 if(!verify()){status->error=1;return 1;}
 loadControlMode();
 resolveRenderCvars();
 resolveShadowCascade();
 timeStopReady=resolveCvar(0x39A0E10,"g_stopTime",&stopTimeValue)&&resolveCvar(0x39A60E0,"g_freezeTime",&freezeTimeValue);
 hudReady=true;for(auto& c:hudCvars)if(!resolveCvar(c.objectRva,c.name,&c.integer,&c.real)){hudReady=false;break;}
 focusLossReady=resolveFocusLossValue();
 if(focusLossReady){originalPauseOnFocusLoss=*pauseOnFocusLossValue;InterlockedExchange8(reinterpret_cast<volatile char*>(pauseOnFocusLossValue),0);focusLossEnabled=true;}
 if(MH_Initialize()!=MH_OK||MH_CreateHook((void*)(base+0x375711),(void*)cameraHook,&cameraOriginal)!=MH_OK){status->error=2;return 1;}
 if(MH_EnableHook((void*)(base+0x375711))!=MH_OK){status->error=3;return 1;}
 if(!overlayInstall()){MH_DisableHook((void*)(base+0x375711));status->error=4;return 1;}
 status->ready=1;for(;;){cameraFrame();Sleep(8);} }
}
void saveControlMode(){saveControlModeInternal();}
void cameraToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(cameraEnabled){disable();return;}Capture s;if(read<unsigned>(0x2c)||*(volatile unsigned char*)(base+0x39a529c)||!snapshot(s))return;target=s.ptr;aspect=float(s.w)/s.h;memcpy(pose.xyz,s.f+2,12);eulers(s.f+5);q=fromEuler();pose.fov=std::clamp(deg(s.f[1]),5.f,150.f);publish(true);write<uint64_t>(8,target);MemoryBarrier();write<unsigned>(0,1);cameraEnabled=true;lastHitTime=GetTickCount64();}
void timeStopToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!timeStopReady)return;if(!timeStopped){originalStopTime=*stopTimeValue;originalFreezeTime=*freezeTimeValue;InterlockedExchange(stopTimeValue,1);InterlockedExchange(freezeTimeValue,1);timeStopped=true;}else{InterlockedExchange(stopTimeValue,originalStopTime);InterlockedExchange(freezeTimeValue,originalFreezeTime);timeStopped=false;}}
void hudToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!hudReady)return;if(!hudHidden){for(auto& c:hudCvars){c.originalInteger=*c.integer;c.originalReal=*c.real;InterlockedExchange(c.integer,0);*c.real=0;}hudHidden=true;}else{for(auto& c:hudCvars){InterlockedExchange(c.integer,c.originalInteger);*c.real=c.originalReal;}hudHidden=false;}}
void focusLossToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!focusLossReady)return;if(!focusLossEnabled){InterlockedExchange8(reinterpret_cast<volatile char*>(pauseOnFocusLossValue),0);focusLossEnabled=true;}else{InterlockedExchange8(reinterpret_cast<volatile char*>(pauseOnFocusLossValue),static_cast<char>(originalPauseOnFocusLoss));focusLossEnabled=false;}}
void cameraPublish(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!cameraEnabled)return;for(float x:pose.xyz)if(!std::isfinite(x)||fabsf(x)>=1e7)return;for(float x:pose.pyr)if(!std::isfinite(x))return;pose.fov=std::clamp(pose.fov,5.f,150.f);pose.speed=std::clamp(pose.speed,.001f,500.f);q=fromEuler();publish(false);}
void cameraFrame(){std::lock_guard<std::recursive_mutex> g(cameraMutex);static uint64_t last=GetTickCount64();uint64_t now=GetTickCount64();float dt=std::min((now-last)*.001f,.05f);last=now;DWORD fg=0;GetWindowThreadProcessId(GetForegroundWindow(),&fg);bool focused=fg==GetCurrentProcessId();bool ins=edge(VK_INSERT),del=edge(VK_DELETE),pause=edge(VK_NUMPAD0),home=edge(VK_HOME);if(focused&&ins&&!igcsSessionActive)cameraToggle();if(focused&&del)hudToggle();if(focused&&pause)timeStopToggle();if(focused&&home)cameraLocked=!cameraLocked;XINPUT_STATE gamepadState{};gamepadConnected=controlMode==1&&XInputGetState(0,&gamepadState)==ERROR_SUCCESS;
 Capture s;cameraReady=snapshot(s);if(cameraReady){aspect=float(s.w)/s.h;if(!cameraEnabled){memcpy(pose.xyz,s.f+2,12);eulers(s.f+5);q=fromEuler();pose.fov=deg(s.f[1]);}}
 if(cameraEnabled){unsigned hits=read<unsigned>(0x24);if(hits!=lastHits||!focused){lastHitTime=now;lastHits=hits;}if(*(volatile unsigned char*)(base+0x39a529c)||(!igcsSessionActive&&!timeStopped&&now-lastHitTime>3000))disable();}
 if(cameraEnabled){if(focused&&!status->overlay&&!cameraLocked&&!igcsSessionActive){float factor=key(VK_MENU)?.01f:key(VK_CONTROL)?.1f:key(VK_SHIFT)?5.f:1.f;float a=rad(45*dt*factor);float p=0,y=0,r=0,f=0,right=0,up=0,fov=0;
  if(controlMode==1){if(gamepadConnected){auto& gpad=gamepadState.Gamepad;p=-stick(gpad.sThumbRY,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);y=stick(gpad.sThumbRX,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);f=stick(gpad.sThumbLY,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);right=stick(gpad.sThumbLX,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);up=(float(gpad.bLeftTrigger)-float(gpad.bRightTrigger))/255.f;factor*=gpad.wButtons&XINPUT_GAMEPAD_A?5.f:gpad.wButtons&XINPUT_GAMEPAD_X?.1f:1.f;fov=(gpad.wButtons&XINPUT_GAMEPAD_DPAD_DOWN?1.f:0.f)-(gpad.wButtons&XINPUT_GAMEPAD_DPAD_UP?1.f:0.f);r=(gpad.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT?1.f:0.f)-(gpad.wButtons&XINPUT_GAMEPAD_DPAD_LEFT?1.f:0.f);if(gpad.wButtons&XINPUT_GAMEPAD_B)pose.fov=60.f;}}
  else{p=float(key(VK_DOWN))-key(VK_UP);y=float(key(VK_RIGHT))-key(VK_LEFT);r=float(key(VK_NUMPAD1))-key(VK_NUMPAD3);f=float(key(VK_NUMPAD8))-key(VK_NUMPAD5);right=float(key(VK_NUMPAD6))-key(VK_NUMPAD4);up=float(key(VK_NUMPAD9))-key(VK_NUMPAD7);fov=float(key(VK_ADD))-key(VK_SUBTRACT);if(key(VK_NUMPAD2)){pose.pyr[2]=0;q=fromEuler();}}
  q=norm(mul(mul(axis(2,-y*a),q),mul(axis(1,p*a),axis(0,r*a))));float m[9];native(q,m);eulers(m);float len=sqrtf(f*f+right*right+up*up);if(len>1){f/=len;right/=len;up/=len;len=1;}if(len>0){float step=pose.speed*factor*dt;for(int i=0;i<3;++i)pose.xyz[i]+=(m[i]*f-m[i+3]*right+(i==2?up:0))*step;}pose.fov=std::clamp(pose.fov+fov*20*dt*factor,5.f,150.f);}publish(false);}
 if(status->command!=status->ack){LONG cmd=status->command;if(cmd==1&&!cameraEnabled)cameraToggle();if(cmd==2&&cameraEnabled)disable();if(cmd==3){memcpy(&pose,status->args,sizeof(Pose));cameraPublish();}if(cmd==4)status->overlay=!status->overlay;if(cmd==5&&!timeStopped)timeStopToggle();if(cmd==6&&timeStopped)timeStopToggle();status->ack=cmd;}
 status->enabled=cameraEnabled;status->captures=read<unsigned>(0x24);status->applied=read<unsigned>(0x28);status->cached=read<unsigned>(0x30);status->view=read<uint64_t>(0x10);status->pose=pose;
 status->timeStopReady=timeStopReady;status->timeStopped=timeStopped;status->hudReady=hudReady;status->hudHidden=hudHidden;status->focusLossReady=focusLossReady;status->focusLossEnabled=focusLossEnabled;
 status->locked=cameraLocked;status->controlMode=controlMode;status->aspect=aspect;status->horizontalFov=deg(2*atanf(tanf(rad(pose.fov)*.5f)*aspect));float connectorRotation[9];native(q,connectorRotation);updateIgcsConnector(cameraEnabled,cameraLocked||igcsSessionActive,pose,q,connectorRotation);InterlockedExchange(&status->igcsConnectorReady,connectorData?1:0);
}
bool renderControlReady(RenderControl id){auto i=static_cast<unsigned>(id);return i<static_cast<unsigned>(RenderControl::Count)&&renderCvars[i].ready;}
bool renderControlEnabled(RenderControl id){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready)return false;auto& c=renderCvars[i];return ((*c.integer!=0)!=c.inverse);}
float renderControlValue(RenderControl id){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready)return 0;auto& c=renderCvars[i];return c.floating?*c.real:static_cast<float>(*c.integer);}
namespace { void writeRenderCvar(RenderCvar& c,float value){char text[16]{};int n=c.floating?sprintf_s(text,"%.7g",static_cast<double>(value)):sprintf_s(text,"%d",static_cast<int>(value));if(n<0||n>=16)return;memcpy(const_cast<char*>(c.text),text,sizeof text);MemoryBarrier();InterlockedExchange(c.integer,static_cast<LONG>(value));*c.real=value;} }
void renderControlSetEnabled(RenderControl id,bool value){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready)return;auto& c=renderCvars[i];LONG raw=(value!=c.inverse)?1:0;writeRenderCvar(c,static_cast<float>(raw));}
void renderControlSetValue(RenderControl id,float value){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready||!std::isfinite(value))return;writeRenderCvar(renderCvars[i],value);}
bool shadowCascadeReady(){return shadowCascadeReadyFlag;}
int shadowCascadeFarDistance(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!shadowCascadeReadyFlag)return 0;char value[16]{};for(int i=0;i<15;++i){value[i]=shadowCascadeText[i];if(!value[i])break;}int a,b,c,d;if(sscanf_s(value,"%d %d %d %d",&a,&b,&c,&d)!=4)return 0;return d;}
void shadowCascadeSetFarDistance(int distance){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!shadowCascadeReadyFlag||distance<1||distance>999)return;char current[16]{};for(int i=0;i<15;++i){current[i]=shadowCascadeText[i];if(!current[i])break;}int a,b,c,d;if(sscanf_s(current,"%d %d %d %d",&a,&b,&c,&d)!=4||distance<=c)return;char updated[16]{};int length=sprintf_s(updated,"%d %d %d %d",a,b,c,distance);if(length<0||length>=16)return;memcpy(const_cast<char*>(shadowCascadeText),updated,sizeof updated);MemoryBarrier();}
void renderControlRestore(){std::lock_guard<std::recursive_mutex> g(cameraMutex);for(auto& c:renderCvars)if(c.ready){memcpy(const_cast<char*>(c.text),c.originalText,sizeof c.originalText);MemoryBarrier();InterlockedExchange(c.integer,c.originalInteger);*c.real=c.originalReal;}if(shadowCascadeReadyFlag)memcpy(const_cast<char*>(shadowCascadeText),originalShadowCascadeText,sizeof originalShadowCascadeText);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){DisableThreadLibraryCalls(h);HANDLE t=CreateThread(nullptr,0,run,nullptr,0,nullptr);if(t)CloseHandle(t);}return TRUE;}
