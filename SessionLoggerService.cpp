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
#include "SessionLoggerService.h"
#include "SessionLoggerServiceDefs.h"
#include "ServiceControlManager.h"
#include "Service.h"
#include "Session.h"
#include "ResourceString.h"
#include "ErrorMessage.h"
#include "RegistryKey.h"
#include "Path.h"
#include "Directory.h"
#include "resource.h"


using namespace UiPathTeam;


SessionLoggerService* SessionLoggerService::m_pSingleton = NULL;


SessionLoggerService& SessionLoggerService::Instance()
{
    return *m_pSingleton;
}


SessionLoggerService::SessionLoggerService()
    : m_pszServiceName(_wcsdup(SERVICE_NAME))
    , m_pszDisplayName(_wcsdup(ResourceString(IDS_DISPLAYNAME)))
    , m_dwError(ERROR_SUCCESS)
    , m_hServiceStatus(NULL)
    , m_dwCurrentState(0)
    , m_dwCheckPoint(0)
    , m_ExclusiveOperation(0)
    , m_hEventMain(CreateEventW(NULL, TRUE, FALSE, NULL))
{
    if (!m_pszServiceName || !m_pszDisplayName)
    {
        throw std::bad_alloc();
    }

    if (InterlockedCompareExchangePointer(reinterpret_cast<void**>(&m_pSingleton), this, NULL))
    {
        throw std::runtime_error("Two SessionLoggerService class instantiated.");
    }
}


SessionLoggerService::~SessionLoggerService()
{
    InterlockedExchangePointer(reinterpret_cast<void**>(&m_pSingleton), NULL);

    CloseHandle(m_hEventMain);

    free(m_pszServiceName);
    free(m_pszDisplayName);
}


static PWCHAR GetDescription(WCHAR szBuf[], SIZE_T ccSize, UINT uId)
{
    if (GetModuleFileNameW(NULL, &szBuf[1], static_cast<DWORD>(ccSize - 1)))
    {
        szBuf[0] = L'@';
        SIZE_T ccLen = wcslen(szBuf);
        _snwprintf_s(szBuf + ccLen, ccSize - ccLen, _TRUNCATE, L",-%u", uId);
    }
    else
    {
        wcscpy_s(szBuf, ccSize, ResourceString(uId));
    }
    return szBuf;
}


bool SessionLoggerService::Install()
{
    ServiceControlManager hSCM;
    Service hSvc;

    if (!hSCM.Open())
    {
        fwprintf(stderr, ResourceString(IDS_FAILURE_SCM_OPEN), m_pszServiceName, ErrorMessage(hSCM.GetError()).Ptr());
        return false;
    }

    WCHAR szDescription[MAX_PATH + 16];

    if (!hSvc.Create(hSCM, m_pszServiceName, m_pszDisplayName, GetDescription(szDescription, _countof(szDescription), IDS_DESCRIPTION)))
    {
        fwprintf(stderr, ResourceString(IDS_FAILURE_SVC_CREATE), m_pszServiceName, ErrorMessage(hSvc.GetError()).Ptr());
        return false;
    }

    fwprintf(stderr, ResourceString(IDS_SVC_INSTALLED), m_pszServiceName);
    //fwprintf(stderr, L"%s\n", szDescription);

    RegistryKey key;
    if (key.Create(REGROOT, REGKEY))
    {
        if (m_LogFile.GetFileName())
        {
            if (!key.SetValue(REGVAL_LOGFILENAME, m_LogFile.GetFileName()))
            {
                fwprintf(stderr, ResourceString(IDS_ERROR_SETLOGFILENAME), ErrorMessage(key.GetError()).Ptr());
            }
        }
        else if (key.IsCreated())
        {
            if (!key.SetValue(REGVAL_LOGFILENAME, LOGFILENAME_DEFAULT, REG_EXPAND_SZ))
            {
                fwprintf(stderr, ResourceString(IDS_ERROR_SETLOGFILENAME), ErrorMessage(key.GetError()).Ptr());
            }
        }
        std::wstring strLogFileName;
        if (key.GetValue(REGVAL_LOGFILENAME, strLogFileName))
        {
            fwprintf(stderr, ResourceString(IDS_LOGFILE), key.GetPath(), strLogFileName.c_str());
        }
    }

    return true;
}


bool SessionLoggerService::Uninstall()
{
    ServiceControlManager hSCM;
    Service hSvc;

    if (!hSCM.Open())
    {
        fwprintf(stderr, ResourceString(IDS_FAILURE_SCM_OPEN), m_pszServiceName, ErrorMessage(hSCM.GetError()).Ptr());
        return false;
    }

    if (!hSvc.Open(hSCM, m_pszServiceName))
    {
        fwprintf(stderr, ResourceString(IDS_FAILURE_SVC_OPEN), m_pszServiceName, ErrorMessage(hSvc.GetError()).Ptr());
        return false;
    }

    if (!hSvc.Delete())
    {
        fwprintf(stderr, ResourceString(IDS_FAILURE_SVC_DELETE), m_pszServiceName, ErrorMessage(hSvc.GetError()).Ptr());
        return false;
    }

    fwprintf(stderr, ResourceString(IDS_SVC_UNINSTALLED), m_pszServiceName);

    return true;
}


bool SessionLoggerService::Start()
{
    try
    {
        SERVICE_TABLE_ENTRYW ServiceTable[] =
        {
            { m_pszServiceName, ServiceMain },
            { NULL, NULL }
        };
        if (StartServiceCtrlDispatcherW(ServiceTable))
        {
            return true;
        }
        else
        {
            m_dwError = GetLastError();
            return false;
        }
    }
    catch (std::bad_alloc ex)
    {
        (void)ex;
        m_LogFile.Put(L"ERROR: Ouf of memory.");
        throw;
    }
}


VOID WINAPI SessionLoggerService::ServiceMain(DWORD dwArgc, PWSTR* pszArgv)
{
    SessionLoggerService& svc = SessionLoggerService::Instance();

    if (!svc.OnStart())
    {
        return;
    }

    if (svc.SetStatus(SERVICE_RUNNING, SERVICE_START_PENDING))
    {
        svc.MainLoop();
    }

    svc.SetStatus(SERVICE_STOPPED);
}


bool SessionLoggerService::OnStart()
{
    m_hServiceStatus = RegisterServiceCtrlHandlerExW(m_pszServiceName, HandlerEx, this);
    if (m_hServiceStatus)
    {
        std::wstring strLogFileName = LOGFILENAME_DEFAULT;
        RegistryKey key;
        if (key.Open(REGROOT, REGKEY))
        {
            key.GetValue(REGVAL_LOGFILENAME, strLogFileName);
        }
        std::wstring strParent = Path::GetDirectory(strLogFileName);
        if (!Directory::Exists(strParent))
        {
            Directory::Create(strParent);
        }
        m_LogFile.SetFileName(strLogFileName.c_str());
        m_LogFile.Open();
        SetStatus(SERVICE_START_PENDING);
        return true;
    }
    else
    {
        m_dwError = GetLastError();
        Mutex lock;
        _InterlockedExchange(&m_dwCurrentState, SERVICE_STOPPED);
        m_dwCheckPoint = 0;
        return false;
    }
}


void SessionLoggerService::MainLoop()
{
    std::list<Session> sessionList;
    if (Session::Enumerate(sessionList))
    {
        for (std::list<Session>::const_iterator iter = sessionList.begin(); iter != sessionList.end(); iter++)
        {
            WCHAR buf[300];
            m_LogFile.Put(L"%s", iter->ToString(buf, _countof(buf)));
        }
    }

    while (m_dwCurrentState != SERVICE_STOP_PENDING)
    {
        switch (m_dwCurrentState)
        {
        case SERVICE_PAUSE_PENDING:
            SetStatus(SERVICE_PAUSED, SERVICE_PAUSE_PENDING);
            ResetEvent(m_hEventMain);
            break;

        case SERVICE_CONTINUE_PENDING:
            SetStatus(SERVICE_RUNNING, SERVICE_CONTINUE_PENDING);
            ResetEvent(m_hEventMain);
            break;

        default:
            break;
        }

        WaitForSingleObject(m_hEventMain, 3000);
    }
}


DWORD WINAPI SessionLoggerService::HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    SessionLoggerService* pSvc = reinterpret_cast<SessionLoggerService*>(lpContext);

    switch (dwControl) {
    case SERVICE_CONTROL_INTERROGATE:
        return pSvc->OnInterrogate();

    case SERVICE_CONTROL_STOP:
        return pSvc->OnStop();

    case SERVICE_CONTROL_PAUSE:
        return pSvc->OnPause();

    case SERVICE_CONTROL_CONTINUE:
        return pSvc->OnContinue();

    case SERVICE_CONTROL_SHUTDOWN:
        return pSvc->OnShutdown();

    case SERVICE_CONTROL_SESSIONCHANGE:
        return pSvc->OnSessionChange(dwEventType, reinterpret_cast<WTSSESSION_NOTIFICATION*>(lpEventData)->dwSessionId);

    default:
        return pSvc->OnUnknownRequest(dwControl);
    }
}


DWORD SessionLoggerService::OnStop()
{
    m_LogFile.Put(L"SERVICE_CONTROL_STOP");
    if (SetStatus(SERVICE_STOP_PENDING))
    {
        SetEvent(m_hEventMain);
        return NO_ERROR;
    }
    else
    {
        return m_dwError;
    }
}


DWORD SessionLoggerService::OnPause()
{
    m_LogFile.Put(L"SERVICE_CONTROL_PAUSE");
    if (SetStatus(SERVICE_PAUSE_PENDING))
    {
        SetEvent(m_hEventMain);
        return NO_ERROR;
    }
    else
    {
        return m_dwError;
    }
}


DWORD SessionLoggerService::OnContinue()
{
    m_LogFile.Put(L"SERVICE_CONTROL_CONTINUE");
    if (SetStatus(SERVICE_CONTINUE_PENDING))
    {
        SetEvent(m_hEventMain);
        return NO_ERROR;
    }
    else
    {
        return m_dwError;
    }
}


DWORD SessionLoggerService::OnInterrogate()
{
    m_LogFile.Put(L"SERVICE_CONTROL_INTERROGATE");
    return NO_ERROR;
}


DWORD SessionLoggerService::OnShutdown()
{
    m_LogFile.Put(L"SERVICE_CONTROL_SHUTDOWN");
    SetStatus(SERVICE_STOP_PENDING);
    SetEvent(m_hEventMain);
    return NO_ERROR;
}


DWORD SessionLoggerService::OnSessionChange(DWORD dwEventType, DWORD dwSessionId)
{
    Session session(dwSessionId);
    WCHAR szInfo[300];
    session.ToString(szInfo, _countof(szInfo));
    switch (dwEventType)
    {
    case WTS_CONSOLE_CONNECT: // The session identified by lParam was connected to the console terminal or RemoteFX session.
        m_LogFile.Put(L"CONSOLE_CONNECT %s", szInfo);
        break;
    case WTS_CONSOLE_DISCONNECT: // The session identified by lParam was disconnected from the console terminal or RemoteFX session.
        m_LogFile.Put(L"CONSOLE_DISCONNECT %s", szInfo);
        break;
    case WTS_REMOTE_CONNECT: // The session identified by lParam was connected to the remote terminal.
        m_LogFile.Put(L"REMOTE_CONNECT %s", szInfo);
        break;
    case WTS_REMOTE_DISCONNECT:// The session identified by lParam was disconnected from the remote terminal.
        m_LogFile.Put(L"REMOTE_DISCONNECT %s", szInfo);
        break;
    case WTS_SESSION_LOGON: // A user has logged on to the session identified by lParam.
        m_LogFile.Put(L"SESSION_LOGON %s", szInfo);
        break;
    case WTS_SESSION_LOGOFF: // A user has logged off the session identified by lParam.
        m_LogFile.Put(L"SESSION_LOGOFF %s", szInfo);
        break;
    case WTS_SESSION_LOCK: // The session identified by lParam has been locked.
        m_LogFile.Put(L"SESSION_LOCK %s", szInfo);
        break;
    case WTS_SESSION_UNLOCK: // The session identified by lParam has been unlocked.
        m_LogFile.Put(L"SESSION_UNLOCK %s", szInfo);
        break;
    case WTS_SESSION_REMOTE_CONTROL: // The session identified by lParam has changed its remote controlled status.To determine the status, call GetSystemMetrics and check the SM_REMOTECONTROL metric.
        m_LogFile.Put(L"SESSION_REMOTE_CONTROL %s", szInfo);
        break;
    case WTS_SESSION_CREATE: // Reserved for future use.
        m_LogFile.Put(L"SESSION_CREATE %s", szInfo);
        break;
    case WTS_SESSION_TERMINATE: // Reserved for future use.
        m_LogFile.Put(L"SESSION_TERMINATE %s", szInfo);
        break;
    default:
        m_LogFile.Put(L"SERVICE_CONTROL_SESSIONCHANGE: 0x%04lX", dwEventType);
        break;
    }
    return NO_ERROR;
}


DWORD SessionLoggerService::OnUnknownRequest(DWORD dwControl)
{
    m_LogFile.Put(L"Control=%lu not implemented.", dwControl);
    return ERROR_CALL_NOT_IMPLEMENTED;
}


bool SessionLoggerService::SetStatus(DWORD dwState, DWORD dwPreviousStatus, DWORD dwWaitHint)
{
    Mutex lock;

    if (dwPreviousStatus)
    {
        if (_InterlockedCompareExchange(&m_dwCurrentState, dwState, dwPreviousStatus) != dwPreviousStatus)
        {
            //m_LogFile.Put(L"# SetStatus failed. Current=%ld New=%ld Expected=%ld", m_dwCurrentState, dwState, dwPreviousStatus);
            return false;
        }
    }
    else
    {
        dwPreviousStatus = _InterlockedExchange(&m_dwCurrentState, dwState);
    }

    if (dwPreviousStatus != dwState)
    {
        m_dwCheckPoint = 0;
    }

    SERVICE_STATUS ss = { 0 };

    ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ss.dwCurrentState = dwState;
    ss.dwCheckPoint = ++m_dwCheckPoint;
    ss.dwWaitHint = dwWaitHint;
    switch (dwState)
    {
    case SERVICE_STOPPED:
        break;
    case SERVICE_START_PENDING:
        ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        break;
    case SERVICE_STOP_PENDING:
        break;
    case SERVICE_RUNNING:
        ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_PAUSE_CONTINUE;
        break;
    case SERVICE_CONTINUE_PENDING:
        ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;
        break;
    case SERVICE_PAUSE_PENDING:
        ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;
        break;
    case SERVICE_PAUSED:
        ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_PAUSE_CONTINUE;
        break;
    default:
        break;
    }

    if (SetServiceStatus(m_hServiceStatus, &ss))
    {
        switch (dwState)
        {
        case SERVICE_STOPPED:
            m_LogFile.Put(L"SetServiceStatus(STOPPED)");
            break;
        case SERVICE_START_PENDING:
            m_LogFile.Put(L"SetServiceStatus(START_PENDING) CheckPoint=%lu WaitHint=%lu", ss.dwCheckPoint, ss.dwWaitHint);
            break;
        case SERVICE_STOP_PENDING:
            m_LogFile.Put(L"SetServiceStatus(STOP_PENDING)");
            break;
        case SERVICE_RUNNING:
            m_LogFile.Put(L"SetServiceStatus(RUNNING)");
            break;
        case SERVICE_CONTINUE_PENDING:
            m_LogFile.Put(L"SetServiceStatus(CONTINUE_PENDING)");
            break;
        case SERVICE_PAUSE_PENDING:
            m_LogFile.Put(L"SetServiceStatus(PAUSE_PENDING)");
            break;
        case SERVICE_PAUSED:
            m_LogFile.Put(L"SetServiceStatus(PAUSED)");
            break;
        default:
            m_LogFile.Put(L"SetServiceStatus(0x%04X)", dwState);
            break;
        }
        return true;
    }
    else
    {
        m_dwError = GetLastError();
        m_LogFile.Put(L"SetServiceStatus failed. %s", ErrorMessage(m_dwError).Ptr());
        return false;
    }
}
