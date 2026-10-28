#include "camera.h"
#include <MinHook.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cwchar>
#include <mutex>
#include <vector>
#include <intrin.h>
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
int rotationSpeed=950; // PYR angular speed in degrees per second
bool cameraNeedsNeutralInput=false;
bool cameraNeutralInputFrameSeen=false;
Status* status=nullptr;
namespace {
uintptr_t base;
volatile LONG* stopTimeValue=nullptr;
volatile LONG* freezeTimeValue=nullptr;
volatile unsigned char* pauseOnFocusLossValue=nullptr;
LONG originalStopTime=0,originalFreezeTime=0;
unsigned char originalPauseOnFocusLoss=1;
WCHAR settingsPath[MAX_PATH]{};
struct HudCvar { uintptr_t objectRva; const char* name; volatile LONG* integer=nullptr; volatile float* real=nullptr; volatile char* text=nullptr; LONG originalInteger=0; float originalReal=0; char originalText[16]{}; };
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
 {0x3937460,"r_zfar",true,false},
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
constexpr RenderControl lodDistanceControls[]={RenderControl::LodForceVisibleDistance,RenderControl::LodDistance0,RenderControl::LodDistance1,RenderControl::LodDistance2,RenderControl::LodDistance3,RenderControl::LodDistanceFar};
constexpr float lodForceValues[]={100000.f,100000.f,200000.f,300000.f,400000.f,500000.f};
float lodPreForceValues[6]{};
bool lodForceActive=false;
FILE* lodLogFile=nullptr;
wchar_t lodLogPath[MAX_PATH]{};
uint64_t lodLastSample=0;
int lodPendingMeshMark=0;
volatile char* shadowCascadeText=nullptr;
char originalShadowCascadeText[16]{};
bool shadowCascadeReadyFlag=false;
bool nativeViewDistanceSetterReadyFlag=false;
using NativeCvarSetter=unsigned char(__fastcall*)(void*,int,unsigned char);
NativeCvarSetter nativeCvarSetterOriginal=nullptr;
FILE* presetTraceFile=nullptr;
FILE* presetDiffFile=nullptr;
FILE* presetManagerTraceFile=nullptr;
FILE* presetCallbackTraceFile=nullptr;
wchar_t presetTracePath[MAX_PATH]{};
std::vector<uintptr_t> trackedCvarObjects;
std::vector<uintptr_t> loggedPresetManagerVtables;
std::mutex presetTraceMutex;
thread_local unsigned presetTraceDepth=0;
thread_local bool presetTraceActive=false;
thread_local uint64_t presetTraceId=0;
volatile LONG presetTraceSequence=0;
uintptr_t presetNotifyTarget=0;
using PresetNotifyFn=uintptr_t(__fastcall*)(void*,uint32_t);
PresetNotifyFn presetNotifyOriginal=nullptr;
float aspect=1;
bool renderAspectReady=false;
uint64_t target=0,lastHitTime=0;
unsigned lastHits=0;
struct Q { float x=0,y=0,z=0,w=1; } q;
bool igcsSessionActive=false;
Pose igcsSessionStart{};
Q igcsSessionStartOrientation{};
void native(Q a,float* r);
void publish(bool seed);
void openPresetTrace(){wchar_t path[MAX_PATH]{};DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,MAX_PATH);if(!n||n>=MAX_PATH-48)return;wcscat_s(path,L"\\VoidCamera");CreateDirectoryW(path,nullptr);wcscpy_s(presetTracePath,path);wcscat_s(path,L"\\r-viewdist-preset-trace.csv");if(_wfopen_s(&presetTraceFile,path,L"w")!=0||!presetTraceFile){presetTraceFile=nullptr;return;}fprintf(presetTraceFile,"event_id,tick_ms,thread_id,call_depth,event,name,object_rva,old_int,requested_int,new_int,old_float,new_float,force,return_al_observed,caller_rva,stack_rvas\n");fflush(presetTraceFile);wcscpy_s(path,presetTracePath);wcscat_s(path,L"\\r-viewdist-cvar-diff.csv");if(_wfopen_s(&presetDiffFile,path,L"w")!=0||!presetDiffFile){presetDiffFile=nullptr;return;}fprintf(presetDiffFile,"event_id,name,object_rva,old_int,new_int,old_float,new_float,old_text,new_text\n");fflush(presetDiffFile);wcscpy_s(path,presetTracePath);wcscat_s(path,L"\\r-viewdist-notify-vtable.csv");if(_wfopen_s(&presetManagerTraceFile,path,L"w")!=0||!presetManagerTraceFile)presetManagerTraceFile=nullptr;else{fprintf(presetManagerTraceFile,"event_id,tick_ms,thread_id,event,manager_ptr,manager_rva,vtable_ptr,vtable_rva,flags,pending_before,pending_after,slot,target_ptr,target_rva\n");fflush(presetManagerTraceFile);}wcscpy_s(path,presetTracePath);wcscat_s(path,L"\\r-viewdist-cvar-callbacks.csv");if(_wfopen_s(&presetCallbackTraceFile,path,L"w")!=0||!presetCallbackTraceFile)presetCallbackTraceFile=nullptr;else{fprintf(presetCallbackTraceFile,"event_id,tick_ms,thread_id,phase,cvar_rva,list_head,entry_index,node_ptr,callback_object,callback_vtable,callback_method,callback_method_rva,next_node\n");fflush(presetCallbackTraceFile);}}
bool cvarNameFromObject(void* object,char* out,size_t capacity,uintptr_t* objectRva){if(!object||!out||capacity<2)return false;uintptr_t address=reinterpret_cast<uintptr_t>(object);if(address<base||address+0x38>=base+0x41CB000)return false;if(objectRva)*objectRva=address-base;uint64_t name=0;memcpy(&name,reinterpret_cast<const void*>(address+0x30),sizeof name);if(name<base||name>=base+0x41CB000-128)return false;size_t i=0;for(;i+1<capacity&&i<127;++i){char ch=*reinterpret_cast<const char*>(name+i);if(!ch)break;if(static_cast<unsigned char>(ch)<32||static_cast<unsigned char>(ch)>126)return false;out[i]=ch;}if(i==0||i+1>=capacity)return false;out[i]=0;return true;}
struct CvarSample{LONG integer;float real;char text[16];};
std::vector<CvarSample> snapshotStaticCvars(){std::vector<CvarSample> values;values.reserve(trackedCvarObjects.size());for(uintptr_t rva:trackedCvarObjects){CvarSample s{};uintptr_t object=base+rva;s.integer=*reinterpret_cast<volatile LONG*>(object+0x28);s.real=*reinterpret_cast<volatile float*>(object+0x2c);for(unsigned i=0;i<sizeof s.text;++i)s.text[i]=*reinterpret_cast<volatile char*>(object+0x10+i);values.push_back(s);}return values;}
void writeCsvString(FILE* file,const char* value){fputc('"',file);for(const char* p=value;*p;++p){if(*p=='"')fputc('"',file);fputc(*p,file);}fputc('"',file);}
void writePresetCvarDiff(uint64_t id,const std::vector<CvarSample>& before){if(!presetDiffFile||before.size()!=trackedCvarObjects.size())return;std::lock_guard<std::mutex> guard(presetTraceMutex);for(size_t i=0;i<before.size();++i){uintptr_t object=base+trackedCvarObjects[i];CvarSample after{};after.integer=*reinterpret_cast<volatile LONG*>(object+0x28);after.real=*reinterpret_cast<volatile float*>(object+0x2c);for(unsigned c=0;c<sizeof after.text;++c)after.text[c]=*reinterpret_cast<volatile char*>(object+0x10+c);if(after.integer==before[i].integer&&memcmp(&after.real,&before[i].real,sizeof(float))==0&&memcmp(after.text,before[i].text,sizeof after.text)==0)continue;char name[128]{};uintptr_t objectRva=0;if(!cvarNameFromObject(reinterpret_cast<void*>(object),name,sizeof name,&objectRva))continue;fprintf(presetDiffFile,"%llu,%s,%llX,%ld,%ld,%.9g,%.9g,",static_cast<unsigned long long>(id),name,static_cast<unsigned long long>(objectRva),before[i].integer,after.integer,before[i].real,after.real);writeCsvString(presetDiffFile,before[i].text);fputc(',',presetDiffFile);writeCsvString(presetDiffFile,after.text);fputc('\n',presetDiffFile);}fflush(presetDiffFile);}
void discoverStaticCvars(){
 constexpr uintptr_t listHeadRva=0x31C04B0;
 trackedCvarObjects.clear();trackedCvarObjects.reserve(2048);
 uintptr_t object=*reinterpret_cast<volatile uintptr_t*>(base+listHeadRva);
 for(unsigned count=0;object&&count<8192;++count){
  if(object<base||object+0x80>=base+0x41CB000)break;
  uintptr_t rva=object-base;char name[128]{};
  if(!cvarNameFromObject(reinterpret_cast<void*>(object),name,sizeof name,nullptr))break;
  if(std::find(trackedCvarObjects.begin(),trackedCvarObjects.end(),rva)!=trackedCvarObjects.end())break;
  trackedCvarObjects.push_back(rva);
  object=*reinterpret_cast<volatile uintptr_t*>(object+0x78);
 }
 if(presetDiffFile){fprintf(presetDiffFile,"# cvar_registry_head,%llX\n# discovered_cvars,%zu\n",static_cast<unsigned long long>(*reinterpret_cast<volatile uintptr_t*>(base+listHeadRva)-base),trackedCvarObjects.size());fflush(presetDiffFile);}
}
void traceCvarSetterEvent(uint64_t id,unsigned depth,const char* event,const char* name,uintptr_t objectRva,LONG oldInt,int requested,LONG newInt,float oldFloat,float newFloat,unsigned char force,int accepted,uintptr_t caller,const uintptr_t* frames,unsigned frameCount){std::lock_guard<std::mutex> guard(presetTraceMutex);if(!presetTraceFile)return;fprintf(presetTraceFile,"%llu,%llu,%lu,%u,%s,%s,%llX,%ld,%d,%ld,%.9g,%.9g,%u,%d,%llX,\"",static_cast<unsigned long long>(id),static_cast<unsigned long long>(GetTickCount64()),GetCurrentThreadId(),depth,event,name,static_cast<unsigned long long>(objectRva),oldInt,requested,newInt,oldFloat,newFloat,static_cast<unsigned>(force),accepted,static_cast<unsigned long long>(caller));for(unsigned i=0;i<frameCount;++i)fprintf(presetTraceFile,"%s%llX",i?" ":"",static_cast<unsigned long long>(frames[i]));fprintf(presetTraceFile,"\"\n");fflush(presetTraceFile);}
bool readableRange(uintptr_t address,size_t size){if(!address||size>UINTPTR_MAX-address)return false;MEMORY_BASIC_INFORMATION mbi{};if(!VirtualQuery(reinterpret_cast<const void*>(address),&mbi,sizeof(mbi))||mbi.State!=MEM_COMMIT||(mbi.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;DWORD protection=mbi.Protect&0xFF;bool readable=protection==PAGE_READONLY||protection==PAGE_READWRITE||protection==PAGE_WRITECOPY||protection==PAGE_EXECUTE_READ||protection==PAGE_EXECUTE_READWRITE||protection==PAGE_EXECUTE_WRITECOPY;uintptr_t regionEnd=reinterpret_cast<uintptr_t>(mbi.BaseAddress)+mbi.RegionSize;return readable&&address+size<=regionEnd;}
void tracePresetCvarCallbacks(uint64_t id,void* object,const char* phase){if(!presetCallbackTraceFile||!object)return;uintptr_t objectAddress=reinterpret_cast<uintptr_t>(object);if(!readableRange(objectAddress+0x70,sizeof(uintptr_t)))return;uintptr_t head=*reinterpret_cast<volatile uintptr_t*>(objectAddress+0x70);std::lock_guard<std::mutex> guard(presetTraceMutex);if(!head){fprintf(presetCallbackTraceFile,"%llu,%llu,%lu,%s,%llX,0,-1,0,0,0,0,0,0\n",static_cast<unsigned long long>(id),static_cast<unsigned long long>(GetTickCount64()),GetCurrentThreadId(),phase,static_cast<unsigned long long>(objectAddress-base));fflush(presetCallbackTraceFile);return;}uintptr_t node=head;for(unsigned index=0;node&&index<64;++index){if(!readableRange(node,2*sizeof(uintptr_t))){fprintf(presetCallbackTraceFile,"%llu,%llu,%lu,%s,%llX,%llX,%u,%llX,0,0,0,0,0\n",static_cast<unsigned long long>(id),static_cast<unsigned long long>(GetTickCount64()),GetCurrentThreadId(),phase,static_cast<unsigned long long>(objectAddress-base),static_cast<unsigned long long>(head),index,static_cast<unsigned long long>(node));break;}uintptr_t callback=*reinterpret_cast<volatile uintptr_t*>(node);uintptr_t next=*reinterpret_cast<volatile uintptr_t*>(node+sizeof(uintptr_t));uintptr_t vtable=0,method=0,methodRva=0;if(callback&&readableRange(callback,sizeof(uintptr_t))){vtable=*reinterpret_cast<volatile uintptr_t*>(callback);if(vtable&&readableRange(vtable+2*sizeof(uintptr_t),sizeof(uintptr_t))){method=*reinterpret_cast<volatile uintptr_t*>(vtable+2*sizeof(uintptr_t));if(method>=base&&method<base+0x41CB000)methodRva=method-base;}}fprintf(presetCallbackTraceFile,"%llu,%llu,%lu,%s,%llX,%llX,%u,%llX,%llX,%llX,%llX,%llX,%llX\n",static_cast<unsigned long long>(id),static_cast<unsigned long long>(GetTickCount64()),GetCurrentThreadId(),phase,static_cast<unsigned long long>(objectAddress-base),static_cast<unsigned long long>(head),index,static_cast<unsigned long long>(node),static_cast<unsigned long long>(callback),static_cast<unsigned long long>(vtable),static_cast<unsigned long long>(method),static_cast<unsigned long long>(methodRva),static_cast<unsigned long long>(next));if(next==node)break;node=next;}fflush(presetCallbackTraceFile);}
void tracePresetNotifyManager(uint64_t id,void* manager,uint32_t flags,const char* event){
 if(!presetManagerTraceFile||!manager)return;
 uintptr_t managerAddress=reinterpret_cast<uintptr_t>(manager);
 if(!readableRange(managerAddress,0x44))return;
 uintptr_t vtable=*reinterpret_cast<volatile uintptr_t*>(managerAddress);
 uint32_t pendingBefore=*reinterpret_cast<volatile uint32_t*>(managerAddress+0x40);
 uintptr_t managerRva=managerAddress>=base&&managerAddress<base+0x41CB000?managerAddress-base:0;
 uintptr_t vtableRva=vtable>=base&&vtable<base+0x41CB000?vtable-base:0;
 std::lock_guard<std::mutex> guard(presetTraceMutex);
 if(strcmp(event,"notify_begin")==0&&std::find(loggedPresetManagerVtables.begin(),loggedPresetManagerVtables.end(),vtable)==loggedPresetManagerVtables.end()){
  loggedPresetManagerVtables.push_back(vtable);
  for(unsigned slot=0;slot<64&&readableRange(vtable+slot*sizeof(uintptr_t),sizeof(uintptr_t));++slot){uintptr_t target=*reinterpret_cast<volatile uintptr_t*>(vtable+slot*sizeof(uintptr_t));uintptr_t targetRva=target>=base&&target<base+0x41CB000?target-base:0;fprintf(presetManagerTraceFile,"%llu,%llu,%lu,vtable_entry,%llX,%llX,%llX,%llX,%u,%u,%u,%02X,%llX,%llX\n",static_cast<unsigned long long>(id),static_cast<unsigned long long>(GetTickCount64()),GetCurrentThreadId(),static_cast<unsigned long long>(managerAddress),static_cast<unsigned long long>(managerRva),static_cast<unsigned long long>(vtable),static_cast<unsigned long long>(vtableRva),flags,pendingBefore,pendingBefore,slot,static_cast<unsigned long long>(target),static_cast<unsigned long long>(targetRva));}
 }
 uint32_t pendingAfter=*reinterpret_cast<volatile uint32_t*>(managerAddress+0x40);
 fprintf(presetManagerTraceFile,"%llu,%llu,%lu,%s,%llX,%llX,%llX,%llX,%u,%u,%u,,,%llX\n",static_cast<unsigned long long>(id),static_cast<unsigned long long>(GetTickCount64()),GetCurrentThreadId(),event,static_cast<unsigned long long>(managerAddress),static_cast<unsigned long long>(managerRva),static_cast<unsigned long long>(vtable),static_cast<unsigned long long>(vtableRva),flags,pendingBefore,pendingAfter,static_cast<unsigned long long>(0));
 fflush(presetManagerTraceFile);
}
uintptr_t __fastcall presetNotifyHook(void* manager,uint32_t flags){
 if(!presetNotifyOriginal)return 0;
 if(!presetTraceActive)return presetNotifyOriginal(manager,flags);
 const LONG current=*reinterpret_cast<volatile LONG*>(base+0x3982E70+0x28);
 const float currentFloat=*reinterpret_cast<volatile float*>(base+0x3982E70+0x2c);
 uintptr_t frames[12]{};USHORT count=CaptureStackBackTrace(1,12,reinterpret_cast<void**>(frames),nullptr);
 for(USHORT i=0;i<count;++i){if(frames[i]>=base)frames[i]-=base;else frames[i]=0;}
 uintptr_t caller=reinterpret_cast<uintptr_t>(_ReturnAddress());if(caller>=base)caller-=base;
 tracePresetNotifyManager(presetTraceId,manager,flags,"notify_begin");
 traceCvarSetterEvent(presetTraceId,presetTraceDepth,"notify_begin","CVarChangeNotify",presetNotifyTarget-base,current,static_cast<int>(flags),current,currentFloat,currentFloat,0,-1,caller,frames,count);
 uintptr_t result=presetNotifyOriginal(manager,flags);
 tracePresetNotifyManager(presetTraceId,manager,flags,"notify_end");
 traceCvarSetterEvent(presetTraceId,presetTraceDepth,"notify_end","CVarChangeNotify",presetNotifyTarget-base,current,static_cast<int>(flags),*reinterpret_cast<volatile LONG*>(base+0x3982E70+0x28),currentFloat,*reinterpret_cast<volatile float*>(base+0x3982E70+0x2c),0,static_cast<int>(result),caller,nullptr,0);
 return result;
}
uintptr_t resolvePresetNotifyTarget(){
 constexpr uintptr_t managerSlotRva=0x228BAE8;
 const uintptr_t manager=*reinterpret_cast<volatile uintptr_t*>(base+managerSlotRva);
 if(manager<base||manager+0x10>=base+0x41CB000)return 0;
 const uintptr_t vtable=*reinterpret_cast<volatile uintptr_t*>(manager);
 if(vtable<base||vtable+0x90>=base+0x41CB000)return 0;
 const uintptr_t target=*reinterpret_cast<volatile uintptr_t*>(vtable+0x88);
 if(target<base||target>=base+0x41CB000)return 0;
 MEMORY_BASIC_INFORMATION mbi{};
 if(!VirtualQuery(reinterpret_cast<const void*>(target),&mbi,sizeof(mbi))||mbi.State!=MEM_COMMIT||!(mbi.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)))return 0;
 return target;
}
void tracePresetNotifyTarget(){if(!presetTraceFile)return;std::lock_guard<std::mutex> guard(presetTraceMutex);fprintf(presetTraceFile,"# notify_method_rva,%llX\n",static_cast<unsigned long long>(presetNotifyTarget?presetNotifyTarget-base:0));fflush(presetTraceFile);}
__declspec(noinline) unsigned char __fastcall nativeCvarSetterHook(void* object,int value,unsigned char force){
 if(!nativeCvarSetterOriginal)return 0;
 uintptr_t address=reinterpret_cast<uintptr_t>(object);
 if(!presetTraceActive&&address!=base+0x3982E70)return nativeCvarSetterOriginal(object,value,force);
 char name[128]{};uintptr_t objectRva=0;
 if(!cvarNameFromObject(object,name,sizeof name,&objectRva))return nativeCvarSetterOriginal(object,value,force);
 const bool isTarget=objectRva==0x3982E70&&strcmp(name,"r_viewDistPreset")==0;
 const bool starts=isTarget&&!presetTraceActive;
 const LONG oldInt=*reinterpret_cast<volatile LONG*>(address+0x28);
 const float oldFloat=*reinterpret_cast<volatile float*>(address+0x2c);
 if(starts){presetTraceActive=true;presetTraceId=static_cast<uint64_t>(InterlockedIncrement(&presetTraceSequence));}
 std::vector<CvarSample> presetBefore;if(starts)presetBefore=snapshotStaticCvars();
 if(starts)tracePresetCvarCallbacks(presetTraceId,object,"before_setter");
 const bool capture=presetTraceActive;const unsigned depth=presetTraceDepth;
 uintptr_t stack[12]{};USHORT count=0;uintptr_t caller=0;
 if(capture){caller=reinterpret_cast<uintptr_t>(_ReturnAddress());if(caller>=base)caller-=base;count=CaptureStackBackTrace(1,12,reinterpret_cast<void**>(stack),nullptr);for(USHORT i=0;i<count;++i){if(stack[i]>=base)stack[i]-=base;else stack[i]=0;}traceCvarSetterEvent(presetTraceId,depth,starts?"preset_begin":"setter_begin",name,objectRva,oldInt,value,oldInt,oldFloat,oldFloat,force,-1,caller,stack,count);}
 ++presetTraceDepth;unsigned char returnAl=nativeCvarSetterOriginal(object,value,force);--presetTraceDepth;
 const LONG newInt=*reinterpret_cast<volatile LONG*>(address+0x28);const float newFloat=*reinterpret_cast<volatile float*>(address+0x2c);
 if(starts)tracePresetCvarCallbacks(presetTraceId,object,"after_setter");
 if(starts)writePresetCvarDiff(presetTraceId,presetBefore);
 if(capture){traceCvarSetterEvent(presetTraceId,depth,starts?"preset_end":"setter_end",name,objectRva,oldInt,value,newInt,oldFloat,newFloat,force,returnAl,caller,nullptr,0);if(starts)presetTraceActive=false;}
 return returnAl;
}
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
bool snapshot(Capture& s){for(int attempt=0;attempt<3;++attempt){auto seq=read<unsigned>(0x20);if(seq&1)continue;s.ptr=read<uint64_t>(0x10);s.w=read<unsigned>(0x18);s.h=read<unsigned>(0x1c);for(int i=0;i<14;++i)s.f[i]=read<float>(0x40+i*4);MemoryBarrier();if(seq!=read<unsigned>(0x20))continue;if(!s.ptr||s.w<16||s.h<16||s.w>65536||s.h>65536)return false;for(auto v:s.f)if(!std::isfinite(v))return false;/* +0x3A6C is the engine's aspect ratio; +0x3A70 is vertical FOV in radians. */if(s.f[0]<.1||s.f[0]>10||s.f[1]<.01||s.f[1]>3.14)return false;for(int i=2;i<5;++i)if(fabsf(s.f[i])>1e7)return false;for(int i=5;i<14;i+=3){float n=s.f[i]*s.f[i]+s.f[i+1]*s.f[i+1]+s.f[i+2]*s.f[i+2];if(n<.85||n>1.15)return false;}auto r=s.f+5;float det=r[0]*(r[4]*r[8]-r[5]*r[7])-r[1]*(r[3]*r[8]-r[5]*r[6])+r[2]*(r[3]*r[7]-r[4]*r[6]);return det>.8&&det<1.2;}return false;}
bool key(int k);
void writeLodDiagnosticSample(uint64_t now,bool focused,const Capture& s){
 if(!lodLogFile||now-lodLastSample<100)return;lodLastSample=now;SYSTEMTIME t{};GetLocalTime(&t);XINPUT_STATE pad{};bool padOk=XInputGetState(0,&pad)==ERROR_SUCCESS;auto& g=pad.Gamepad;int meshMark=InterlockedExchange(reinterpret_cast<volatile LONG*>(&lodPendingMeshMark),0);
 unsigned readyMask=0;for(unsigned i=0;i<6;++i)if(renderCvars[static_cast<unsigned>(lodDistanceControls[i])].ready)readyMask|=1u<<i;
 fprintf(lodLogFile,"%llu,%04u-%02u-%02u %02u:%02u:%02u.%03u,%d,%d,%d,%d,%04X,%d,%d,%d,%d,%d,",
  static_cast<unsigned long long>(now),t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds,focused?1:0,key('Z')?1:0,key('W')?1:0,padOk?1:0,static_cast<unsigned>(padOk?g.wButtons:0),padOk&&((g.wButtons&XINPUT_GAMEPAD_RIGHT_THUMB)!=0)?1:0,padOk?static_cast<int>(g.sThumbRX):0,padOk?static_cast<int>(g.sThumbRY):0,padOk?static_cast<int>(g.bLeftTrigger):0,padOk?static_cast<int>(g.bRightTrigger):0);
 fprintf(lodLogFile,"%llX,%u,%u,%.6f,%.6f,%.6f,%.6f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%u,%.0f,%.1f,%s\n",
  static_cast<unsigned long long>(s.ptr),s.w,s.h,aspect,pose.xyz[0],pose.xyz[1],pose.xyz[2],pose.fov,renderControlValue(RenderControl::StaticLodZoomMultiplier),renderControlValue(RenderControl::LodForceVisibleDistance),renderControlValue(RenderControl::LodDistance0),renderControlValue(RenderControl::LodDistance1),renderControlValue(RenderControl::LodDistance2),renderControlValue(RenderControl::LodDistance3),renderControlValue(RenderControl::LodDistanceFar),readyMask,renderControlValue(RenderControl::ViewDistancePreset),renderControlValue(RenderControl::FarZClipDistance),meshMark==1?"mesh_absent":meshMark==2?"mesh_visible":"");
 fflush(lodLogFile);
}
void publish(bool seed){float f[14];f[1]=rad(pose.fov);f[0]=2*atanf(tanf(f[1]/2)*aspect);memcpy(f+2,pose.xyz,12);native(q,f+5);InterlockedIncrement(reinterpret_cast<LONG*>(cameraData+4));for(int i=0;i<14;++i)write<float>(0x80+i*4,f[i]);MemoryBarrier();InterlockedIncrement(reinterpret_cast<LONG*>(cameraData+4));if(seed)memcpy(cameraData+0x200,f,sizeof f);}
void disable(){write<unsigned>(0,0);MemoryBarrier();write<uint64_t>(8,0);cameraEnabled=false;cameraNeedsNeutralInput=false;cameraNeutralInputFrameSeen=false;}
bool key(int k){return (GetAsyncKeyState(k)&0x8000)!=0;}
bool edge(int k){static bool down[256]{};bool d=key(k),r=d&&!down[k];down[k]=d;return r;}
float stick(SHORT value,SHORT deadzone){float v=float(value)/32767.f;float a=fabsf(v);return a<=float(deadzone)/32767.f?0.f:std::copysign((a-float(deadzone)/32767.f)/(1.f-float(deadzone)/32767.f),v);}
bool cameraControlsNeutral(const XINPUT_STATE& gamepadState){
 if(controlMode==1){
  if(!gamepadConnected)return false;
  const auto& g=gamepadState.Gamepad;
  constexpr WORD activeButtons=XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT|XINPUT_GAMEPAD_A|XINPUT_GAMEPAD_B|XINPUT_GAMEPAD_X;
  return stick(g.sThumbLX,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)==0.f&&stick(g.sThumbLY,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)==0.f&&stick(g.sThumbRX,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)==0.f&&stick(g.sThumbRY,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)==0.f&&g.bLeftTrigger<5&&g.bRightTrigger<5&&(g.wButtons&activeButtons)==0;
 }
 constexpr int controls[]={VK_INSERT,VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4,VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9,VK_ADD,VK_SUBTRACT};
 for(int control:controls)if(key(control))return false;
 return true;
}
bool resolveCvar(uintptr_t objectRva,const char* expected,volatile LONG** value,volatile float** floatValue=nullptr){auto object=base+objectRva;auto name=*reinterpret_cast<volatile uint64_t*>(object+0x30);size_t length=strlen(expected);if(name<base||name>=base+0x41CB000||length+1>base+0x41CB000-name)return false;if(memcmp((const void*)name,expected,length+1))return false;*value=reinterpret_cast<volatile LONG*>(object+0x28);if(floatValue)*floatValue=reinterpret_cast<volatile float*>(object+0x2c);return true;}
bool resolveHudCvars(){for(auto& c:hudCvars){if(!resolveCvar(c.objectRva,c.name,&c.integer,&c.real))return false;c.text=reinterpret_cast<volatile char*>(base+c.objectRva+0x10);bool terminated=false;for(int i=0;i<16;++i)if(c.text[i]==0){terminated=true;break;}if(!terminated)return false;}return true;}
void resolveRenderCvars(){for(auto& c:renderCvars){c.ready=resolveCvar(c.objectRva,c.name,&c.integer,&c.real);if(c.ready){c.text=reinterpret_cast<volatile char*>(base+c.objectRva+0x10);bool terminated=false;for(int i=0;i<16;++i){c.originalText[i]=c.text[i];if(!c.originalText[i]){terminated=true;break;}}if(!terminated){c.ready=false;c.text=nullptr;continue;}c.originalInteger=*c.integer;c.originalReal=*c.real;}}nativeViewDistanceSetterReadyFlag=renderCvars[static_cast<unsigned>(RenderControl::ViewDistancePreset)].ready&&*reinterpret_cast<const unsigned char*>(base+0x1028E0)==0x40&&*reinterpret_cast<const unsigned char*>(base+0x1028E1)==0x57&&*reinterpret_cast<const unsigned char*>(base+0x1028E2)==0x48;}
void resolveShadowCascade(){volatile LONG* unused=nullptr;if(!resolveCvar(0x397A350,"r_shadowCascadeFars",&unused))return;shadowCascadeText=reinterpret_cast<volatile char*>(base+0x397A360);char value[16]{};for(int i=0;i<15;++i){value[i]=shadowCascadeText[i];if(!value[i])break;}int a,b,c,d;if(sscanf_s(value,"%d %d %d %d",&a,&b,&c,&d)!=4)return;strncpy_s(originalShadowCascadeText,value,_TRUNCATE);shadowCascadeReadyFlag=true;}
void loadControlMode(){DWORD length=GetEnvironmentVariableW(L"LOCALAPPDATA",settingsPath,MAX_PATH);if(!length||length>=MAX_PATH-32){settingsPath[0]=0;controlMode=0;return;}wcscat_s(settingsPath,L"\\VoidCamera");CreateDirectoryW(settingsPath,nullptr);wcscat_s(settingsPath,L"\\settings.ini");controlMode=static_cast<int>(GetPrivateProfileIntW(L"Input",L"ControlMode",0,settingsPath));if(controlMode<0||controlMode>1)controlMode=0;rotationSpeed=std::clamp(static_cast<int>(GetPrivateProfileIntW(L"Camera",L"RotationSpeed",950,settingsPath)),45,1800);}
void saveControlModeInternal(){if(!settingsPath[0])return;wchar_t value[2]{static_cast<wchar_t>(L'0'+controlMode),0};WritePrivateProfileStringW(L"Input",L"ControlMode",value,settingsPath);}
void saveRotationSpeedInternal(){if(!settingsPath[0])return;wchar_t value[16]{};swprintf_s(value,L"%d",rotationSpeed);WritePrivateProfileStringW(L"Camera",L"RotationSpeed",value,settingsPath);}
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
 openPresetTrace();
 discoverStaticCvars();
 timeStopReady=resolveCvar(0x39A0E10,"g_stopTime",&stopTimeValue)&&resolveCvar(0x39A60E0,"g_freezeTime",&freezeTimeValue);
 hudReady=resolveHudCvars();
 focusLossReady=resolveFocusLossValue();
 if(focusLossReady){originalPauseOnFocusLoss=*pauseOnFocusLossValue;InterlockedExchange8(reinterpret_cast<volatile char*>(pauseOnFocusLossValue),0);focusLossEnabled=true;}
 if(MH_Initialize()!=MH_OK||MH_CreateHook((void*)(base+0x375711),(void*)cameraHook,&cameraOriginal)!=MH_OK||!nativeViewDistanceSetterReadyFlag||MH_CreateHook((void*)(base+0x1028E0),(void*)nativeCvarSetterHook,reinterpret_cast<void**>(&nativeCvarSetterOriginal))!=MH_OK){status->error=2;return 1;}
 presetNotifyTarget=resolvePresetNotifyTarget();
 if(presetNotifyTarget)MH_CreateHook(reinterpret_cast<void*>(presetNotifyTarget),reinterpret_cast<void*>(presetNotifyHook),reinterpret_cast<void**>(&presetNotifyOriginal));
 if(MH_EnableHook((void*)(base+0x375711))!=MH_OK||MH_EnableHook((void*)(base+0x1028E0))!=MH_OK){status->error=3;return 1;}
 if(presetNotifyTarget&&presetNotifyOriginal)MH_EnableHook(reinterpret_cast<void*>(presetNotifyTarget));
 tracePresetNotifyTarget();
 if(!overlayInstall()){MH_DisableHook((void*)(base+0x375711));status->error=4;return 1;}
 status->ready=1;for(;;){cameraFrame();Sleep(8);} }
}
void saveControlMode(){saveControlModeInternal();}
void saveRotationSpeed(){std::lock_guard<std::recursive_mutex> g(cameraMutex);rotationSpeed=std::clamp(rotationSpeed,45,1800);saveRotationSpeedInternal();}
void cameraToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(cameraEnabled){disable();return;}Capture s;if(read<unsigned>(0x2c)||*(volatile unsigned char*)(base+0x39a529c)||!snapshot(s))return;target=s.ptr;if(!renderAspectReady)aspect=float(s.w)/s.h;memcpy(pose.xyz,s.f+2,12);eulers(s.f+5);q=fromEuler();pose.fov=std::clamp(deg(s.f[1]),5.f,150.f);publish(true);write<uint64_t>(8,target);MemoryBarrier();write<unsigned>(0,1);cameraEnabled=true;cameraNeedsNeutralInput=true;cameraNeutralInputFrameSeen=false;lastHitTime=GetTickCount64();}
void timeStopToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!timeStopReady)return;if(!timeStopped){originalStopTime=*stopTimeValue;originalFreezeTime=*freezeTimeValue;InterlockedExchange(stopTimeValue,1);InterlockedExchange(freezeTimeValue,1);timeStopped=true;}else{InterlockedExchange(stopTimeValue,originalStopTime);InterlockedExchange(freezeTimeValue,originalFreezeTime);timeStopped=false;}}
void hudToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!hudReady)return;if(!hudHidden){for(auto& c:hudCvars){c.originalInteger=*c.integer;c.originalReal=*c.real;for(int i=0;i<16;++i)c.originalText[i]=c.text[i];c.text[0]='0';for(int i=1;i<16;++i)c.text[i]=0;MemoryBarrier();InterlockedExchange(c.integer,0);*c.real=0;}hudHidden=true;}else{for(auto& c:hudCvars){for(int i=0;i<16;++i)c.text[i]=c.originalText[i];MemoryBarrier();InterlockedExchange(c.integer,c.originalInteger);*c.real=c.originalReal;}hudHidden=false;}}
void focusLossToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!focusLossReady)return;if(!focusLossEnabled){InterlockedExchange8(reinterpret_cast<volatile char*>(pauseOnFocusLossValue),0);focusLossEnabled=true;}else{InterlockedExchange8(reinterpret_cast<volatile char*>(pauseOnFocusLossValue),static_cast<char>(originalPauseOnFocusLoss));focusLossEnabled=false;}}
void cameraPublish(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!cameraEnabled)return;for(float x:pose.xyz)if(!std::isfinite(x)||fabsf(x)>=1e7)return;for(float x:pose.pyr)if(!std::isfinite(x))return;pose.fov=std::clamp(pose.fov,5.f,150.f);pose.speed=std::clamp(pose.speed,.001f,500.f);q=fromEuler();publish(false);}
void cameraSetRenderResolution(unsigned width,unsigned height){if(width<16||height<16)return;std::lock_guard<std::recursive_mutex> g(cameraMutex);aspect=float(width)/float(height);renderAspectReady=true;}
bool lodTestEntityLod0Forced(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return lodForceActive;}
bool lodTestForceEntityLod0(){std::lock_guard<std::recursive_mutex> g(cameraMutex);for(auto id:lodDistanceControls)if(!renderControlReady(id))return false;if(!lodForceActive){for(unsigned i=0;i<6;++i)lodPreForceValues[i]=renderControlValue(lodDistanceControls[i]);for(unsigned i=0;i<6;++i)renderControlSetValue(lodDistanceControls[i],lodForceValues[i]);lodForceActive=true;}else{for(unsigned i=0;i<6;++i)renderControlSetValue(lodDistanceControls[i],lodPreForceValues[i]);lodForceActive=false;}return true;}
bool lodDiagnosticLogActive(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return lodLogFile!=nullptr;}
bool lodDiagnosticLogToggle(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(lodLogFile){fprintf(lodLogFile,"# session stopped at %llu ms\n",static_cast<unsigned long long>(GetTickCount64()));fclose(lodLogFile);lodLogFile=nullptr;return false;}wchar_t path[MAX_PATH]{};DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,MAX_PATH);if(!n||n>=MAX_PATH-48)return false;wcscat_s(path,L"\\VoidCamera");CreateDirectoryW(path,nullptr);wcscpy_s(lodLogPath,path);wcscat_s(lodLogPath,L"\\lod-mesh-test.csv");if(_wfopen_s(&lodLogFile,lodLogPath,L"w")!=0||!lodLogFile){lodLogFile=nullptr;return false;}fprintf(lodLogFile,"tick_ms,local_time,game_focused,zoom_key_Z,move_key_W,gamepad_connected,gamepad_buttons_hex,right_thumb_click,right_stick_x,right_stick_y,left_trigger,right_trigger,view_ptr,view_width,view_height,render_aspect,camera_x,camera_y,camera_z,vertical_fov,r_lodZoomMultiplicator,lod_forcevisibledistance,lod_dist_level0,lod_dist_level1,lod_dist_level2,lod_dist_level3,lod_dist_far,lod_ready_mask,r_viewDistPreset,r_zfar,mesh_event\n# session started at %llu ms\n",static_cast<unsigned long long>(GetTickCount64()));fflush(lodLogFile);lodLastSample=0;lodPendingMeshMark=0;return true;}
void lodDiagnosticLogMark(bool visible){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(lodLogFile)InterlockedExchange(reinterpret_cast<volatile LONG*>(&lodPendingMeshMark),visible?2:1);}
void cameraFrame(){std::lock_guard<std::recursive_mutex> g(cameraMutex);static uint64_t last=GetTickCount64();uint64_t now=GetTickCount64();float dt=std::min((now-last)*.001f,.05f);last=now;DWORD fg=0;GetWindowThreadProcessId(GetForegroundWindow(),&fg);bool focused=fg==GetCurrentProcessId();bool ins=edge(VK_INSERT),del=edge(VK_DELETE),pause=edge(VK_NUMPAD0),home=edge(VK_HOME);if(focused&&ins&&!igcsSessionActive)cameraToggle();if(focused&&del)hudToggle();if(focused&&pause)timeStopToggle();if(focused&&home)cameraLocked=!cameraLocked;XINPUT_STATE gamepadState{};gamepadConnected=controlMode==1&&XInputGetState(0,&gamepadState)==ERROR_SUCCESS;
 Capture s{};cameraReady=snapshot(s);if(cameraReady){if(!renderAspectReady)aspect=float(s.w)/s.h;if(!cameraEnabled){memcpy(pose.xyz,s.f+2,12);eulers(s.f+5);q=fromEuler();pose.fov=deg(s.f[1]);}}
 if(cameraEnabled){unsigned hits=read<unsigned>(0x24);if(hits!=lastHits||!focused){lastHitTime=now;lastHits=hits;}if(*(volatile unsigned char*)(base+0x39a529c)||(!igcsSessionActive&&!timeStopped&&now-lastHitTime>3000))disable();}
 if(cameraEnabled){if(cameraNeedsNeutralInput&&focused){if(!cameraControlsNeutral(gamepadState))cameraNeutralInputFrameSeen=false;else if(cameraNeutralInputFrameSeen){cameraNeedsNeutralInput=false;cameraNeutralInputFrameSeen=false;}else cameraNeutralInputFrameSeen=true;}if(focused&&!status->overlay&&!cameraLocked&&!igcsSessionActive&&!cameraNeedsNeutralInput){float factor=key(VK_MENU)?.01f:key(VK_CONTROL)?.1f:key(VK_SHIFT)?5.f:1.f;float a=rad(rotationSpeed*dt*factor);float p=0,y=0,r=0,f=0,right=0,up=0,fov=0;
  if(controlMode==1){if(gamepadConnected){auto& gpad=gamepadState.Gamepad;p=-stick(gpad.sThumbRY,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);y=stick(gpad.sThumbRX,XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);f=stick(gpad.sThumbLY,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);right=stick(gpad.sThumbLX,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);up=(float(gpad.bLeftTrigger)-float(gpad.bRightTrigger))/255.f;factor*=gpad.wButtons&XINPUT_GAMEPAD_A?5.f:gpad.wButtons&XINPUT_GAMEPAD_X?.1f:1.f;fov=(gpad.wButtons&XINPUT_GAMEPAD_DPAD_DOWN?1.f:0.f)-(gpad.wButtons&XINPUT_GAMEPAD_DPAD_UP?1.f:0.f);r=(gpad.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT?1.f:0.f)-(gpad.wButtons&XINPUT_GAMEPAD_DPAD_LEFT?1.f:0.f);if(gpad.wButtons&XINPUT_GAMEPAD_B)pose.fov=60.f;}}
  else{p=float(key(VK_DOWN))-key(VK_UP);y=float(key(VK_RIGHT))-key(VK_LEFT);r=float(key(VK_NUMPAD1))-key(VK_NUMPAD3);f=float(key(VK_NUMPAD8))-key(VK_NUMPAD5);right=float(key(VK_NUMPAD6))-key(VK_NUMPAD4);up=float(key(VK_NUMPAD9))-key(VK_NUMPAD7);fov=float(key(VK_ADD))-key(VK_SUBTRACT);if(key(VK_NUMPAD2)){pose.pyr[2]=0;q=fromEuler();}}
  // Keep pitch and yaw as independent pose angles; local roll must not swap them.
  pose.pyr[0]=std::clamp(pose.pyr[0]+p*a,-89.9f,89.9f);
  pose.pyr[1]=std::remainder(pose.pyr[1]-y*a,360.f);
  pose.pyr[2]=std::remainder(pose.pyr[2]+r*a,360.f);
  q=fromEuler();
  Q moveOrientation=norm(mul(axis(2,rad(pose.pyr[1])),axis(1,rad(pose.pyr[0]))));
  float moveMatrix[9];native(moveOrientation,moveMatrix);
  float len=sqrtf(f*f+right*right+up*up);if(len>1){f/=len;right/=len;up/=len;len=1;}
  if(len>0){float step=pose.speed*factor*dt;for(int i=0;i<3;++i)pose.xyz[i]+=(moveMatrix[i]*f-moveMatrix[i+3]*right+(i==2?up:0))*step;}
  pose.fov=std::clamp(pose.fov+fov*20*dt*factor,5.f,150.f);
 }publish(false);}
 if(status->command!=status->ack){LONG cmd=status->command;if(cmd==1&&!cameraEnabled)cameraToggle();if(cmd==2&&cameraEnabled)disable();if(cmd==3){memcpy(&pose,status->args,sizeof(Pose));cameraPublish();}if(cmd==4)status->overlay=!status->overlay;if(cmd==5&&!timeStopped)timeStopToggle();if(cmd==6&&timeStopped)timeStopToggle();status->ack=cmd;}
 status->enabled=cameraEnabled;status->captures=read<unsigned>(0x24);status->applied=read<unsigned>(0x28);status->cached=read<unsigned>(0x30);status->view=read<uint64_t>(0x10);status->pose=pose;
 status->timeStopReady=timeStopReady;status->timeStopped=timeStopped;status->hudReady=hudReady;status->hudHidden=hudHidden;status->focusLossReady=focusLossReady;status->focusLossEnabled=focusLossEnabled;
 writeLodDiagnosticSample(now,focused,s);status->locked=cameraLocked;status->controlMode=controlMode;status->aspect=aspect;status->horizontalFov=deg(2*atanf(tanf(rad(pose.fov)*.5f)*aspect));float connectorRotation[9];native(q,connectorRotation);updateIgcsConnector(cameraEnabled,cameraLocked||igcsSessionActive,pose,q,connectorRotation);InterlockedExchange(&status->igcsConnectorReady,connectorData?1:0);
}
bool renderControlReady(RenderControl id){auto i=static_cast<unsigned>(id);return i<static_cast<unsigned>(RenderControl::Count)&&renderCvars[i].ready;}
bool renderControlEnabled(RenderControl id){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready)return false;auto& c=renderCvars[i];return ((*c.integer!=0)!=c.inverse);}
float renderControlValue(RenderControl id){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready)return 0;auto& c=renderCvars[i];return c.floating?*c.real:static_cast<float>(*c.integer);}
namespace {
void writeRenderCvar(RenderCvar& c,float value){char text[16]{};int n=c.floating?sprintf_s(text,"%.7g",static_cast<double>(value)):sprintf_s(text,"%d",static_cast<int>(value));if(n<0||n>=16)return;memcpy(const_cast<char*>(c.text),text,sizeof text);MemoryBarrier();InterlockedExchange(c.integer,static_cast<LONG>(value));*c.real=value;}
bool writeNativeViewDistancePreset(RenderCvar& c,float value){if(!nativeViewDistanceSetterReadyFlag||c.objectRva!=0x3982E70)return false;int preset=static_cast<int>(std::lround(value));if(preset<0||preset>5)return false;if(*c.integer==preset)return true;using SetCvarFn=unsigned char(__fastcall*)(void*,int,unsigned char);auto setCvar=reinterpret_cast<SetCvarFn>(base+0x1028E0);setCvar(reinterpret_cast<void*>(base+c.objectRva),preset,1);return *c.integer==preset;}
}
void renderControlSetEnabled(RenderControl id,bool value){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready)return;auto& c=renderCvars[i];LONG raw=(value!=c.inverse)?1:0;writeRenderCvar(c,static_cast<float>(raw));}
void renderControlSetValue(RenderControl id,float value){std::lock_guard<std::recursive_mutex> g(cameraMutex);auto i=static_cast<unsigned>(id);if(i>=static_cast<unsigned>(RenderControl::Count)||!renderCvars[i].ready||!std::isfinite(value))return;if(id==RenderControl::ViewDistancePreset){writeNativeViewDistancePreset(renderCvars[i],value);return;}writeRenderCvar(renderCvars[i],value);}
bool nativeViewDistanceSetterReady(){return nativeViewDistanceSetterReadyFlag;}
bool shadowCascadeReady(){return shadowCascadeReadyFlag;}
int shadowCascadeFarDistance(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!shadowCascadeReadyFlag)return 0;char value[16]{};for(int i=0;i<15;++i){value[i]=shadowCascadeText[i];if(!value[i])break;}int a,b,c,d;if(sscanf_s(value,"%d %d %d %d",&a,&b,&c,&d)!=4)return 0;return d;}
void shadowCascadeSetFarDistance(int distance){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(!shadowCascadeReadyFlag||distance<1||distance>999)return;char current[16]{};for(int i=0;i<15;++i){current[i]=shadowCascadeText[i];if(!current[i])break;}int a,b,c,d;if(sscanf_s(current,"%d %d %d %d",&a,&b,&c,&d)!=4||distance<=c)return;char updated[16]{};int length=sprintf_s(updated,"%d %d %d %d",a,b,c,distance);if(length<0||length>=16)return;memcpy(const_cast<char*>(shadowCascadeText),updated,sizeof updated);MemoryBarrier();}
void renderControlRestore(){std::lock_guard<std::recursive_mutex> g(cameraMutex);for(size_t i=0;i<sizeof(renderCvars)/sizeof(renderCvars[0]);++i){auto& c=renderCvars[i];if(!c.ready)continue;if(i==static_cast<unsigned>(RenderControl::ViewDistancePreset)&&nativeViewDistanceSetterReadyFlag){using SetCvarFn=unsigned char(__fastcall*)(void*,int,unsigned char);auto setCvar=reinterpret_cast<SetCvarFn>(base+0x1028E0);setCvar(reinterpret_cast<void*>(base+c.objectRva),c.originalInteger,1);}else{memcpy(const_cast<char*>(c.text),c.originalText,sizeof c.originalText);MemoryBarrier();InterlockedExchange(c.integer,c.originalInteger);*c.real=c.originalReal;}}if(shadowCascadeReadyFlag)memcpy(const_cast<char*>(shadowCascadeText),originalShadowCascadeText,sizeof originalShadowCascadeText);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){DisableThreadLibraryCalls(h);HANDLE t=CreateThread(nullptr,0,run,nullptr,0,nullptr);if(t)CloseHandle(t);}return TRUE;}
