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
#include "RegistryKey.h"


using namespace UiPathTeam;


RegistryKey::RegistryKey()
    : m_hKey(NULL)
    , m_lError(ERROR_SUCCESS)
    , m_dwDisposition(0)
    , m_pszPath(NULL)
    , m_ccKey(0)
{
}


RegistryKey::~RegistryKey()
{
    Close();
    free(m_pszPath);
}


static PCWSTR GetRootText(HKEY hKey)
{
    if (hKey == HKEY_LOCAL_MACHINE)
    {
        return L"HKLM\\";
    }
    else if (hKey == HKEY_CURRENT_USER)
    {
        return L"HKCU\\";
    }
    else if (hKey == HKEY_CLASSES_ROOT)
    {
        return L"HKCR\\";
    }
    else if (hKey == HKEY_USERS)
    {
        return L"HKU\\";
    }
    else if (hKey == HKEY_PERFORMANCE_DATA)
    {
        return L"HKPD\\";
    }
    else if (hKey == HKEY_PERFORMANCE_TEXT)
    {
        return L"HKPT\\";
    }
    else if (hKey == HKEY_PERFORMANCE_NLSTEXT)
    {
        return L"HKPN\\";
    }
    else if (hKey == HKEY_CURRENT_CONFIG)
    {
        return L"HKCC\\";
    }
    else if (hKey == HKEY_DYN_DATA)
    {
        return L"HKDD\\";
    }
    else if (hKey == HKEY_CURRENT_USER_LOCAL_SETTINGS)
    {
        return L"HKLS\\";
    }
    else
    {
        return L"HK??\\";
    }
}


bool RegistryKey::Create(HKEY hRootKey, PCWSTR pszSubKey, REGSAM samDesired, PSECURITY_ATTRIBUTES pSecurityAttributes)
{
    if (m_hKey)
    {
        m_lError = ERROR_INVALID_OPERATION;
        return false;
    }
    m_ccKey = 0;
    if (!BuildPath(GetRootText(hRootKey), pszSubKey, NULL))
    {
        return false;
    }
    m_ccKey = wcslen(m_pszPath);
    m_lError = RegCreateKeyExW(hRootKey, pszSubKey, 0, NULL, 0, samDesired, pSecurityAttributes, &m_hKey, &m_dwDisposition);
    return m_lError == ERROR_SUCCESS;
}


bool RegistryKey::Open(HKEY hRootKey, PCWSTR pszSubKey, REGSAM samDesired)
{
    if (m_hKey)
    {
        m_lError = ERROR_INVALID_OPERATION;
        return false;
    }
    m_ccKey = 0;
    if (!BuildPath(GetRootText(hRootKey), pszSubKey, NULL))
    {
        return false;
    }
    m_ccKey = wcslen(m_pszPath);
    m_lError = RegOpenKeyExW(hRootKey, pszSubKey, 0, samDesired, &m_hKey);
    return m_lError == ERROR_SUCCESS;
}


bool RegistryKey::Close()
{
    if (!m_hKey)
    {
        return true;
    }
    HKEY hKey = m_hKey;
    m_hKey = NULL;
    m_lError = RegCloseKey(hKey);
    return m_lError == ERROR_SUCCESS;
}


void RegistryKey::ClearError()
{
    m_lError = ERROR_SUCCESS;
}


bool RegistryKey::BuildPath(PCWSTR psz, ...)
{
    size_t len = m_ccKey;
    va_list argList;
    va_start(argList, psz);
    for (PCWSTR pCur = psz; pCur; pCur = va_arg(argList, PCWSTR))
    {
        len += wcslen(pCur);
    }
    va_end(argList);
    len++;
    m_pszPath = (PWSTR)realloc(m_pszPath, len * sizeof(WCHAR));
    if (!m_pszPath)
    {
        throw std::bad_alloc();
    }
    PWSTR pDst = m_pszPath + m_ccKey;
    PWSTR pEnd = m_pszPath + len;
    va_start(argList, psz);
    for (PCWSTR pCur = psz; pCur; pCur = va_arg(argList, PCWSTR))
    {
        wcscpy_s(pDst, pEnd - pDst, pCur);
        pDst += wcslen(pCur);
    }
    va_end(argList);
    return true;
}


static void ExpandString(PCWSTR pszSource, DWORD dwSourceSize, std::wstring& strDestination)
{
    DWORD dwSize = dwSourceSize;
    if (!dwSize)
    {
        dwSize = 32;
    }
    for (;;)
    {
        dwSize *= 2;
        WCHAR* pData = new WCHAR[dwSize];
        if (!pData)
        {
            throw std::bad_alloc();
        }
        if (ExpandEnvironmentStringsW(pszSource, pData, dwSize) <= dwSize)
        {
            strDestination = pData;
            delete[] pData;
            break;
        }
        delete[] pData;
    }
}


bool RegistryKey::GetValue(PCWSTR pszName, std::wstring& strValue)
{
    if (!BuildPath(L"\\", pszName, NULL))
    {
        return false;
    }
    DWORD dwType = 0;
    DWORD dwSize = 0;
    m_lError = RegQueryValueExW(m_hKey, pszName, NULL, &dwType, NULL, &dwSize);
    if (m_lError == ERROR_SUCCESS)
    {
        if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
        {
            WCHAR* pData = new WCHAR[(dwSize + 1) / sizeof(WCHAR)];
            if (!pData)
            {
                throw std::bad_alloc();
            }
            m_lError = RegQueryValueExW(m_hKey, pszName, NULL, &dwType, (LPBYTE)pData, &dwSize);
            if (m_lError == ERROR_SUCCESS)
            {
                if (dwType == REG_SZ)
                {
                    strValue.assign(pData, dwSize / sizeof(WCHAR));
                    delete[] pData;
                    return true;
                }
                else if (dwType == REG_EXPAND_SZ)
                {
                    strValue.assign(pData, dwSize / sizeof(WCHAR));
                    ExpandString(strValue.c_str(), dwSize / sizeof(WCHAR), strValue);
                    delete[] pData;
                    return true;
                }
                else
                {
                    m_lError = ERROR_INVALID_DATATYPE;
                }
            }
            delete[] pData;
        }
        else
        {
            m_lError = ERROR_INVALID_DATATYPE;
        }
    }
    return false;
}


bool RegistryKey::SetValue(PCWSTR pszName, PCWSTR pszValue, DWORD dwType)
{
    if (!BuildPath(L"\\", pszName, NULL))
    {
        return false;
    }
    m_lError = RegSetValueExW(m_hKey, pszName, 0, dwType, (BYTE*)pszValue, (DWORD)((wcslen(pszValue) + 1) * sizeof(WCHAR)));
    return m_lError == ERROR_SUCCESS;
}


bool RegistryKey::GetValue(PCWSTR pszName, DWORD& dwValue)
{
    if (!BuildPath(L"\\", pszName, NULL))
    {
        return false;
    }
    DWORD dwType = 0;
    DWORD dwSize = (DWORD)sizeof(dwValue);
    m_lError = RegQueryValueExW(m_hKey, pszName, NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
    if (m_lError == ERROR_SUCCESS)
    {
        if (dwType == REG_DWORD)
        {
            if (dwSize == (DWORD)sizeof(dwValue))
            {
                return true;
            }
            else
            {
                m_lError = ERROR_INVALID_DATA;
            }
        }
        else
        {
            m_lError = ERROR_INVALID_DATATYPE;
        }
    }
    return false;
}


bool RegistryKey::SetValue(PCWSTR pszName, DWORD dwValue)
{
    if (!BuildPath(L"\\", pszName, NULL))
    {
        return false;
    }
    m_lError = RegSetValueExW(m_hKey, pszName, 0, REG_DWORD, (BYTE*)&dwValue, (DWORD)sizeof(dwValue));
    return m_lError == ERROR_SUCCESS;
}


bool RegistryKey::GetValue(PCWSTR pszName, bool& fValue)
{
    DWORD dwValue;
    if (GetValue(pszName, dwValue))
    {
        fValue = dwValue ? true : false;
        return true;
    }
    else
    {
        return false;
    }
}


bool RegistryKey::SetValue(PCWSTR pszName, bool fValue)
{
    return SetValue(pszName, fValue ? 1UL : 0UL);
}
