#pragma once
#include <fltKernel.h>
/////////////////////////////////////////////////////
//// macro
#define dprintf(...)                                                           \
  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

/////////////////////////////////////////////////////
//// declare function
void TriggerByPatchIDT1();
