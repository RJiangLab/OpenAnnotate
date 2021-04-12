#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "eutil.h"


int MakeDir(const char * dir)
{
#if defined(WINDOWS)
	_mkdir(dir);
#elif defined(LINUX)
	mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#elif defined(MACOS)
	mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	return PathExists(dir) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int MakePath(const char * pathin)
{
	int result = EXIT_SUCCESS;

	string path = pathin;
	string base = path.substr(0, path.find_last_of(PATH_SEPARATOR_NATIVE, string::npos));
	if(!PathExists(base.c_str())){
		result = MakePath(base.c_str());
	}
	if(result == EXIT_SUCCESS){
		result = MakeDir(base.c_str()) ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	return result;
}

int PathExists(const char * path)
{
	DIR * dir = NULL;
	if((dir = opendir(path)) != NULL){
		closedir(dir);
	}
	return dir != NULL;
}

char * MakeFileName(char * name, int seperator)
{
	if(name) for(char * p = name; *p; p++){
		*p = strchr("\\/", *p) ? seperator : *p;
	}
	return name;
}

string MakeFileName(const string& namein, int seperator)
{
	string nameout = namein;
	for(int i = 0, I = nameout.length(); i < I; i++){
		nameout[i] = strchr("\\/", nameout[i]) ? seperator : nameout[i];
	}
	return nameout;
}

string GetDirName(const string & path)
{
	int last = path.find_last_of("/");
	return (last == string::npos) ? path : path.substr(0,last);
}

string GetBaseName(const string & path)
{
	int last = path.find_last_of("/");
	return (last == string::npos) ? path : path.substr(last+1);
}

string GetExecPath()
{
	char exec[1024] = {0};

#if defined(WINDOWS)
	GetModuleFileName(NULL, (LPTSTR)exec, sizeof(exec));
#elif defined(LINUX)
	readlink("/proc/self/exe", exec, sizeof(exec));
#elif defined(MACOS)
	readlink("/proc/self/exe", exec, sizeof(exec));
#endif
//	MakeFileName(exec, PATH_SEPARATOR_INTERNAL);
//	char * last = strrchr(exec, PATH_SEPARATOR_INTERNAL);
//	if(last) *last = 0;

	return string(exec);
}

string GetDataPath()
{
	string pexec = GetExecPath();
	string phome = GetDirName(GetDirName(pexec));
	string pdata = phome + "/data";

	return pdata;
}


void Sleep(unsigned const int millisecond) 
{
	timeval timeout;
	timeout.tv_sec = millisecond / 1000;
	timeout.tv_usec =millisecond % 1000*1000;
	::select(0, NULL, NULL, NULL, &timeout);
	return;
}
