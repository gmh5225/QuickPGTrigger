#include "trigger.h"

VOID DrvUnload(PDRIVER_OBJECT DriverObject) {}

EXTERN_C
NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
  DriverObject->DriverUnload = DrvUnload;

  TriggerByHookIDT1();

  return STATUS_VIRUS_INFECTED;
}
