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
    class LogFile
    {
    public:

        LogFile(PCWSTR = 0);
        virtual ~LogFile();
        inline PCWSTR GetFileName() const;
        inline DWORD GetError() const;
        inline void ClearError();
        inline operator HANDLE() const;
        inline HANDLE Handle() const;
        void SetFileName(PCWSTR);
        bool Close();
        bool Open();
        bool Put(PCWSTR, ...);

    protected:

        LogFile(const LogFile&) {}
        void operator =(const LogFile&) {}
        PCHAR GetMbBuf(int wcLen, int mbLen);

        class Mutex
        {
        public:

            inline Mutex(LogFile*);
            inline ~Mutex();

        private:

            Mutex(const Mutex&) {}
            void operator =(const Mutex&) {}

            volatile LONG* m_ptr;
        };

        PWSTR m_pszFileName;
        DWORD m_dwError;
        HANDLE m_hFile;
        PWCHAR m_wcBuf;
        int m_wcSize;
        LONG m_mutex;
    };


    inline PCWSTR LogFile::GetFileName() const
    {
        return m_pszFileName;
    }


    inline DWORD LogFile::GetError() const
    {
        return m_dwError;
    }


    inline void LogFile::ClearError()
    {
        m_dwError = 0;
    }


    inline LogFile::operator HANDLE() const
    {
        return m_hFile;
    }


    inline HANDLE LogFile::Handle() const
    {
        return m_hFile;
    }


    inline LogFile::Mutex::Mutex(LogFile* pLogFile)
        : m_ptr(&pLogFile->m_mutex)
    {
        while (InterlockedCompareExchange(m_ptr, 1L, 0L))
        {
            Sleep(0);
        }
    }


    inline LogFile::Mutex::~Mutex()
    {
        InterlockedExchange(m_ptr, 0L);
    }
}
