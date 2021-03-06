/*++

Copyright (c) 2015, Pankaj Garg <pankaj@intellectualheaven.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--*/

#ifndef _CDCORE_H_
#define _CDCORE_H_

//System-Includes!!!
#include <dbghelp.h>
#include <vector>
#include <string>
#include <map>
#include <stack>

// Local-Includes!!!
#include "cdui.h"


//
// struct to store information about the process to initialize symbols DLL
//
typedef struct _CD_SYMBOL_PROCESS_INFO
{
    DWORD   ProcessId;
    HANDLE  ProcessHandle;
    BOOL    SymbolInitialized;

}CD_SYMBOL_PROCESS_INFO, *PCD_SYMBOL_PROCESS_INFO;


//
// Class to do the core work for recovering a process from the crash
//
class CRecoverCrash
{
public:

    CRecoverCrash();
    ~CRecoverCrash();

    void AttachToProcess(
            DWORD               processId,
            HANDLE              hEvent,
            CRecoveryHandler    *pRecoveryHandler);

private:

    void HandleCreateProcess(
        DEBUG_EVENT             &inDebugEvent,
        CD_SYMBOL_PROCESS_INFO  &symProcInfo);

    void HandleCreateThread(
        DEBUG_EVENT &inDebugEvent);

    void HandleExitProcess(
        DEBUG_EVENT &inDebugEvent);

    void HandleExitThread(
        DEBUG_EVENT &inDebugEvent);

    void HandleOutputDebugString(
        DEBUG_EVENT &inDebugEvent);

    void HandleLoadDll(
        DEBUG_EVENT &inDebugEvent);

    void HandleCreateFileHooking(
        LPBYTE inImageAddress);

    bool InsertBreakPoints(
        PBYTE       inImageBase);

    void InsertBreakPoint(
        LPVOID      inAddress,
        std::string inFunctionName);

    void ReInsertBreakPoint(
        LPVOID  inAddress);

    bool HookCreateFileIATEntries(
        PBYTE       inImageBase);

    LPVOID DebuggeeMemAlloc(
        DWORD   inSize);

    bool DebuggeeReadMemory(
        LPCVOID inAddress,
        LPVOID  ioBuffer,
        SIZE_T  inSize,
        SIZE_T  *oBytesRead = NULL);

    bool DebuggeeReadMemoryGreedy(
        LPCVOID inAddress,
        LPVOID  ioBuffer,
        SIZE_T  inSize,
        SIZE_T  *oBytesRead = NULL);

    bool DebuggeeWriteMemory(
        LPVOID  inAddress,
        LPVOID  inBuffer,
        SIZE_T  inSize,
        SIZE_T  *oBytesWrote = NULL);

    bool ReadDllImageName(
        PBYTE       inImageBase,
        LPSTR       oImageName,
        DWORD       inImageNameLength);

    bool HandleException(
        DEBUG_EVENT &inDebugEvent);

    bool HandleBreakPoint(
        DEBUG_EVENT &inDebugEvent);

    bool HandleSingleStep(
        DEBUG_EVENT &inDebugEvent);

    bool HandleFatalException(
        DEBUG_EVENT &inDebugEvent);

    bool RecoverSkipInstruction(
            CONTEXT *Context);

    bool RecoverSkipFunction(
        HANDLE ThreadHandle,
        CONTEXT *Context);

    bool RecoverSkipFunctionHomebrew(
        CONTEXT *Context);

    bool Recover(
            DWORD   inThreadId);

    void InitializeSymbolHelper(
        CD_SYMBOL_PROCESS_INFO &ioSymProcInfo);

    void DeinitializeSymbolHelper(
        CD_SYMBOL_PROCESS_INFO &ioSymProcInfo);

    void HandleCreateFileW(
        CONTEXT &inContext);

    void HandleCreateFileA(
        CONTEXT &inContext);

    void HandleCreateFileCommon(
        CONTEXT &inContext,
        bool    inUnicodeFileName);

    void __cdecl HandleError(
        DWORD   inErrorCode,
        LPCTSTR inErrorMsg,
        ...);


    // process and thread informaiton for the crashing process. This info is
    // used to identify the crashing process during debugging events. Right
    // now we don't hadnle the child processes
    typedef struct _CD_THREAD_INFO
    {
    	HANDLE		hThread;
    	DWORD		threadId;

    }CD_THREAD_INFO;

    typedef std::vector<CD_THREAD_INFO>		THREAD_LIST;
    typedef THREAD_LIST::iterator			THREAD_LIST_ITER;

    typedef struct _CD_PROCESS_INFO
    {
    	bool			notInitialized;
    	HANDLE			hProcess;
    	DWORD			processId;
    	THREAD_LIST		threadList;
        BOOL            isProcess32bit;

    }CD_PROCESS_INFO;

	// Data structures to store information about the breakpoints that we
	// insert in functions like CreateFile
	typedef struct _BP_INFO
	{
		BYTE		OriginalByte;
		std::string	FunctionName;

	}BP_INFO;

	typedef std::map<PVOID, BP_INFO>	ADDR_TO_BP_MAP;
	typedef std::stack<PVOID>			ADDR_STACK;
	typedef std::map<DWORD, ADDR_STACK>	THREAD_TO_ADDR_STACK_MAP;

	// Data structure to manage functions for which breakpoint needs to be
	// inserted
	class CRecoverCrash;
	typedef void (CRecoverCrash::*PFN_FunctionHandler)(CONTEXT &inContext);

	typedef std::map<std::string, PFN_FunctionHandler>
												FUNCTION_NAME_TO_HANDLER_MAP;

	// Data structure for inserting a synthetic IAT patch in debuggee
	// It will be used for windows 9x OS to patch IAT
	#pragma pack(push)
	#pragma pack(1)
	typedef struct _PATCH_CODE
	{
		BYTE		JMP[2];
		DWORD_PTR   pdwAddress;
		DWORD_PTR   dwAddress;

	}PATCH_CODE;
	#pragma pack(pop)


	// variable to manage functions information for patched functions
	FUNCTION_NAME_TO_HANDLER_MAP	mFunctionNameToHandlerMap;

	// Variables to store breakpoints related information
	ADDR_TO_BP_MAP				mAddrToBpInfoMap;
	THREAD_TO_ADDR_STACK_MAP	mThreadToAddrStackMap;

    // Variable to store information about the process that
    // we are debugging
    CD_PROCESS_INFO     mCrashedProcessInfo;

    // variable for deciding the lifetime of debug loop
    bool                mKeepAlive;

    // variable to count the number of crashes
    DWORD               mCountCrash;

    // Did symbol initialized for this process or not?
    // if symbols are initialized then we will use symbol library stack walking
    // functions. Otherwise we will use the in-house stack generation method;
    // We always should try to use the symbol library stack generation because
    // it is way more thorough than our approach.
    BOOL                mSymbolInitialized;

    // Array for printing both ascii and unicode strings. Because maximum
    // data sent to debugger in one call is 512 characters, a 1026 bytes char
    // array can be used safely for both ascii and unicode debug output
    char                mDbgString[1026];

	// CRecoveryHandler pointer is used for reporting any error or other
	// information to the user
	CRecoveryHandler	*mRecoveryHandler;

	// variable to see if we successfully recovered the crash or not
	bool				mCrashRecovered;
};

#endif