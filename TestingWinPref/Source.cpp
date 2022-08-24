#pragma once
#define _WIN32_DCOM

//Win32_Processor

#include <iostream>
#include <Wbemidl.h>
#include <cassert>
#include <vector>
#pragma comment(lib, "wbemuuid.lib")

int __cdecl wmain(int argc, wchar_t* argv[])
{
    //<GENERAL PROCESES CECL>

    IWbemRefresher* pRefresher = NULL; //refresher
    IWbemConfigureRefresher* pConfig = NULL; //setter for refresher's env 
    IWbemHiPerfEnum* pEnum = NULL;
    IWbemServices* pNameSpace = NULL;
    IWbemLocator* pWbemLocator = NULL;
    IWbemObjectAccess** apEnumAccess = NULL;
    BSTR bstrNameSpace = NULL;
    DWORD dwNumObjects = 0;
    DWORD  dwNumReturned = 0;
    DWORD i = 0;
    int x = 0;
    long lID = 0;

    //<MEMORY INFO DECL>

    long                    lVirtualBytesHandle = 0;
    DWORD                   dwVirtualBytes = 0;

    //<PROCESSES ID DECL>

    DWORD                   dwProcessId = 0;
    DWORD                   dwIDProcess = 0;
    long                    lIDProcessHandle = 0;

    //<PERCENT PROCESSOR DECL>

    BSTR* NameKW = new BSTR();
    HRESULT                 hr = S_OK;
    long                    lPercentProcessorTime = 0;

    //<PROCESSOR INFO decl>
    long                    lThreadCount = 0;
    long                    lCoresCounter = 0;
    DWORD                   dwCPU_TIME = 0;
    DWORD                   dwCPU_THREADS_NUMBER = 0;

    //<PROCESS NAME DECL>

    BSTR* bstrProcessName = new BSTR();
    long lProcessNameHandler = 0;
    VARIANT varProcessNameResult;


    //<PREPARING ENV>

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&pWbemLocator);
    bstrNameSpace = SysAllocString(L"\\\\.\\root\\cimv2"); // Will be possible to implement a networking interface
    pWbemLocator->ConnectServer(bstrNameSpace,
        NULL, // User name
        NULL, // Password
        NULL, // Locale
        0L,   // Security flags
        NULL, // Authority
        NULL, // Wbem context
        &pNameSpace); //------>Server provider or host

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
        hr = pEnum->GetObjects(0L, dwNumObjects, apEnumAccess, &dwNumReturned);
        if (hr == WBEM_E_BUFFER_TOO_SMALL
            && dwNumReturned > dwNumObjects)
        {
            apEnumAccess = new IWbemObjectAccess * [dwNumReturned];
            if (NULL == apEnumAccess)
            {
                hr = E_OUTOFMEMORY;
            }
            SecureZeroMemory(apEnumAccess, dwNumReturned * sizeof(IWbemObjectAccess*));
            dwNumObjects = dwNumReturned;
            hr = pEnum->GetObjects(0L, dwNumObjects, apEnumAccess, &dwNumReturned);
        }

        // First time through, get the handles.
        if (0 == x)
        {
            CIMTYPE VirtualBytesType;
            CIMTYPE ProcessHandleType;
            CIMTYPE CpuUsageHandleType;
            CIMTYPE NumberOfCPUCores;
            apEnumAccess[0]->GetPropertyHandle(L"VirtualBytes", &VirtualBytesType, &lVirtualBytesHandle);
            apEnumAccess[0]->GetPropertyHandle(L"IDProcess", &ProcessHandleType, &lIDProcessHandle);
            apEnumAccess[0]->GetPropertyHandle(L"PercentProcessorTime", &CpuUsageHandleType, &lPercentProcessorTime);
        }

        for (i = 0; i < dwNumReturned; i++)
        {
            apEnumAccess[i]->Get(L"Name", 0, &varProcessNameResult, NULL, NULL);
            apEnumAccess[i]->ReadDWORD(lVirtualBytesHandle, &dwVirtualBytes);
            apEnumAccess[i]->ReadDWORD(lIDProcessHandle, &dwIDProcess);
            apEnumAccess[i]->ReadDWORD(lPercentProcessorTime, &dwCPU_TIME);
            apEnumAccess[i]->ReadDWORD(lThreadCount, &dwCPU_THREADS_NUMBER);

            //<ASSIGN REAL VALUES>

            bstrProcessName = &varProcessNameResult.bstrVal;
            assert(NameKW != nullptr);
            std::wstring ws(*bstrProcessName, SysStringLen(*bstrProcessName));

            dwVirtualBytes = dwVirtualBytes / 1048576;

            wprintf(L"El proceso %ls PID: % lu usa % lu MB de memoria y % lu cpu en % lu hilos\n", &ws[0], dwIDProcess, dwVirtualBytes, dwCPU_TIME, dwCPU_THREADS_NUMBER);
            apEnumAccess[i]->Release();
            apEnumAccess[i] = NULL;
        }

        delete[] apEnumAccess;
        apEnumAccess = NULL;

    }

    return 0;
}