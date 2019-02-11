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
#include "LogFile.h"


namespace UiPathTeam
{
    class SessionLoggerService
    {
    public:

        static SessionLoggerService& Instance();

        SessionLoggerService();
        ~SessionLoggerService();
        bool Install();
        bool Uninstall();
        bool Start();
        void MainLoop();
        bool OnStart();
        DWORD OnStop();
        DWORD OnPause();
        DWORD OnContinue();
        DWORD OnInterrogate();
        DWORD OnShutdown();
        DWORD OnSessionChange(DWORD, DWORD);
        DWORD OnUnknownRequest(DWORD);
        inline PCWSTR GetServiceName() const;
        inline PCWSTR GetDisplayName() const;
        inline DWORD GetError() const;
        inline PCWSTR GetLogFileName();
        inline void SetLogFileName(PCWSTR);

    private:

        SessionLoggerService(const SessionLoggerService&) {}
        void operator =(const SessionLoggerService&) {}
        static VOID WINAPI ServiceMain(DWORD dwArgc, PWSTR* pszArgv);
        static DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
        bool SetStatus(DWORD dwState, DWORD dwPreviousStatus = 0, DWORD dwWaitHint = 0);

        class Mutex
        {
        public:

            inline Mutex();
            inline ~Mutex();

        private:

            Mutex(const Mutex&) {}
            void operator =(const Mutex&) {}
        };

        static SessionLoggerService* m_pSingleton;

        PWSTR m_pszServiceName;
        PWSTR m_pszDisplayName;
        DWORD m_dwError;
        SERVICE_STATUS_HANDLE m_hServiceStatus;
        DWORD m_dwCurrentState;
        DWORD m_dwCheckPoint;
        LONG m_ExclusiveOperation;
        HANDLE m_hEventMain;
        LogFile m_LogFile;
    };


    inline PCWSTR SessionLoggerService::GetServiceName() const
    {
        return m_pszServiceName;
    }


    inline PCWSTR SessionLoggerService::GetDisplayName() const
    {
        return m_pszDisplayName;
    }


    inline DWORD SessionLoggerService::GetError() const
    {
        return m_dwError;
    }


    inline SessionLoggerService::Mutex::Mutex()
    {
        while (InterlockedCompareExchange(&m_pSingleton->m_ExclusiveOperation, 1L, 0L))
        {
            Sleep(0);
        }
    }


    inline SessionLoggerService::Mutex::~Mutex()
    {
        InterlockedExchange(&m_pSingleton->m_ExclusiveOperation, 0);
    }


    inline PCWSTR SessionLoggerService::GetLogFileName()
    {
        return m_LogFile.GetFileName();
    }


    inline void SessionLoggerService::SetLogFileName(PCWSTR pszFileName)
    {
        m_LogFile.SetFileName(pszFileName);
    }
}
