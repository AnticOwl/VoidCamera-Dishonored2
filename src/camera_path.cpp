#include "camera_path.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {
struct Node {
    Pose pose{};
    float secondsToNext=2.0f;
};
std::vector<Node> nodes;
int selected=0;
bool playing=false,loop=false,smooth=true;
int segment=0;
float segmentElapsed=0.0f;
std::string statusText="Add at least two camera nodes to create a path.";
WCHAR pathFile[MAX_PATH]{};

void ensurePathFile(){
    if(pathFile[0])return;
    DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",pathFile,MAX_PATH);
    if(!n||n>=MAX_PATH-48){pathFile[0]=0;return;}
    wcscat_s(pathFile,L"\\VoidCamera");
    CreateDirectoryW(pathFile,nullptr);
    wcscat_s(pathFile,L"\\Dishonored2_CameraPath.vcp");
}
int indexFor(int i){
    const int count=static_cast<int>(nodes.size());
    if(count<=0)return 0;
    if(loop){i%=count;if(i<0)i+=count;return i;}
    return std::clamp(i,0,count-1);
}
float catmull(float p0,float p1,float p2,float p3,float t){
    const float t2=t*t,t3=t2*t;
    return .5f*((2*p1)+(-p0+p2)*t+(2*p0-5*p1+4*p2-p3)*t2+(-p0+3*p1-3*p2+p3)*t3);
}
float shortestDelta(float a,float b){return std::remainder(b-a,360.0f);}
float catmullAngle(float a0,float a1,float a2,float a3,float t){
    const float p0=a1+shortestDelta(a1,a0);
    const float p1=a1;
    const float p2=a1+shortestDelta(a1,a2);
    const float p3=p2+shortestDelta(a2,a3);
    return std::remainder(catmull(p0,p1,p2,p3,t),360.0f);
}
bool validPose(const Pose& p){
    for(float v:p.xyz)if(!std::isfinite(v)||std::fabs(v)>1.0e7f)return false;
    for(float v:p.pyr)if(!std::isfinite(v)||std::fabs(v)>1.0e7f)return false;
    return std::isfinite(p.fov)&&p.fov>=5.0f&&p.fov<=150.0f&&std::isfinite(p.speed)&&p.speed>=.001f&&p.speed<=500.0f;
}
Pose interpolate(int seg,float t){
    const Node& n0=nodes[indexFor(seg-1)];
    const Node& n1=nodes[indexFor(seg)];
    const Node& n2=nodes[indexFor(seg+1)];
    const Node& n3=nodes[indexFor(seg+2)];
    Pose out=n1.pose;
    for(int axis=0;axis<3;++axis){
        out.xyz[axis]=smooth?catmull(n0.pose.xyz[axis],n1.pose.xyz[axis],n2.pose.xyz[axis],n3.pose.xyz[axis],t):n1.pose.xyz[axis]+(n2.pose.xyz[axis]-n1.pose.xyz[axis])*t;
        out.pyr[axis]=smooth?catmullAngle(n0.pose.pyr[axis],n1.pose.pyr[axis],n2.pose.pyr[axis],n3.pose.pyr[axis],t):n1.pose.pyr[axis]+shortestDelta(n1.pose.pyr[axis],n2.pose.pyr[axis])*t;
    }
    out.fov=smooth?catmull(n0.pose.fov,n1.pose.fov,n2.pose.fov,n3.pose.fov,t):n1.pose.fov+(n2.pose.fov-n1.pose.fov)*t;
    out.fov=std::clamp(out.fov,5.0f,150.0f);
    out.speed=n1.pose.speed+(n2.pose.speed-n1.pose.speed)*t;
    return out;
}
}

bool cameraPathIsPlaying(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return playing;}
bool cameraPathUpdate(float deltaSeconds){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(!playing)return false;
    if(!cameraEnabled||nodes.size()<2){playing=false;statusText="Playback stopped: free camera is off.";return false;}
    const int lastSegment=loop?static_cast<int>(nodes.size())-1:static_cast<int>(nodes.size())-2;
    if(segment>lastSegment){
        if(loop)segment=0;
        else{pose=nodes.back().pose;cameraPublish();playing=false;statusText="Path finished.";return true;}
    }
    segmentElapsed+=std::clamp(deltaSeconds,0.0f,.05f);
    const float duration=std::clamp(nodes[indexFor(segment)].secondsToNext,.1f,3600.0f);
    const float t=std::clamp(segmentElapsed/duration,0.0f,1.0f);
    pose=interpolate(segment,t);
    cameraPublish();
    if(t>=1.0f){
        ++segment;segmentElapsed=0.0f;
        if(!loop&&segment>lastSegment){pose=nodes.back().pose;playing=false;statusText="Path finished.";}
    }
    return true;
}
unsigned cameraPathNodeCount(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return static_cast<unsigned>(nodes.size());}
int cameraPathSelectedNode(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return selected;}
bool cameraPathGetNode(unsigned i,Pose& p,float& duration){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(i>=nodes.size())return false;p=nodes[i].pose;duration=nodes[i].secondsToNext;return true;}
void cameraPathSelectNode(int i){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(nodes.empty())selected=0;else selected=std::clamp(i,0,static_cast<int>(nodes.size())-1);}
void cameraPathSetDuration(float seconds){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(nodes.empty()||!std::isfinite(seconds))return;nodes[selected].secondsToNext=std::clamp(seconds,.1f,3600.0f);}
void cameraPathSetLoop(bool v){std::lock_guard<std::recursive_mutex> g(cameraMutex);loop=v;}
bool cameraPathLoop(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return loop;}
void cameraPathSetSmooth(bool v){std::lock_guard<std::recursive_mutex> g(cameraMutex);smooth=v;}
bool cameraPathSmooth(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return smooth;}
std::string cameraPathStatus(){std::lock_guard<std::recursive_mutex> g(cameraMutex);return statusText;}
void cameraPathAddNode(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(!cameraEnabled){statusText="Enable the free camera before adding a node.";return;}
    if(nodes.size()>=256){statusText="Path limit reached (256 nodes).";return;}
    Node node{};node.pose=pose;if(!nodes.empty())node.secondsToNext=nodes.back().secondsToNext;
    nodes.push_back(node);selected=static_cast<int>(nodes.size())-1;statusText="Camera node added.";
}
void cameraPathInsertAfterSelected(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(!cameraEnabled){statusText="Enable the free camera before inserting a node.";return;}
    if(nodes.size()>=256){statusText="Path limit reached (256 nodes).";return;}
    if(nodes.empty()){Node node{};node.pose=pose;nodes.push_back(node);selected=0;statusText="First camera node added.";return;}
    Node inserted{};inserted.pose=pose;
    const float oldDuration=nodes[selected].secondsToNext;
    if(loop||selected+1<static_cast<int>(nodes.size())){
        const float firstHalf=std::max(.1f,oldDuration*.5f);
        nodes[selected].secondsToNext=firstHalf;
        inserted.secondsToNext=std::max(.1f,oldDuration-firstHalf);
    }else inserted.secondsToNext=oldDuration;
    const auto position=nodes.begin()+selected+1;
    selected=static_cast<int>(std::distance(nodes.begin(),nodes.insert(position,inserted)));
    playing=false;statusText="Current pose inserted after the selected node.";
}
void cameraPathUpdateNode(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(nodes.empty()||!cameraEnabled){statusText="Enable the free camera and select a node first.";return;}
    nodes[selected].pose=pose;statusText="Selected node updated from current camera.";
}
void cameraPathGoToSelected(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(nodes.empty()||!cameraEnabled){statusText="Enable the free camera and select a node first.";return;}
    playing=false;pose=nodes[selected].pose;cameraPublish();statusText="Moved to selected node.";
}
void cameraPathGoToStart(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(nodes.empty()||!cameraEnabled){statusText="Enable the free camera and add a node first.";return;}
    playing=false;selected=0;pose=nodes.front().pose;cameraPublish();statusText="Moved to first node.";
}
void cameraPathDeleteSelected(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(nodes.empty())return;playing=false;nodes.erase(nodes.begin()+selected);selected=nodes.empty()?0:std::clamp(selected,0,static_cast<int>(nodes.size())-1);statusText="Selected node deleted.";
}
void cameraPathMoveSelected(int direction){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(nodes.empty()||direction==0)return;
    const int destination=selected+(direction<0?-1:1);
    if(destination<0||destination>=static_cast<int>(nodes.size())){statusText="Selected node is already at that end of the path.";return;}
    playing=false;std::swap(nodes[selected],nodes[destination]);selected=destination;statusText=direction<0?"Selected node moved earlier in the path.":"Selected node moved later in the path.";
}
void cameraPathClear(){std::lock_guard<std::recursive_mutex> g(cameraMutex);playing=false;nodes.clear();selected=0;statusText="Camera path cleared.";}
void cameraPathPlay(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);
    if(playing)return;
    if(!cameraEnabled){statusText="Enable the free camera before playback.";return;}
    if(nodes.size()<2){statusText="Add at least two nodes before playback.";return;}
    segment=0;segmentElapsed=0;pose=nodes.front().pose;cameraPublish();playing=true;statusText="Playing camera path.";
}
void cameraPathStop(){std::lock_guard<std::recursive_mutex> g(cameraMutex);if(playing){playing=false;statusText="Playback stopped at the current pose.";}}
void cameraPathTogglePlayback(){if(cameraPathIsPlaying())cameraPathStop();else cameraPathPlay();}
void cameraPathSave(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);ensurePathFile();
    if(!pathFile[0]){statusText="Save failed: LOCALAPPDATA unavailable.";return;}
    FILE* f=nullptr;if(_wfopen_s(&f,pathFile,L"w")!=0||!f){statusText="Could not open camera path file for writing.";return;}
    bool ok=std::fprintf(f,"VOID_CAMERA_PATH_V1\n%d %d %u\n",loop?1:0,smooth?1:0,static_cast<unsigned>(nodes.size()))>=0;
    for(const auto& n:nodes)if(ok)ok=std::fprintf(f,"%.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",n.pose.xyz[0],n.pose.xyz[1],n.pose.xyz[2],n.pose.pyr[0],n.pose.pyr[1],n.pose.pyr[2],n.pose.fov,n.pose.speed,n.secondsToNext)>=0;
    if(std::fclose(f)!=0)ok=false;
    statusText=ok?"Camera path saved to %LOCALAPPDATA%\\VoidCamera\\Dishonored2_CameraPath.vcp":"Camera path save failed.";
}
void cameraPathLoad(){
    std::lock_guard<std::recursive_mutex> g(cameraMutex);ensurePathFile();
    if(!pathFile[0]){statusText="Load failed: LOCALAPPDATA unavailable.";return;}
    FILE* f=nullptr;if(_wfopen_s(&f,pathFile,L"r")!=0||!f){statusText="Camera path file not found.";return;}
    char magic[64]{};int loadedLoop=0,loadedSmooth=1;unsigned count=0;
    bool ok=fscanf_s(f,"%63s",magic,static_cast<unsigned>(_countof(magic)))==1&&std::strcmp(magic,"VOID_CAMERA_PATH_V1")==0&&fscanf_s(f,"%d %d %u",&loadedLoop,&loadedSmooth,&count)==3&&count<=256;
    std::vector<Node> loaded;loaded.reserve(ok?count:0);
    for(unsigned i=0;ok&&i<count;++i){Node n{};ok=fscanf_s(f,"%f %f %f %f %f %f %f %f %f",&n.pose.xyz[0],&n.pose.xyz[1],&n.pose.xyz[2],&n.pose.pyr[0],&n.pose.pyr[1],&n.pose.pyr[2],&n.pose.fov,&n.pose.speed,&n.secondsToNext)==9&&validPose(n.pose)&&std::isfinite(n.secondsToNext)&&n.secondsToNext>=.1f&&n.secondsToNext<=3600.0f;if(ok)loaded.push_back(n);}
    std::fclose(f);
    if(!ok){statusText="Invalid camera path file; current path kept.";return;}
    playing=false;nodes=std::move(loaded);selected=0;loop=loadedLoop!=0;smooth=loadedSmooth!=0;statusText="Camera path loaded.";
}
