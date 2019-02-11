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
#include <ctype.h>
#include "Path.h"


using namespace UiPathTeam;


std::wstring Path::GetDirectory(PCWSTR pszPath)
{
    const WCHAR* pRoot = NULL;
    const WCHAR* pLast = NULL;
    const WCHAR* pCur = pszPath;

    if (iswalpha(pCur[0]) && pCur[1] == L':')
    {
        // Drive designator
        pCur += 2;
        if (pCur[0] == L'\\')
        {
            pRoot = pCur++;
        }
    }
    else if (pCur[0] == L'\\')
    {
        if (pCur[1] == L'\\')
        {
            bool bUNC;
            if (pCur[2] == L'?' && pCur[3] == L'\\')
            {
                // Extended-Length style
                pCur += 4;
                if (pCur[0] == L'U' && pCur[1] == L'N' && pCur[2] == L'C' && pCur[3] == L'\\')
                {
                    // Extended-Length UNC style
                    pCur += 4;
                    bUNC = true;
                }
                else
                {
                    bUNC = false;
                }
            }
            else
            {
                pCur += 2;
                bUNC = true;
            }
            if (bUNC)
            {
                // UNC style (\\server\share\*)
                // check server part
                if (pCur[0] != L'\\' && pCur[0] != L'\0')
                {
                    pCur++;
                }
                else
                {
                    // Malformed
                    return std::wstring();
                }
                while (pCur[0] != L'\\')
                {
                    if (pCur[0] != L'\0')
                    {
                        pCur++;
                    }
                    else
                    {
                        // Malformed
                        return std::wstring();
                    }
                }
                pCur++;
                // check share part
                if (pCur[0] != L'\\' && pCur[0] != L'\0')
                {
                    pCur++;
                }
                else
                {
                    // Malformed
                    return std::wstring();
                }
                while (pCur[0] != L'\\')
                {
                    if (pCur[0] != L'\0')
                    {
                        pCur++;
                    }
                    else
                    {
                        // Malformed
                        return std::wstring();
                    }
                }
                pRoot = pCur++;
            }
            else if (iswalpha(pCur[0]) && pCur[1] == L':')
            {
                // Drive designator
                pCur += 2;
                if (pCur[0] == L'\\')
                {
                    pRoot = pCur++;
                }
            }
            else if (pCur[0] == L'\\')
            {
                pRoot = pCur++;
            }
        }
        else
        {
            pRoot = pCur++;
        }
    }

    while (pCur[0] != L'\0')
    {
        if (pCur[0] == L'\\')
        {
            if (pCur[1] != L'\0')
            {
                pLast = pCur++;
            }
            else
            {
                break;
            }
        }
        else
        {
            pCur++;
        }
    }

    if (pLast != NULL)
    {
        return std::wstring(pszPath, pLast - pszPath);
    }
    else if (pRoot != NULL)
    {
        return std::wstring(pszPath, (pRoot + 1) - pszPath);
    }
    else
    {
        return std::wstring(pszPath).append(L".");
    }
}


std::wstring Path::GetDirectory(const std::wstring& sPath)
{
    return GetDirectory(sPath.c_str());
}
