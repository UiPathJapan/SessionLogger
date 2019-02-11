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
#include "LogFile.h"


using namespace UiPathTeam;


#define INITIAL_SIZE 8192


#define DETACH(x) reinterpret_cast<HANDLE>(InterlockedExchangePointer(reinterpret_cast<void**>(&x),INVALID_HANDLE_VALUE))


LogFile::LogFile(PCWSTR pszFileName)
    : m_pszFileName(NULL)
    , m_dwError(0)
    , m_hFile(INVALID_HANDLE_VALUE)
    , m_wcBuf(NULL)
    , m_wcSize(INITIAL_SIZE)
    , m_mutex(0)
{
    m_wcBuf = (PWCHAR)malloc(m_wcSize * sizeof(WCHAR));
    if (!m_wcBuf)
    {
        throw std::bad_alloc();
    }
    if (pszFileName)
    {
        m_pszFileName = _wcsdup(pszFileName);
        if (!m_pszFileName)
        {
            throw std::bad_alloc();
        }
    }
}


LogFile::~LogFile()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(DETACH(m_hFile));
    }
    free(m_pszFileName);
    free(m_wcBuf);
}


void LogFile::SetFileName(PCWSTR pszFileName)
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(DETACH(m_hFile));
    }
    free(m_pszFileName);
    if (pszFileName)
    {
        m_pszFileName = _wcsdup(pszFileName);
        if (!m_pszFileName)
        {
            throw std::bad_alloc();
        }
    }
    else
    {
        m_pszFileName = NULL;
    }
    m_dwError = 0;
}


bool LogFile::Close()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return true;
    }
    else if (CloseHandle(DETACH(m_hFile)))
    {
        return true;
    }
    else
    {
        m_dwError = GetLastError();
        return false;
    }
}


bool LogFile::Open()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(DETACH(m_hFile));
    }
    SetLastError(0);
    m_hFile = CreateFileW(m_pszFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            LARGE_INTEGER offset = { 0, 0 };
            if (!SetFilePointerEx(m_hFile, offset, NULL, FILE_END))
            {
                m_dwError = GetLastError();
                return false;
            }
        }
        else
        {
            const BYTE ByteOrderMark[3] = { 0xEF, 0xBB, 0xBF };
            DWORD dwCount = 0;
            if (!WriteFile(m_hFile, ByteOrderMark, _countof(ByteOrderMark), &dwCount, NULL))
            {
                m_dwError = GetLastError();
                return false;
            }
        }
        return true;
    }
    else
    {
        m_dwError = GetLastError();
        return false;
    }
}


bool LogFile::Put(PCWSTR pszFormat, ...)
{
    Mutex lock(this);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        m_dwError = ERROR_NOT_READY;
        return false;
    }
    bool bRet = true;
    SYSTEMTIME t;
    GetLocalTime(&t);
    static const WCHAR szTimeFormat[] = { L"%4d-%02d-%02dT%02d:%02d:%02d.%03d " };
    static const int wcLen1 = 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 3 + 1;
    _snwprintf_s(m_wcBuf, m_wcSize, _TRUNCATE, szTimeFormat, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    va_list argList;
    va_start(argList, pszFormat);
    _vsnwprintf_s(&m_wcBuf[wcLen1], m_wcSize - wcLen1 - 2, _TRUNCATE, pszFormat, argList);
    va_end(argList);
    int wcLen = static_cast<int>(wcslen(m_wcBuf));
    if (m_wcBuf[wcLen - 1] != L'\n')
    {
        m_wcBuf[wcLen + 0] = L'\r';
        m_wcBuf[wcLen + 1] = L'\n';
        wcLen += 2;
    }
    int mbLen = WideCharToMultiByte(CP_UTF8, 0, m_wcBuf, wcLen, NULL, 0, NULL, NULL);
    PCHAR mbBuf = GetMbBuf(wcLen, mbLen);
    WideCharToMultiByte(CP_UTF8, 0, m_wcBuf, wcLen, mbBuf, mbLen, NULL, NULL);
    DWORD dwBytes = 0;
    if (!WriteFile(m_hFile, mbBuf, mbLen, &dwBytes, NULL))
    {
        m_dwError = GetLastError();
        bRet = false;
    }
    return bRet;
}


PCHAR LogFile::GetMbBuf(int wcLen, int mbLen)
{
    if (m_wcSize * sizeof(WCHAR) < wcLen * sizeof(WCHAR) + mbLen)
    {
        int wcNew = wcLen + (mbLen + 1) / sizeof(WCHAR);
        PWCHAR wcBuf = (PWCHAR)realloc(m_wcBuf, wcNew * sizeof(WCHAR));
        if (!wcBuf)
        {
            throw std::bad_alloc();
        }
        m_wcBuf = wcBuf;
        m_wcSize = wcNew;
    }
    return (PCHAR)&m_wcBuf[wcLen];
}
