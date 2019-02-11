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
#include "Session.h"
#include <Wtsapi32.h>


#pragma comment(lib,"Wtsapi32.lib")


using namespace UiPathTeam;


#define PROTOCOL_TYPE_UNSET static_cast<USHORT>(-1)
#define CONNECTION_STATE_UNSET (-1)


bool Session::Enumerate(std::list<Session>& sessionList)
{
    PWTS_SESSION_INFOW pSessionInfo = NULL;
    DWORD dwCount = 0;
    if (WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
    {
        if (pSessionInfo)
        {
            for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
            {
                Session session(pSessionInfo[dwIndex].SessionId);
                sessionList.push_back(session);
            }
            WTSFreeMemory(pSessionInfo);
        }
        return true;
    }
    else
    {
        return false;
    }
}


Session::Session(DWORD dwSessionId)
    : m_dwSessionId(dwSessionId)
    , m_pszUserName(NULL)
    , m_pszDomainName(NULL)
    , m_wProtocolType(PROTOCOL_TYPE_UNSET)
    , m_ConnectState(CONNECTION_STATE_UNSET)
{
    void* pData;
    DWORD dwBytes;

    if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSUserName, (LPWSTR*)&pData, &dwBytes))
    {
        m_pszUserName = _wcsdup(reinterpret_cast<LPWSTR>(pData));
        WTSFreeMemory(pData);
    }

    if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSDomainName, (LPWSTR*)&pData, &dwBytes))
    {
        m_pszDomainName = _wcsdup(reinterpret_cast<LPWSTR>(pData));
        WTSFreeMemory(pData);
    }

    if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSClientProtocolType, (LPWSTR*)&pData, &dwBytes))
    {
        if (dwBytes == sizeof(USHORT))
        {
            m_wProtocolType = *reinterpret_cast<USHORT*>(pData);
        }
        WTSFreeMemory(pData);
    }

    if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSConnectState, (LPWSTR*)&pData, &dwBytes))
    {
        if (dwBytes == sizeof(WTS_CONNECTSTATE_CLASS))
        {
            m_ConnectState = *reinterpret_cast<WTS_CONNECTSTATE_CLASS*>(pData);
        }
        WTSFreeMemory(pData);
    }
}


Session::~Session()
{
    free(m_pszUserName);
    free(m_pszDomainName);
}


Session::Session(const Session& src)
    : m_dwSessionId(src.m_dwSessionId)
    , m_pszUserName(src.m_pszUserName ? _wcsdup(src.m_pszUserName) : NULL)
    , m_pszDomainName(src.m_pszDomainName ? _wcsdup(src.m_pszDomainName) : NULL)
    , m_wProtocolType(src.m_wProtocolType)
    , m_ConnectState(src.m_ConnectState)
{
}


void Session::operator =(const Session& src)
{
    m_dwSessionId = src.m_dwSessionId;
    free(m_pszUserName);
    m_pszUserName = src.m_pszUserName ? _wcsdup(src.m_pszUserName) : NULL;
    free(m_pszDomainName);
    m_pszDomainName = src.m_pszDomainName ? _wcsdup(src.m_pszDomainName) : NULL;
    m_wProtocolType = src.m_wProtocolType;
    m_ConnectState = src.m_ConnectState;
}


PCWSTR Session::ToString(PWCHAR pBuf, SIZE_T ccSize) const
{
    PWCHAR pCur = pBuf;
    PWCHAR pEnd = pBuf + ccSize;

    _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L"SessionId=%lu", m_dwSessionId);
    pCur += wcslen(pCur);

    if (m_pszUserName && *m_pszUserName)
    {
        if (m_pszDomainName && *m_pszDomainName)
        {
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" User=%s\\%s", m_pszDomainName, m_pszUserName);
        }
        else
        {
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" User=%s", m_pszUserName);
        }
        pCur += wcslen(pCur);
    }

    if (m_wProtocolType != PROTOCOL_TYPE_UNSET)
    {
        if (m_wProtocolType == 0)
        {
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" Protocol=CONSOLE");
        }
        else if (m_wProtocolType == 2)
        {
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" Protocol=RDP");
        }
        else
        {
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" Protocol=%u", m_wProtocolType);
        }
        pCur += wcslen(pCur);
    }

    if (m_ConnectState != CONNECTION_STATE_UNSET)
    {
        switch (m_ConnectState)
        {
        case WTSActive:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Active");
            break;
        case WTSConnected:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Connected");
            break;
        case WTSConnectQuery:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=ConnectQuery");
            break;
        case WTSShadow:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Shadow");
            break;
        case WTSDisconnected:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Disconnected");
            break;
        case WTSIdle:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Idle");
            break;
        case WTSListen:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Listen");
            break;
        case WTSReset:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Reset");
            break;
        case WTSDown:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Down");
            break;
        case WTSInit:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=Init");
            break;
        default:
            _snwprintf_s(pCur, pEnd - pCur, _TRUNCATE, L" State=%d", m_ConnectState);
            break;
        }
        pCur += wcslen(pCur);
    }

    return pBuf;
}
