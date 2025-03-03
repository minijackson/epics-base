/*************************************************************************\
* Copyright (c) 2009 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

#include "epicsFindSymbol.h"
#include "iocsh.h"
#include "epicsExport.h"

IOCSH_STATIC_FUNC int dlload(const char* name)
{
    if (!epicsLoadLibrary(name)) {
        printf("epicsLoadLibrary failed: %s\n", epicsLoadError());
        return -1;
    }
    return 0;
}

static const iocshArg dlloadArg0 = { "path/library.so", iocshArgStringPath};
static const iocshArg * const dlloadArgs[] = {&dlloadArg0};
static const iocshFuncDef dlloadFuncDef = {
    "dlload",
    1,
    dlloadArgs,
    "Load the given shared library.\n\n"
    "Example: dlload myLibrary.so\n",
};
static void dlloadCallFunc(const iocshArgBuf *args)
{
    iocshSetError(dlload(args[0].sval));
}

static void dlloadRegistar(void) {
    iocshRegister(&dlloadFuncDef, dlloadCallFunc);
}
epicsExportRegistrar(dlloadRegistar);
