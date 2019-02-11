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


#pragma once


#include <Windows.h>


namespace UiPathTeam
{
    class RegistryKey
    {
    public:

        RegistryKey();
        ~RegistryKey();
        PCWSTR GetPath() const { return m_pszPath; }
        bool Create(HKEY, PCWSTR, REGSAM = KEY_ALL_ACCESS | KEY_WOW64_64KEY, PSECURITY_ATTRIBUTES = 0);
        bool Open(HKEY, PCWSTR, REGSAM = KEY_READ | KEY_WOW64_64KEY);
        bool Close();
        bool GetValue(PCWSTR pszName, std::wstring& strValue);
        bool SetValue(PCWSTR pszName, PCWSTR pszValue, DWORD dwType = REG_SZ);
        bool GetValue(PCWSTR pszName, DWORD& dwValue);
        bool SetValue(PCWSTR pszName, DWORD dwValue);
        bool GetValue(PCWSTR pszName, bool& fValue);
        bool SetValue(PCWSTR pszName, bool fValue);
        HKEY GetHandle() const { return m_hKey; }
        operator HKEY() const { return m_hKey; }
        void ClearError();
        LONG GetError() const { return m_lError; }
        bool IsCreated() const { return m_dwDisposition == REG_CREATED_NEW_KEY; }
        bool IsOpened() const { return m_dwDisposition == REG_OPENED_EXISTING_KEY; }

    private:

        RegistryKey(const RegistryKey&) {}
        void operator =(const RegistryKey&) {}
        bool BuildPath(PCWSTR psz, ...);

        HKEY m_hKey;
        LONG m_lError;
        DWORD m_dwDisposition;
        PWSTR m_pszPath;
        SIZE_T m_ccKey;
    };
}
