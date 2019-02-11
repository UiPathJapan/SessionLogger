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
#include "ErrorMessage.h"


using namespace UiPathTeam;


ErrorMessage::ErrorMessage(DWORD dwError)
{
    WCHAR sz[320];
    _snwprintf_s(sz, _TRUNCATE, L"%lu (", dwError);
    PWCHAR pCur = sz + wcslen(sz);
    PWCHAR pEnd = sz + _countof(sz);
    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, 0, pCur, static_cast<DWORD>(pEnd - pCur), NULL))
    {
        pCur += wcslen(pCur);
        while (pCur[-1] == L'\n')
        {
            pCur--;
            if (pCur[-1] == L'\r')
            {
                pCur--;
            }
        }
        _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L")");
    }
    else
    {
        _snwprintf_s(sz, _TRUNCATE, L"%lu", dwError);
    }
    assign(sz);
}


ErrorMessage::~ErrorMessage()
{
}


ErrorMessage::ErrorMessage(const ErrorMessage& src)
    : std::wstring(src)
{
}


void ErrorMessage::operator =(const ErrorMessage& src)
{
    assign(src);
}
