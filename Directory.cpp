/*

Copyright (C) 2018-2019 UiPath, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/


#include "stdafx.h"
#include "Path.h"
#include "Directory.h"


using namespace UiPathTeam;


bool Directory::Exists(PCWSTR pszPath)
{
    DWORD dwRet = GetFileAttributesW(pszPath);
    return dwRet != INVALID_FILE_ATTRIBUTES && (dwRet & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}


bool Directory::Exists(const std::wstring& sPath)
{
    return Exists(sPath.c_str());
}


bool Directory::Create(PCWSTR pszPath, PSECURITY_ATTRIBUTES pSecurityAttributes)
{
    std::wstring sParent = Path::GetDirectory(pszPath);
    if (sParent.length() > 0 && sParent != pszPath && !Exists(sParent))
    {
        if (!Create(sParent, pSecurityAttributes))
        {
            return false;
        }
    }
    return CreateDirectoryW(pszPath, pSecurityAttributes) ? true : false;
}


bool Directory::Create(const std::wstring& sPath, PSECURITY_ATTRIBUTES pSecurityAttributes)
{
    return Create(sPath.c_str(), pSecurityAttributes);
}
