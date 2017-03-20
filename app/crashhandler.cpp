/*
 * Fedora Media Writer
 * Copyright (C) 2017 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "crashhandler.h"

#ifdef _WIN32
// it's not possible to use QDebug here
# include <stdio.h>
# include <windows.h>
# include <dbghelp.h>

void printStack(void) {
     HANDLE process = GetCurrentProcess();
     SymInitialize( process, NULL, TRUE );

     void *stack[64];
     unsigned short frames = CaptureStackBackTrace( 0, 64, stack, NULL );

     SYMBOL_INFO *symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
     symbol->MaxNameLen = 255;
     symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

     fprintf(stderr, "Backtrace:\n");
     for(int i = 0; i < frames; i++) {
         SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
         fprintf(stderr, "\t%i: %s - 0x%p\n", frames - i - 1, symbol->Name, (void*)symbol->Address);
     }

     free(symbol);
}

LONG faultHandler(struct _EXCEPTION_POINTERS *info) {
    int code = info->ExceptionRecord->ExceptionCode;
    int flags = info->ExceptionRecord->ExceptionFlags;
    void *address = info->ExceptionRecord->ExceptionAddress;
    const char *faultName = "";
    switch(info->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:      faultName = "ACCESS VIOLATION"     ; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT: faultName = "DATATYPE MISALIGNMENT"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:    faultName = "FLT DIVIDE BY ZERO"   ; break;
        default:                              faultName = "(N/A)"                ; break;
    }

    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fprintf(stderr, "An unhandled exception occurred:\n");
    fprintf(stderr, "Code: %08X - %s\n", code, faultName);
    fprintf(stderr, "Flags: %08X\n", flags);
    fprintf(stderr, "Address: %8p\n", address);

    printStack();
    fflush(stderr);

    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashHandler::install() {
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) faultHandler);
}
#else // _WIN32

void CrashHandler::install() {
    // do nothing
}

#endif // _WIN32


