#pragma once
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cwchar>

struct SavedResolution { unsigned width=0,height=0; };

class ResolutionHistory {
public:
 static constexpr unsigned Limit=16;
 std::array<SavedResolution,Limit> entries{};
 unsigned count=0;
 static bool valid(unsigned w,unsigned h){return w>=320&&h>=240&&w<=16384&&h<=16384;}
 void remember(unsigned w,unsigned h){
  if(!valid(w,h))return;
  unsigned index=0;for(;index<count;++index)if(entries[index].width==w&&entries[index].height==h)break;
  if(index==count){if(count<Limit)++count;index=count-1;}
  for(unsigned i=index;i>0;--i)entries[i]=entries[i-1];
  entries[0]={w,h};
 }
 void load(const wchar_t* path){
  count=0;entries={};
  unsigned n=std::min(unsigned(GetPrivateProfileIntW(L"Resolutions",L"Count",0,path)),Limit);
  std::array<SavedResolution,Limit> loaded{};
  for(unsigned i=0;i<n;++i){
   wchar_t key[32]{},value[64]{};swprintf_s(key,L"Resolution%u",i);
   GetPrivateProfileStringW(L"Resolutions",key,L"",value,64,path);
   unsigned w=0,h=0;if(swscanf_s(value,L"%u x %u",&w,&h)==2&&valid(w,h))loaded[i]={w,h};
  }
  for(unsigned i=n;i>0;--i)remember(loaded[i-1].width,loaded[i-1].height);
 }
 bool save(const wchar_t* path)const{
  wchar_t value[64]{};swprintf_s(value,L"%u",count);
  bool ok=WritePrivateProfileStringW(L"Resolutions",L"Count",value,path)!=FALSE;
  for(unsigned i=0;i<count;++i){
   wchar_t key[32]{};swprintf_s(key,L"Resolution%u",i);
   swprintf_s(value,L"%u x %u",entries[i].width,entries[i].height);
   ok=WritePrivateProfileStringW(L"Resolutions",key,value,path)!=FALSE&&ok;
  }
  return ok;
 }
};
