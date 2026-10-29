#pragma once
#include <Windows.h>
#include <cstdint>
#include <mutex>
struct Pose { float xyz[3]{},pyr[3]{},fov=60,speed=2; };
struct Status { unsigned magic=0x56433031; long ready=0,enabled=0,error=0,overlay=1; unsigned captures=0,applied=0,cached=0; unsigned long long view=0; Pose pose; LONG command=0,ack=0; float args[8]{}; long timeStopReady=0,timeStopped=0,locked=0,controlMode=0,hudReady=0,hudHidden=0,focusLossReady=0,focusLossEnabled=0; float aspect=1,horizontalFov=60; LONG igcsConnectorReady=0; };
enum class RenderControl : unsigned { AutoExposure, FreezeExposure, Bloom, LensFlares, ColorGrading, DepthOfField, MotionBlur, ToneMapping, CustomEV, DofFocalDistance, DofFocusRange, DofCoc, Fog, GodRays, LightScattering, Sky, AreaDiffuse, AreaSpecular, AreaShadowMin, AreaShadowMax, DebugAreaLights, DebugLightVolumes, ShadowDistance, ViewDistancePreset, FarZClipDistance, ShadowQualityPreset, ModelsQualityPreset, StaticLodZoomMultiplier, LodForceVisibleDistance, LodDistance0, LodDistance1, LodDistance2, LodDistance3, LodDistanceFar, TextureDetailsPreset, TextureLodBias, ForceFirstMip, AddAllMips, TextureStreamerBudget, ShadowPartialCascadeUpdate, ShadowParallelSliceSize, Count };
extern std::recursive_mutex cameraMutex;
extern Pose pose;
extern bool cameraReady,cameraEnabled;
extern bool timeStopReady,timeStopped;
extern bool hudReady,hudHidden;
extern bool cutsceneSubtitleReady;
extern bool focusLossReady,focusLossEnabled;
extern bool cameraLocked;
extern bool gamepadConnected;
extern int controlMode;
extern int rotationSpeed;
extern Status* status;
void cameraToggle();
void timeStopToggle();
void hudToggle();
void cameraApplyCutsceneSubtitleLevel(int level);
void queueCutsceneSubtitleLevel(int level);
void focusLossToggle();
void saveControlMode();
void saveRotationSpeed();
void cameraPublish();
void cameraFrame();
void cameraSetRenderResolution(unsigned width,unsigned height);
bool lodTestForceEntityLod0();
bool lodTestEntityLod0Forced();
bool lodDiagnosticLogToggle();
bool lodDiagnosticLogActive();
void lodDiagnosticLogMark(bool visible);
bool renderControlReady(RenderControl id);
bool renderControlEnabled(RenderControl id);
float renderControlValue(RenderControl id);
void renderControlSetEnabled(RenderControl id,bool value);
void renderControlSetValue(RenderControl id,float value);
bool nativeViewDistanceSetterReady();
void renderControlRestore();
bool shadowCascadeReady();
int shadowCascadeFarDistance();
void shadowCascadeSetFarDistance(int distance);
bool overlayInstall();
