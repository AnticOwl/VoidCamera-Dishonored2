#pragma once
#include "camera.h"
#include <string>

bool cameraPathIsPlaying();
bool cameraPathUpdate(float deltaSeconds);
unsigned cameraPathNodeCount();
int cameraPathSelectedNode();
bool cameraPathGetNode(unsigned index, Pose& nodePose, float& secondsToNext);
void cameraPathSelectNode(int index);
void cameraPathSetDuration(float seconds);
void cameraPathSetLoop(bool enabled);
bool cameraPathLoop();
void cameraPathSetSmooth(bool enabled);
bool cameraPathSmooth();
std::string cameraPathStatus();
void cameraPathAddNode();
void cameraPathInsertAfterSelected();
void cameraPathUpdateNode();
void cameraPathGoToSelected();
void cameraPathGoToStart();
void cameraPathDeleteSelected();
void cameraPathMoveSelected(int direction);
void cameraPathClear();
void cameraPathTogglePlayback();
void cameraPathPlay();
void cameraPathStop();
void cameraPathSave();
void cameraPathLoad();
