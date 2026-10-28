#include "camera.h"
#include <d3d11.h>
#include <dxgi.h>
#include <MinHook.h>
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "sampling_history.h"
#include <locale.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
namespace {
using Present=HRESULT(WINAPI*)(IDXGISwapChain*,UINT,UINT);
using Resize=HRESULT(WINAPI*)(IDXGISwapChain*,UINT,UINT,UINT,DXGI_FORMAT,UINT);
Present originalPresent;Resize originalResize;
using Warp=BOOL(WINAPI*)(int,int);Warp originalWarp;
using Clip=BOOL(WINAPI*)(const RECT*);Clip originalClip;
ID3D11Device* device;ID3D11DeviceContext* context;IDXGISwapChain* active;
HWND window;WNDPROC previous;
std::recursive_mutex renderMutex;
constexpr UINT WM_VOIDCAMERA_APPLY_HOTSAMPLE=WM_APP+0x451;
LONG pendingHotsampleWidth=0,pendingHotsampleHeight=0,hotsampleApplyResult=0;
LONG requestedHotsampleWidth=0,requestedHotsampleHeight=0;
int clientWidth=0,clientHeight=0,renderWidth=0,renderHeight=0,targetWidth=0,targetHeight=0,defaultWidth=0,defaultHeight=0;
ULONGLONG hotsampleRequestedAt=0;
bool hotsampleInitialized=false,swapChainWindowed=false;
ResolutionHistory resolutionHistory;bool resolutionHistoryLoaded=false,resolutionHistorySaveFailed=false;
unsigned historyProcessedSequence=0;LONG successfulResizeSequence=0;
int historyRequestWidth=0,historyRequestHeight=0;
bool visible(){return status&&status->overlay;}
BOOL WINAPI warp(int x,int y){return visible()?TRUE:originalWarp(x,y);}
BOOL WINAPI clip(const RECT* r){return originalClip(visible()?nullptr:r);}
bool windowHasNormalFrame(HWND h){DWORD style=static_cast<DWORD>(GetWindowLongPtrW(h,GWL_STYLE));return (style&WS_CAPTION)!=0;}
bool resizeClientArea(HWND h,int width,int height){
 if(!IsWindow(h)||width<320||height<240||width>16384||height>16384)return false;
 if(!swapChainWindowed){if(!active||FAILED(active->SetFullscreenState(FALSE,nullptr)))return false;DXGI_SWAP_CHAIN_DESC desc{};if(FAILED(active->GetDesc(&desc))||!desc.Windowed)return false;swapChainWindowed=true;}
 if(IsZoomed(h))ShowWindow(h,SW_RESTORE);
 DWORD style=static_cast<DWORD>(GetWindowLongPtrW(h,GWL_STYLE)),exStyle=static_cast<DWORD>(GetWindowLongPtrW(h,GWL_EXSTYLE));
 RECT r{0,0,width,height};BOOL adjusted=FALSE;
 using AdjustForDpi=BOOL(WINAPI*)(LPRECT,DWORD,BOOL,DWORD,UINT);
 auto adjustForDpi=reinterpret_cast<AdjustForDpi>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"AdjustWindowRectExForDpi"));
 using DpiForWindow=UINT(WINAPI*)(HWND);
 auto dpiForWindow=reinterpret_cast<DpiForWindow>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForWindow"));
 if(adjustForDpi&&dpiForWindow)adjusted=adjustForDpi(&r,style,FALSE,exStyle,dpiForWindow(h));
 if(!adjusted)adjusted=AdjustWindowRectEx(&r,style,FALSE,exStyle);
 if(!adjusted)return false;
 int outerWidth=r.right-r.left,outerHeight=r.bottom-r.top;
 if(!SetWindowPos(h,nullptr,0,0,outerWidth,outerHeight,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_FRAMECHANGED))return false;
 RECT actual{};if(!GetClientRect(h,&actual))return false;
 // Reconcile any DPI virtualization or game-specific non-client frame adjustment.
 for(int attempt=0;attempt<3&&(actual.right-actual.left!=width||actual.bottom-actual.top!=height);++attempt){
  outerWidth+=width-(actual.right-actual.left);outerHeight+=height-(actual.bottom-actual.top);
  if(!SetWindowPos(h,nullptr,0,0,outerWidth,outerHeight,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER))break;
  if(!GetClientRect(h,&actual))break;
 }
 clientWidth=actual.right-actual.left;clientHeight=actual.bottom-actual.top;
 return clientWidth==width&&clientHeight==height;
}
LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){
 if((m==WM_KEYDOWN||m==WM_SYSKEYDOWN)&&w==VK_F10&&!(l&(1LL<<30))){status->overlay=!status->overlay;if(status->overlay&&originalClip)originalClip(nullptr);return 0;}
 if((m==WM_KEYUP||m==WM_SYSKEYUP)&&w==VK_F10)return 0;
 if(m==WM_VOIDCAMERA_APPLY_HOTSAMPLE){int width=InterlockedExchange(&pendingHotsampleWidth,0),height=InterlockedExchange(&pendingHotsampleHeight,0);bool ok=resizeClientArea(h,width,height);if(ok){InterlockedExchange(&requestedHotsampleWidth,width);InterlockedExchange(&requestedHotsampleHeight,height);InterlockedIncrement(&successfulResizeSequence);}InterlockedExchange(&hotsampleApplyResult,ok?2:-1);return 0;}
 if(m==WM_GETMINMAXINFO){LRESULT result=CallWindowProcW(previous,h,m,w,l);auto info=reinterpret_cast<MINMAXINFO*>(l);if(info){info->ptMaxTrackSize.x=std::max<LONG>(info->ptMaxTrackSize.x,16384);info->ptMaxTrackSize.y=std::max<LONG>(info->ptMaxTrackSize.y,16384);}return result;}
 std::lock_guard<std::recursive_mutex> lock(renderMutex);
 ImGui_ImplWin32_WndProcHandler(h,m,w,l);
 if(visible()&&((m>=WM_MOUSEFIRST&&m<=WM_MOUSELAST)||(m>=WM_KEYFIRST&&m<=WM_KEYLAST)||m==WM_CHAR||m==WM_INPUT)){if(m==WM_INPUT)return DefWindowProcW(h,m,w,l);return 0;}
 return CallWindowProcW(previous,h,m,w,l);
}
void helpMarker(const char*){}
void loadResolutionHistory(){if(resolutionHistoryLoaded)return;HMODULE module=nullptr;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&loadResolutionHistory),&module);wchar_t path[MAX_PATH]{};GetModuleFileNameW(module,path,MAX_PATH);auto slash=wcsrchr(path,L'\\');if(slash)wcscpy_s(slash+1,MAX_PATH-size_t(slash+1-path),L"VoidCamera-Hotsampling.ini");resolutionHistory.load(path);resolutionHistoryLoaded=true;}
void saveAppliedResolutionHistory(){unsigned sequence=static_cast<unsigned>(InterlockedCompareExchange(&successfulResizeSequence,0,0));if(sequence==historyProcessedSequence||!resolutionHistoryLoaded)return;if(renderWidth!=InterlockedCompareExchange(&requestedHotsampleWidth,0,0)||renderHeight!=InterlockedCompareExchange(&requestedHotsampleHeight,0,0))return;historyRequestWidth=renderWidth;historyRequestHeight=renderHeight;resolutionHistory.remember(unsigned(renderWidth),unsigned(renderHeight));HMODULE module=nullptr;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&loadResolutionHistory),&module);wchar_t path[MAX_PATH]{};GetModuleFileNameW(module,path,MAX_PATH);auto slash=wcsrchr(path,L'\\');if(slash)wcscpy_s(slash+1,MAX_PATH-size_t(slash+1-path),L"VoidCamera-Hotsampling.ini");resolutionHistorySaveFailed=!resolutionHistory.save(path);historyProcessedSequence=sequence;InterlockedExchange(&hotsampleApplyResult,3);}
void renderToggle(const char* label,RenderControl id,const char* help){bool ready=renderControlReady(id);ImGui::BeginDisabled(!ready);bool value=renderControlEnabled(id);if(ImGui::Checkbox(label,&value))renderControlSetEnabled(id,value);ImGui::EndDisabled();helpMarker(help);}
void renderFloat(const char* label,RenderControl id,float speed,float min,float max,const char* format,const char* help){bool ready=renderControlReady(id);ImGui::BeginDisabled(!ready);float value=renderControlValue(id);ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x*.55f);if(ImGui::DragFloat(label,&value,speed,min,max,format))renderControlSetValue(id,std::clamp(value,min,max));ImGui::EndDisabled();helpMarker(help);}
void queueHotsampleResize(int width,int height){if(!window||width<320||height<240||width>16384||height>16384)return;InterlockedExchange(&pendingHotsampleWidth,width);InterlockedExchange(&pendingHotsampleHeight,height);hotsampleRequestedAt=GetTickCount64();InterlockedExchange(&hotsampleApplyResult,1);if(!PostMessageW(window,WM_VOIDCAMERA_APPLY_HOTSAMPLE,0,0))InterlockedExchange(&hotsampleApplyResult,-1);}
void hotsamplingTab(){
 loadResolutionHistory();saveAppliedResolutionHistory();
 ImGui::TextUnformatted("HOT SAMPLING");ImGui::Separator();
 ImGui::TextWrapped("Resize the game's client area to render at a custom resolution and aspect ratio.");
 ImGui::SeparatorText("Current output");
 ImGui::Text("Rendered resolution: %d x %d px",renderWidth,renderHeight);
 ImGui::Text("Window client area: %d x %d px",clientWidth,clientHeight);
 ImGui::Text("Camera AR %.4f:1  |  FOV H %.2f deg / V %.2f deg",status->aspect,status->horizontalFov,pose.fov);
 ImGui::TextDisabled("Swap chain: %s",swapChainWindowed?"Windowed":"Fullscreen");
 ImGui::SeparatorText("Target size");
 ImGui::SetNextItemWidth(320);ImGui::InputInt("Width (px)",&targetWidth,64,256);
 ImGui::SetNextItemWidth(320);ImGui::InputInt("Height (px)",&targetHeight,64,256);
 ImGui::TextDisabled("Target AR: %.4f:1",targetHeight>0?float(targetWidth)/targetHeight:0.f);
 bool validTarget=targetWidth>=320&&targetWidth<=16384&&targetHeight>=240&&targetHeight<=16384;bool windowCanResize=swapChainWindowed||windowHasNormalFrame(window);bool canResize=validTarget&&windowCanResize;ImGui::BeginDisabled(!canResize);
 if(ImGui::Button("Apply resolution"))queueHotsampleResize(targetWidth,targetHeight);ImGui::EndDisabled();ImGui::SameLine();
 if(ImGui::Button("Use default res")){targetWidth=defaultWidth;targetHeight=defaultHeight;}
 if(!windowCanResize)ImGui::TextWrapped("Switch the game to Windowed mode. Apply is enabled for a normal titled game window.");
 else if(!validTarget)ImGui::TextWrapped("Enter a client size between 320 x 240 and 16384 x 16384 pixels.");
 LONG result=InterlockedCompareExchange(&hotsampleApplyResult,0,0);
 if(result==2&&GetTickCount64()-hotsampleRequestedAt>5000){InterlockedExchange(&hotsampleApplyResult,-2);result=-2;}
 if(result==1)ImGui::TextDisabled("Resize queued...");else if(result==2)ImGui::TextDisabled("Window resized; waiting for render resolution...");else if(result==3)ImGui::TextDisabled("Applied: %d x %d px",historyRequestWidth,historyRequestHeight);else if(result==-1)ImGui::TextWrapped("Window resize failed. Actual client area: %d x %d px.",clientWidth,clientHeight);else if(result==-2)ImGui::TextWrapped("The game did not reach the requested render resolution. Current: %d x %d px.",renderWidth,renderHeight);
 ImGui::Separator();ImGui::TextUnformatted("Saved resolutions");
 if(!resolutionHistory.count)ImGui::TextDisabled("Applied resolutions will be saved here automatically.");
 else{
  ImGui::TextDisabled("Select a saved resolution to apply it; most recent first.");
  if(ImGui::BeginListBox("##savedHotsamplingResolutions",ImVec2(-FLT_MIN,ImGui::GetTextLineHeightWithSpacing()*5))){
   for(unsigned i=0;i<resolutionHistory.count;++i){const auto& r=resolutionHistory.entries[i];char label[48]{};sprintf_s(label,"%u x %u",r.width,r.height);
    if(ImGui::Selectable(label,targetWidth==int(r.width)&&targetHeight==int(r.height))){targetWidth=int(r.width);targetHeight=int(r.height);queueHotsampleResize(targetWidth,targetHeight);}
   }
   ImGui::EndListBox();
  }
 }
 if(resolutionHistorySaveFailed)ImGui::TextWrapped("Could not save the resolution history beside VoidCamera.dll.");
 ImGui::TextDisabled("Default res at injection: %d x %d px",defaultWidth,defaultHeight);
 ImGui::Spacing();ImGui::TextWrapped("The camera keeps vertical FOV and recalculates horizontal FOV from the new aspect ratio.");
}
void cameraTab(){
 ImGui::TextUnformatted("CAMERA");ImGui::Separator();
 bool enabled=cameraEnabled;if(ImGui::Checkbox("Free camera (Insert)",&enabled))cameraToggle();helpMarker("Enables or disables the free camera. The view must be detected before it can be enabled.");
 ImGui::SetNextItemWidth(170);if(ImGui::Combo("Control device",&controlMode,"Keyboard\0Gamepad\0"))saveControlMode();helpMarker("Selects keyboard or XInput gamepad controls for free camera movement. The choice is saved and applies before the camera is enabled.");ImGui::SameLine();ImGui::BeginDisabled(!cameraEnabled);ImGui::Checkbox("Lock camera (Home)",&cameraLocked);ImGui::EndDisabled();helpMarker("Locks the camera pose so window resizing for hotsampling does not move the view. Press Home again to unlock.");
 if(controlMode==1)ImGui::TextDisabled(gamepadConnected?"XInput controller connected":"No XInput controller detected");
 if(cameraLocked)ImGui::TextDisabled("Camera pose locked. Resize the window for hotsampling, then press Home to unlock.");
 ImGui::Spacing();ImGui::TextUnformatted("TIME");ImGui::Separator();
 ImGui::BeginDisabled(!timeStopReady);bool stopped=timeStopped;if(ImGui::Checkbox("Time stop (Numpad 0)",&stopped))timeStopToggle();ImGui::EndDisabled();helpMarker("Pauses or resumes game simulation using the verified retail time control.");
 if(!timeStopReady){ImGui::SameLine();ImGui::TextDisabled("Retail CVar check failed");}
 ImGui::SameLine();ImGui::BeginDisabled(!hudReady);bool hidden=hudHidden;if(ImGui::Checkbox("Hide game UI + markers (Delete)",&hidden))hudToggle();ImGui::EndDisabled();helpMarker("Hides or restores the HUD and world markers. Some menu or mission elements may use separate render paths.");
 if(!hudReady){ImGui::SameLine();ImGui::TextDisabled("HUD control unavailable");}
 ImGui::BeginDisabled(!cameraEnabled);
 bool changed=false;float fine=ImGui::GetIO().KeyAlt?.01f:1.f;
 ImGui::SetNextItemWidth(400);changed|=ImGui::DragFloat3("Position XYZ",pose.xyz,.1f*fine,0,0,"%.3f");helpMarker("World-space camera position on the X, Y and Z axes. Hold Alt while dragging for fine adjustment.");
 ImGui::SetNextItemWidth(400);changed|=ImGui::DragFloat3("Pitch / Yaw / Roll",pose.pyr,.2f*fine,0,0,"%.3f");helpMarker("Camera orientation in degrees: pitch, yaw, then roll. Hold Alt while dragging for fine adjustment.");
 ImGui::SetNextItemWidth(400);ImGui::SliderInt("PYR rotation speed (deg/s)",&rotationSpeed,45,1800);helpMarker("Angular speed for keyboard and gamepad pitch, yaw and roll. This value is saved in settings.ini.");if(ImGui::IsItemDeactivatedAfterEdit())saveRotationSpeed();
 ImGui::SetNextItemWidth(400);changed|=ImGui::DragFloat("Vertical FOV",&pose.fov,.1f*fine,5,150,"%.3f");helpMarker("Vertical field of view in degrees. Horizontal FOV is calculated from the active aspect ratio, so framing stays proportionate at ultrawide and other aspect ratios.");
 ImGui::SetNextItemWidth(400);changed|=ImGui::DragFloat("Camera speed",&pose.speed,.01f*fine,.001f,500,"%.3f");helpMarker("Movement speed used by the selected keyboard or gamepad controls.");
 if(ImGui::Button("Reset roll")){pose.pyr[2]=0;changed=true;}helpMarker("Sets roll to zero without changing position, pitch or yaw.");if(changed)cameraPublish();
 ImGui::EndDisabled();ImGui::Spacing();ImGui::Separator();
}
void postProcessTab(){
 ImGui::TextUnformatted("POST-PROCESS");ImGui::Separator();
 if(ImGui::BeginTable("PostProcessColumns",2,ImGuiTableFlags_SizingStretchSame)){
  ImGui::TableNextColumn();renderToggle("Auto exposure",RenderControl::AutoExposure,"Lets the game adjust exposure to scene brightness.");renderToggle("Freeze exposure adaptation",RenderControl::FreezeExposure,"Freezes exposure adaptation at its current value while enabled.");renderToggle("Bloom",RenderControl::Bloom,"Enables or disables the glow around bright image areas.");renderToggle("Lens flares",RenderControl::LensFlares,"Enables or disables lens flare effects from bright lights.");renderToggle("Color grading",RenderControl::ColorGrading,"Enables or disables the scene color grading lookup.");renderToggle("Depth of field",RenderControl::DepthOfField,"Enables or disables focus blur based on distance.");renderToggle("Camera motion blur",RenderControl::MotionBlur,"Enables or disables blur caused by camera or object motion.");renderToggle("Tone mapping",RenderControl::ToneMapping,"Enables or disables the mapping from scene lighting to display brightness.");
  ImGui::TableNextColumn();renderFloat("Custom EV",RenderControl::CustomEV,.02f,-4,4,"%.2f EV","Additive exposure bias in EV stops. Positive values brighten the image; negative values darken it. This is separate from automatic exposure.");ImGui::SeparatorText("Depth of field");renderFloat("Focal distance (m)",RenderControl::DofFocalDistance,.1f,0,10000,"%.2f","Distance, in meters, at which the image is focused.");renderFloat("Focus range (m)",RenderControl::DofFocusRange,1,0,10000,"%.1f","Depth-of-field focus range in meters. Larger values keep more depth in focus.");renderFloat("Circle of confusion (mm)",RenderControl::DofCoc,.001f,0,.5f,"%.3f","Circle-of-confusion limit in millimeters; controls the strength and extent of depth-of-field blur.");
  ImGui::EndTable();
 }
 ImGui::Spacing();if(ImGui::Button("Restore initial render settings"))renderControlRestore();helpMarker("Restores render and environment CVar values captured when the tool was injected.");
}
void environmentTab(){
 ImGui::TextUnformatted("ENVIRONMENT");ImGui::Separator();
 ImGui::SeparatorText("Atmosphere effects");renderToggle("Fog",RenderControl::Fog,"Enables or disables scene fog. It changes atmospheric depth and visibility; it does not control object draw distance.");renderToggle("God rays",RenderControl::GodRays,"Enables or disables screen-space light shafts. This does not control fog density or shadow distance.");renderToggle("Fog light scattering",RenderControl::LightScattering,"Enables or disables light scattering inside fog. It can alter how lit or colored the fog appears.");renderToggle("Sky",RenderControl::Sky,"Enables or disables the sky rendering effect. Disabling it can expose the background or skybox behavior of the current scene.");
 ImGui::Spacing();if(ImGui::Button("Restore initial render settings"))renderControlRestore();helpMarker("Restores render and environment CVar values captured when the tool was injected.");
}
void lodTab(){
 ImGui::TextUnformatted("LEVEL OF DETAIL");ImGui::Separator();
 ImGui::SeparatorText("World detail");
 renderFloat("World LOD detail scale (lower = more detail)",RenderControl::StaticLodZoomMultiplier,.001f,.001f,100,"%.3f","Native CVar: r_lodZoomMultiplicator. Applies to static world geometry, not entity distances or camera FOV. Lower values favor finer static-mesh detail at the same view; 0.001 is available for testing but has not been visually tested. Extreme values may cost performance.");
 ImGui::SeparatorText("Shadows");
 ImGui::TextWrapped("Model shadow distance controls model-shadow culling. Cascade distance controls directional sun shadows separately.");
 renderFloat("Model shadow distance",RenderControl::ShadowDistance,.5f,0,100000,"%.1f","Retail CVar: r_shadowCullDistance. Maximum distance for model shadows; this is the model-shadow culling range. It does not change directional sun-shadow cascades.");
 ImGui::Spacing();if(ImGui::Button("Restore initial render settings"))renderControlRestore();helpMarker("Restores render and environment CVar values captured when the tool was injected.");
}
void panel(){std::lock_guard<std::recursive_mutex> g(cameraMutex);
 ImGui::SetNextWindowSize(ImVec2(620,530),ImGuiCond_FirstUseEver);
 ImGui::Begin("Dishonored 2 | Void Camera 1.47 | F10 to hide");
 if(ImGui::BeginTabBar("VoidCameraTabs")){
  if(ImGui::BeginTabItem("Camera")){ImGui::BeginChild("CameraTabBody",ImVec2(0,0),false,ImGuiWindowFlags_AlwaysVerticalScrollbar);cameraTab();ImGui::EndChild();ImGui::EndTabItem();}
  if(ImGui::BeginTabItem("Post-process")){ImGui::BeginChild("PostProcessTabBody",ImVec2(0,0),false,ImGuiWindowFlags_AlwaysVerticalScrollbar);postProcessTab();ImGui::EndChild();ImGui::EndTabItem();}
  if(ImGui::BeginTabItem("Environment")){ImGui::BeginChild("EnvironmentTabBody",ImVec2(0,0),false,ImGuiWindowFlags_AlwaysVerticalScrollbar);environmentTab();ImGui::EndChild();ImGui::EndTabItem();}
  if(ImGui::BeginTabItem("LOD")){ImGui::BeginChild("LodTabBody",ImVec2(0,0),false,ImGuiWindowFlags_AlwaysVerticalScrollbar);lodTab();ImGui::EndChild();ImGui::EndTabItem();}
  if(ImGui::BeginTabItem("Hotsampling")){ImGui::BeginChild("HotsamplingTabBody",ImVec2(0,0),false,ImGuiWindowFlags_AlwaysVerticalScrollbar);hotsamplingTab();ImGui::EndChild();ImGui::EndTabItem();}
  ImGui::EndTabBar();
 }
 ImGui::End();
}
void draw(IDXGISwapChain* chain){
 if(active&&active!=chain)return;
 DXGI_SWAP_CHAIN_DESC d{};if(FAILED(chain->GetDesc(&d)))return;window=d.OutputWindow;swapChainWindowed=d.Windowed!=FALSE;if(d.BufferDesc.Width&&d.BufferDesc.Height){cameraSetRenderResolution(d.BufferDesc.Width,d.BufferDesc.Height);renderWidth=static_cast<int>(d.BufferDesc.Width);renderHeight=static_cast<int>(d.BufferDesc.Height);if(!hotsampleInitialized){defaultWidth=targetWidth=renderWidth;defaultHeight=targetHeight=renderHeight;hotsampleInitialized=true;}}RECT cr{};if(GetClientRect(window,&cr)){clientWidth=cr.right-cr.left;clientHeight=cr.bottom-cr.top;}
 if(!device){if(FAILED(chain->GetDevice(__uuidof(ID3D11Device),(void**)&device)))return;active=chain;device->GetImmediateContext(&context);
  IMGUI_CHECKVERSION();ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.MouseDrawCursor=true;ImFontConfig font{};font.SizePixels=13;io.Fonts->AddFontDefault(&font);ImGui::StyleColorsDark();auto& s=ImGui::GetStyle();s.WindowPadding={8,8};s.FramePadding={4,3};s.ItemSpacing={8,4};s.WindowRounding=4;s.FrameRounding=2;s.Colors[ImGuiCol_WindowBg]={.055f,.065f,.07f,.97f};s.Colors[ImGuiCol_TitleBgActive]={.20f,.25f,.25f,1};s.Colors[ImGuiCol_CheckMark]={.75f,.65f,.43f,1};s.Colors[ImGuiCol_SliderGrab]={.65f,.56f,.36f,1};
  ImGui_ImplWin32_Init(window);ImGui_ImplDX11_Init(device,context);previous=(WNDPROC)SetWindowLongPtrW(window,GWLP_WNDPROC,(LONG_PTR)proc);if(originalClip)originalClip(nullptr);
 }
 if(!visible())return;
 ID3D11Texture2D* texture=nullptr;if(FAILED(chain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&texture)))return;
 ID3D11RenderTargetView* target=nullptr;auto hr=device->CreateRenderTargetView(texture,nullptr,&target);D3D11_TEXTURE2D_DESC td{};texture->GetDesc(&td);texture->Release();if(FAILED(hr))return;renderWidth=static_cast<int>(td.Width);renderHeight=static_cast<int>(td.Height);cameraSetRenderResolution(td.Width,td.Height);
 ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();auto& io=ImGui::GetIO();io.MouseDrawCursor=true;io.DisplayFramebufferScale={io.DisplaySize.x>0?td.Width/io.DisplaySize.x:1,io.DisplaySize.y>0?td.Height/io.DisplaySize.y:1};ImGui::NewFrame();panel();ImGui::Render();
 ID3D11RenderTargetView* old[8]{};ID3D11DepthStencilView* depth=nullptr;context->OMGetRenderTargets(8,old,&depth);context->OMSetRenderTargets(1,&target,nullptr);ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());context->OMSetRenderTargets(8,old,depth);for(auto p:old)if(p)p->Release();if(depth)depth->Release();target->Release();
}
HRESULT WINAPI present(IDXGISwapChain* s,UINT sync,UINT flags){if(!(flags&DXGI_PRESENT_TEST)){std::lock_guard<std::recursive_mutex> g(renderMutex);_configthreadlocale(_ENABLE_PER_THREAD_LOCALE);setlocale(LC_NUMERIC,"C");draw(s);}return originalPresent(s,sync,flags);}
HRESULT WINAPI resize(IDXGISwapChain* s,UINT count,UINT w,UINT h,DXGI_FORMAT fmt,UINT flags){std::lock_guard<std::recursive_mutex> g(renderMutex);return originalResize(s,count,w,h,fmt,flags);}
}
bool overlayInstall(){WNDCLASSW wc{};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"VoidCameraBootstrap";RegisterClassW(&wc);HWND temp=CreateWindowW(wc.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,32,32,nullptr,nullptr,wc.hInstance,nullptr);if(!temp)return false;
 DXGI_SWAP_CHAIN_DESC d{};d.BufferCount=1;d.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;d.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;d.OutputWindow=temp;d.SampleDesc.Count=1;d.Windowed=TRUE;d.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
 IDXGISwapChain* s=nullptr;ID3D11Device* dev=nullptr;ID3D11DeviceContext* ctx=nullptr;auto hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&d,&s,&dev,nullptr,&ctx);
 bool ok=false;if(SUCCEEDED(hr)){auto vt=*(void***)s;ok=MH_CreateHook(vt[8],(void*)present,(void**)&originalPresent)==MH_OK&&MH_CreateHook(vt[13],(void*)resize,(void**)&originalResize)==MH_OK;
 if(ok)ok=MH_EnableHook(vt[8])==MH_OK&&MH_EnableHook(vt[13])==MH_OK;
 ctx->Release();dev->Release();s->Release();}DestroyWindow(temp);UnregisterClassW(wc.lpszClassName,wc.hInstance);
 if(MH_CreateHookApi(L"user32.dll","SetCursorPos",(void*)warp,(void**)&originalWarp)==MH_OK)MH_EnableHook((void*)SetCursorPos);
 if(MH_CreateHookApi(L"user32.dll","ClipCursor",(void*)clip,(void**)&originalClip)==MH_OK)MH_EnableHook((void*)ClipCursor);
 return ok;
}
