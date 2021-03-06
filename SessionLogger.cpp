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
#include "ResourceString.h"
#include "ErrorMessage.h"
#include "resource.h"
#include <locale.h>


using namespace UiPathTeam;


int wmain(int argc, wchar_t* argv[])
{
    try
    {
        _wsetlocale(LC_ALL, L"");

        SessionLoggerService svc;

        wchar_t** ppCur = &argv[1];
        wchar_t** ppEnd = &argv[argc];

        if (ppCur < ppEnd)
        {
            wchar_t* psz = *ppCur;
            if (!_wcsicmp(psz, L"install"))
            {
                for (ppCur++; ppCur < ppEnd; ppCur++)
                {
                    psz = *ppCur;
                    if (!_wcsicmp(psz, L"-logfile"))
                    {
                        ppCur++;
                        if (ppCur < ppEnd)
                        {
                            psz = *ppCur;
                            svc.SetLogFileName(psz);
                        }
                        else
                        {
                            fwprintf(stderr, ResourceString(IDS_ERROR_CMDLINE));
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        fwprintf(stderr, ResourceString(IDS_ERROR_CMDLINE));
                        return EXIT_FAILURE;
                    }
                }
                if (svc.Install())
                {
                    return EXIT_SUCCESS;
                }
                else
                {
                    return EXIT_FAILURE;
                }
            }
            else if (!_wcsicmp(psz, L"uninstall"))
            {
                ppCur++;
                if (svc.Uninstall())
                {
                    return EXIT_SUCCESS;
                }
                else
                {
                    return EXIT_FAILURE;
                }
            }
            else if (!_wcsicmp(psz, L"help"))
            {
                WCHAR szName[MAX_PATH];
                wcscpy_s(szName, wcschr(argv[0], L'\\') ? (wcsrchr(argv[0], L'\\') + 1) : argv[0]);
                if (wcschr(argv[0], L'.'))
                {
                    *wcsrchr(argv[0], L'.') = L'\0';
                }
                fwprintf(stderr, ResourceString(IDS_USAGE), szName, szName, svc.GetServiceName(), svc.GetServiceName());
                return EXIT_SUCCESS;
            }
        }

        if (ppCur < ppEnd)
        {
            fwprintf(stderr, ResourceString(IDS_ERROR_CMDLINE));
        }
        else if (svc.Start())
        {
            return EXIT_SUCCESS;
        }
        else
        {
            fwprintf(stderr, ResourceString(IDS_ERROR), ErrorMessage(svc.GetError()).Ptr());

        }
    }
    catch (std::bad_alloc ex)
    {
        (void)ex;
        fwprintf(stderr, ResourceString(IDS_ERROR_OUTOFMEMORY));
    }
    catch (...)
    {
        fwprintf(stderr, ResourceString(IDS_ERROR_UNHANDLED_EXCEPTION));
    }

    return EXIT_FAILURE;
}
