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
#include "Service.h"


using namespace UiPathTeam;


Service::Service()
    : ServiceBase()
    , m_pszName(NULL)
{
}


Service::~Service()
{
    free(m_pszName);
}


bool Service::Create(SC_HANDLE hSCM, PCWSTR pszName, PCWSTR pszDisplayName, PCWSTR pszDescription, PCWSTR pszFileName)
{
    if (m_hService)
    {
        m_dwError = ERROR_INVALID_OPERATION;
        return false;
    }
    free(m_pszName);
    m_pszName = _wcsdup(pszName);
    if (!m_pszName)
    {
        m_dwError = ERROR_OUTOFMEMORY;
        return false;
    }
    WCHAR szFileName[MAX_PATH] = { 0 };
    if (!pszFileName)
    {
        if (!GetModuleFileNameW(NULL, szFileName, MAX_PATH))
        {
            m_dwError = GetLastError();
            return false;
        }
        pszFileName = szFileName;
    }
    m_hService = CreateServiceW(hSCM, pszName, pszDisplayName, GENERIC_ALL, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_CRITICAL, pszFileName, NULL, NULL, NULL, NULL, L"");
    if (m_hService)
    {
        m_dwError = ERROR_SUCCESS;
        if (pszDescription)
        {
            SERVICE_DESCRIPTION sd;
            sd.lpDescription = _wcsdup(pszDescription);
            if (sd.lpDescription)
            {
                if (!ChangeServiceConfig2W(m_hService, SERVICE_CONFIG_DESCRIPTION, &sd))
                {
                    m_dwError = GetLastError();
                }
                free(sd.lpDescription);
            }
            else
            {
                m_dwError = ERROR_OUTOFMEMORY;
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


bool Service::Open(SC_HANDLE hSCM, PCWSTR pszName)
{
    if (m_hService)
    {
        m_dwError = ERROR_INVALID_OPERATION;
        return false;
    }
    free(m_pszName);
    m_pszName = _wcsdup(pszName);
    if (!m_pszName)
    {
        m_dwError = ERROR_OUTOFMEMORY;
        return false;
    }
    m_hService = OpenServiceW(hSCM, pszName, SERVICE_ALL_ACCESS);
    if (m_hService)
    {
        return true;
    }
    else
    {
        m_dwError = GetLastError();
        return false;
    }
}


bool Service::Delete()
{
    if (DeleteService(m_hService))
    {
        return true;
    }
    else
    {
        m_dwError = GetLastError();
        return false;
    }
}
