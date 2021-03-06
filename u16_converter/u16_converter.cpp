/*
Copyright(c) 2018 glcolor

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "stdafx.h"
#include "Parser.h"
#include "cmdline.h"
#include <functional>
#include <algorithm>

using namespace std;
using namespace jx;

//获取文件的扩展名
std::string GetExtName(const string& fileName)
{
	char ext[_MAX_EXT];
	_splitpath_s(fileName.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);
	string s(ext);
	transform(s.begin(), s.end(), s.begin(), tolower);
	return s;
}

//遍历目录并执行操作
void EnumFiles(const string& path,bool processNoExtFiles, function<void(const string& fileName)> op)
{
	if (op == nullptr) return;
	WIN32_FIND_DATAA FindFileData;

	string curPath = path + "\\*.*";

	HANDLE hFind = ::FindFirstFileA(curPath.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)	return;

	while (true)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				string subPath = path + "\\";
				subPath+=(char*)(FindFileData.cFileName);
				EnumFiles(subPath, processNoExtFiles, op);
			}
		}
		else
		{
			//根据文件扩展名判断文件是否是C++源代码文件
			string ext=GetExtName(FindFileData.cFileName);
			if (ext==".h" || ext==".hpp" || ext == ".hxx" || ext==".c" || ext==".cpp" || ext == ".cxx" || ext == ".cc" || (processNoExtFiles && ext.empty()) )
			{
				string fileName = path + "\\";
				fileName += (char*)FindFileData.cFileName;
				op(fileName);
			}
		}
		if (!FindNextFileA(hFind, &FindFileData))	break;
	}
	FindClose(hFind);
}

int main(int argc,const char* argv[])
{
	cmdline::parser cp;
	cp.add<string>("prefix", 'p', "prefix of utf-16 strings", false, "_u");
	cp.add("dir", 'd', "is directory");
	cp.add("noext", 'n', "treat files without ext name as c++ code file");
	cp.add<string>("filename", 'f', "file or directory to convert", true, "");
	if (!cp.parse_check(argc, argv))
	{
		system("pause");
		return 0;
	}

	string unicodeStringPrefix = cp.get<string>("prefix") + "(";
	string fileName = cp.get<string>("filename");
	bool isDir = cp.exist("dir");
	bool noext = cp.exist("noext");

	if (isDir)
	{
		if (fileName.back() == '\\') fileName = fileName.substr(0, fileName.length() - 1);
		EnumFiles(fileName, noext,[unicodeStringPrefix](const string& fileName)
		{
			printf("%s...\t",fileName.c_str());
			Parser p;
			p.Parse(fileName, unicodeStringPrefix);
			printf("completed.\n");
		});
	}
	else
	{
		printf("%s...\t", fileName.c_str());
		Parser p;
		p.Parse(fileName, unicodeStringPrefix);
		printf("completed.\n");
	}	
	system("pause");
    return 0;
}

