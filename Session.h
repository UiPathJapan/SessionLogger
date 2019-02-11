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
#include <list>


namespace UiPathTeam
{
    class Session
    {
    public:

        static bool Enumerate(std::list<Session>&);

        Session(DWORD);
        ~Session();
        Session(const Session&);
        void operator =(const Session&);
        PCWSTR ToString(PWCHAR, SIZE_T) const;
        inline DWORD Id() const;
        inline PCWSTR UserName() const;
        inline PCWSTR DomainName() const;
        inline USHORT ProtocolType() const;
        inline INT ConnectState() const;

    private:

        DWORD m_dwSessionId;
        PWSTR m_pszUserName;
        PWSTR m_pszDomainName;
        USHORT m_wProtocolType;
        INT m_ConnectState;
    };


    inline DWORD Session::Id() const
    {
        return m_dwSessionId;
    }


    inline PCWSTR Session::UserName() const
    {
        return m_pszUserName;
    }


    inline PCWSTR Session::DomainName() const
    {
        return m_pszDomainName;
    }


    inline USHORT Session::ProtocolType() const
    {
        return m_wProtocolType;
    }


    inline INT Session::ConnectState() const
    {
        return m_ConnectState;
    }
}
