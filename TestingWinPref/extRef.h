#pragma once
#define _WIN32_DCOM

#include <iostream>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

int __cdecl wmain(int argc, wchar_t* argv[])
{
    HRESULT                 hr = S_OK;
    IWbemRefresher* pRefresher = NULL;
    IWbemConfigureRefresher* pConfig = NULL;
    IWbemHiPerfEnum* pEnum = NULL;
    IWbemServices* pNameSpace = NULL;
    IWbemLocator* pWbemLocator = NULL;
    IWbemObjectAccess** apEnumAccess = NULL;
    BSTR                    bstrNameSpace = NULL;
    long                    lID = 0;
    long                    lVirtualBytesHandle = 0;
    long                    lIDProcessHandle = 0;
    long                    lPercentProcessorTime = 0;
    long                    lThreadCount = 0;
    DWORD                   dwVirtualBytes = 0;
    DWORD                   dwProcessId = 0;
    DWORD                   dwNumObjects = 0;
    DWORD                   dwNumReturned = 0;
    DWORD                   dwIDProcess = 0;
    DWORD                   dwCPU_TIME = 0;
    DWORD                   dwCPU_THREADS_NUMBER = 0;
    DWORD                   i = 0;
    int                     x = 0;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&pWbemLocator);

    bstrNameSpace = SysAllocString(L"\\\\.\\root\\cimv2");
    pWbemLocator->ConnectServer(
        bstrNameSpace,
        NULL, // User name
        NULL, // Password
        NULL, // Locale
        0L,   // Security flags
        NULL, // Authority
        NULL, // Wbem context
        &pNameSpace);
    pWbemLocator->Release();
    pWbemLocator = NULL;
    SysFreeString(bstrNameSpace);
    bstrNameSpace = NULL;
    hr = CoCreateInstance(CLSID_WbemRefresher, NULL, CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (void**)&pRefresher);
    hr = pRefresher->QueryInterface(IID_IWbemConfigureRefresher, (void**)&pConfig);
    hr = pConfig->AddEnum(pNameSpace, L"Win32_PerfRawData_PerfProc_Process", 0, NULL, &pEnum, &lID);
    pConfig->Release();
    pConfig = NULL;

    // Get a property handle for the VirtualBytes property.

    // Refresh the object ten times and retrieve the value.
    for (x = 0; x < 1; x++)
    {
        dwNumReturned = 0;
        dwIDProcess = 0;
        dwNumObjects = 0;
        dwCPU_TIME = 0;

        hr = pRefresher->Refresh(0L);

        hr = pEnum->GetObjects(0L,
            dwNumObjects,
            apEnumAccess,
            &dwNumReturned);
        if (hr == WBEM_E_BUFFER_TOO_SMALL
            && dwNumReturned > dwNumObjects)
        {
            apEnumAccess = new IWbemObjectAccess * [dwNumReturned];
            if (NULL == apEnumAccess)
            {
                hr = E_OUTOFMEMORY;
                goto CLEANUP;
            }
            SecureZeroMemory(apEnumAccess,
                dwNumReturned * sizeof(IWbemObjectAccess*));
            dwNumObjects = dwNumReturned;

            if (FAILED(hr = pEnum->GetObjects(0L,
                dwNumObjects,
                apEnumAccess,
                &dwNumReturned)))
            {
                goto CLEANUP;
            }
        }
        else
        {
            if (hr == WBEM_S_NO_ERROR)
            {
                hr = WBEM_E_NOT_FOUND;
                goto CLEANUP;
            }
        }

        // First time through, get the handles.
        if (0 == x)
        {
            CIMTYPE VirtualBytesType;
            CIMTYPE ProcessHandleType;
            CIMTYPE CpuUsageHandleType;
            CIMTYPE ProcessNameHandleType;
            CIMTYPE NumberOfCPUCores;

            if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
                L"VirtualBytes",
                &VirtualBytesType,
                &lVirtualBytesHandle)))
            {
                goto CLEANUP;
            }
            if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
                L"IDProcess",
                &ProcessHandleType,
                &lIDProcessHandle)))
            {
                goto CLEANUP;
            }
            if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
                L"PercentProcessorTime",
                &CpuUsageHandleType,
                &lPercentProcessorTime)))
            {
                goto CLEANUP;
            }
            if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
                L"ThreadCount",
                &NumberOfCPUCores,
                &lThreadCount)))
            {
                goto CLEANUP;
            }
        }

        for (i = 0; i < dwNumReturned; i++)
        {
            apEnumAccess[i]->ReadDWORD(lVirtualBytesHandle, &dwVirtualBytes);
            apEnumAccess[i]->ReadDWORD(lIDProcessHandle, &dwIDProcess);
            apEnumAccess[i]->ReadDWORD(lPercentProcessorTime, &dwCPU_TIME);
            apEnumAccess[i]->ReadDWORD(lThreadCount, &dwCPU_THREADS_NUMBER);

            wprintf(L"El proceso %lu usa %lu bytes de memoria y %lu cpu en %lu nucleos\n", dwIDProcess, dwVirtualBytes, dwCPU_TIME, dwCPU_THREADS_NUMBER);
            apEnumAccess[i]->Release();
            //Sleep(3000);
            ///delete apEnumAccess[i];
            apEnumAccess[i] = NULL;
        }

        if (NULL != apEnumAccess)
        {
            delete[] apEnumAccess;
            apEnumAccess = NULL;
        }
    }
CLEANUP:

    if (NULL != bstrNameSpace)
    {
        SysFreeString(bstrNameSpace);
    }

    if (NULL != apEnumAccess)
    {
        for (i = 0; i < dwNumReturned; i++)
        {
            if (apEnumAccess[i] != NULL)
            {
                apEnumAccess[i]->Release();
                apEnumAccess[i] = NULL;
            }
        }
        delete[] apEnumAccess;
    }
    if (NULL != pWbemLocator)
    {
        pWbemLocator->Release();
    }
    if (NULL != pNameSpace)
    {
        pNameSpace->Release();
    }
    if (NULL != pEnum)
    {
        pEnum->Release();
    }
    if (NULL != pConfig)
    {
        pConfig->Release();
    }
    if (NULL != pRefresher)
    {
        pRefresher->Release();
    }

    CoUninitialize();

    if (FAILED(hr))
    {
        wprintf(L"Error status=%08x\n", hr);
    }

    return 0;
}