#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>

#include <vector>
#include <string>
using namespace std;
#include <windowsx.h>
#endif

#include "MeshImport.h"

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

namespace NVSHARE
{

#ifdef WIN32

static void *getMeshBindingInterface(const char *dll,NxI32 version_number) // loads the tetra maker DLL and returns the interface pointer.
{
  void *ret = 0;

  UINT errorMode = 0;
  errorMode = SEM_FAILCRITICALERRORS;
  UINT oldErrorMode = SetErrorMode(errorMode);
  HMODULE module = LoadLibraryA(dll);
  SetErrorMode(oldErrorMode);
  if ( module )
  {
    void *proc = GetProcAddress(module,"getInterface");
    if ( proc )
    {
      typedef void * (__cdecl * NX_GetToolkit)(NxI32 version);
      ret = ((NX_GetToolkit)proc)(version_number);
    }
  }
  return ret;
}



#endif

}; // end of namespace

#ifdef LINUX_GENERIC
#include <sys/types.h>
#include <sys/dir.h>
#endif

#define MAXNAME 512

#define MESHIMPORT_NVSHARE MESHIMPORT_##NVSHARE

namespace MESHIMPORT_NVSHARE
{

class FileFind
{
public:
  FileFind::FileFind(const char *dirname,const char *spec)
  {
    if ( dirname && strlen(dirname) )
      sprintf(mSearchName,"%s\\%s",dirname,spec);
    else
      sprintf(mSearchName,"%s",spec);
   }

  FileFind::~FileFind(void)
  {
  }


  bool FindFirst(char *name)
  {
    bool ret=false;

    #ifdef WIN32
    hFindNext = FindFirstFileA(mSearchName, &finddata);
    if ( hFindNext == INVALID_HANDLE_VALUE )
      ret =  false;
     else
     {
       bFound = 1; // have an initial file to check.
       ret =  FindNext(name);
     }
     #endif
     #ifdef LINUX_GENERIC
     mDir = opendir(".");
     ret = FindNext(name);
    #endif
    return ret;
  }

  bool FindNext(char *name)
  {
    bool ret = false;

    #ifdef WIN32
    while ( bFound )
    {
      bFound = FindNextFileA(hFindNext, &finddata);
      if ( bFound && (finddata.cFileName[0] != '.') && !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
      {
        strncpy(name,finddata.cFileName,MAXNAME);
        ret = true;
        break;
      }
    }
    if ( !ret )
    {
      bFound = 0;
      FindClose(hFindNext);
    }
    #endif

    #ifdef LINUX_GENERIC

    if ( mDir )
    {
      while ( 1 )
      {

        struct direct *di = readdir( mDir );

        if ( !di )
        {
          closedir( mDir );
          mDir = 0;
          ret = false;
          break;
        }

        if ( strcmp(di->d_name,".") == 0 || strcmp(di->d_name,"..") == 0 )
        {
          // skip it!
        }
        else
        {
          strncpy(name,di->d_name,MAXNAME);
          ret = true;
          break;
        }
      }
    }
    #endif
    return ret;
  }

private:
  char mSearchName[MAXNAME];
#ifdef WIN32
  WIN32_FIND_DATAA finddata;
  HANDLE hFindNext;
  NxI32 bFound;
#endif
#ifdef LINUX_GENERIC
  DIR      *mDir;
#endif
};

}; // end of namespace

namespace NVSHARE
{

	using namespace MESHIMPORT_NVSHARE;
 

void GetFileList (vector<string>& list, string dir, string extension, string filenameToIgnore){ 
	WIN32_FIND_DATA findData; 
	HANDLE fileHandle; 
	int flag = 1; 
		
	string temp = dir;
	temp.append("/");
	temp.append(extension); 
	 
	wstring ws;
	ws.assign(temp.begin(), temp.end());
	fileHandle = FindFirstFile(ws.c_str(), &findData);  
	if (fileHandle == INVALID_HANDLE_VALUE) 
		return; 
	while (flag) {  
		string name;
		wstring found(findData.cFileName);
		name.assign(found.begin(), found.end());
		if(filenameToIgnore.compare(name) != 0)
			list.push_back(name);  
		flag = FindNextFile(fileHandle, &findData); 
	} 
	FindClose(fileHandle);
}

NVSHARE::MeshImport * loadMeshImporters(const char * directory) // loads the mesh import library (dll) and all available importers from the same directory.
{
  NVSHARE::MeshImport *ret = 0;
#ifdef _M_IX86
  const char *baseImporter = "MeshImport_x86.dll";
#else
  const char * baseImporter = "MeshImport_x64.dll";
#endif
  char scratch[512];
  if ( directory && strlen(directory) )
  {
    sprintf(scratch,"%s\\%s", directory, baseImporter);
  }
  else
  {
    strcpy(scratch,baseImporter);
  }

#ifdef WIN32
  ret = (NVSHARE::MeshImport *)getMeshBindingInterface(scratch,MESHIMPORT_VERSION);
#else
  ret = 0;
#endif

  if ( ret )
  {
	  vector<string> plugins;
	  #ifdef _M_IX86
	  GetFileList(plugins, directory, "MeshImport*_x86.dll", baseImporter);
	  #else
	  GetFileList(plugins, directory, "MeshImport*_x64.dll", baseImporter);
	  #endif

	  for(size_t i=0;i<plugins.size();i++) {
		  #ifdef WIN32
		  NVSHARE::MeshImporter *imp = (NVSHARE::MeshImporter *)getMeshBindingInterface(plugins[i].c_str(),MESHIMPORT_VERSION);
		  #else
		  NVSHARE::MeshImporter *imp = 0;
		  #endif
		  if ( imp )
		  {
			 ret->addImporter(imp);
			 printf("Added importer '%s'\r\n", plugins[i].c_str() );
		  }
	  } 
  }
  return ret;  
}
}; // end of namespace