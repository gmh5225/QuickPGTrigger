#include "trigger.h"
#include <intrin.h>

//////////////////////////////////////////////////////////////////////////////
//// struct
#include <pshpack1.h>
struct Idtr {
  unsigned short limit;
  ULONG_PTR base;
};
#include <poppack.h>

struct IDTENTRY64 {
  USHORT OffsetLow;
  USHORT Selector;
  union {
    USHORT Access;
    struct {
      USHORT IstIndex : 3;
      USHORT Reserved0 : 5;
      USHORT Type : 5;
      USHORT Dpl : 2;
      USHORT Present : 1;
    } AccessField;
  };
  USHORT OffsetMiddle;
  ULONG OffsetHigh;
  ULONG Reserved1;
};

//////////////////////////////////////////////////////////////////////////////
//// function
void TriggerByPatchIDT1() {
  Idtr idtr = {};
  __sidt(&idtr);
  auto entries = reinterpret_cast<IDTENTRY64 *>(idtr.base);
  auto high = static_cast<ULONG_PTR>(entries[1].OffsetHigh) << 32;
  auto middle = static_cast<ULONG_PTR>(entries[1].OffsetMiddle) << 16;
  auto low = static_cast<ULONG_PTR>(entries[1].OffsetLow);
  auto int1Handler = reinterpret_cast<PUCHAR>(high | middle | low);
  dprintf("int1Handler=%p\n", int1Handler);

  PUCHAR patchAddr = nullptr;
  for (int i = 1; i < 0x100; ++i) {
    if (int1Handler[i] == 0x01 && int1Handler[i + 1] == 0x75) {
      patchAddr = &int1Handler[i + 1];
      break;
    }
  }
  dprintf("patchAddr=%p\n", patchAddr);
  if (!patchAddr) {
    return;
  }

  // patch
  {
    auto irql = KeGetCurrentIrql();
    __writecr8(2);
    auto cr0 = __readcr0();
    __writecr0(cr0 & 0xFFFFFFFFFFFEFFFF);
    _disable();
    *patchAddr = 0x74; //->>>>>>>>>>0x75->0x74
    cr0 = __readcr0();
    _enable();
    __writecr0(cr0 | 0x10000);
    __writecr8(irql);
  }

  // trigger testing
  {
    auto processorsCount = KeQueryActiveProcessorCount(0);
    for (ULONG i = 0; i < processorsCount; ++i) {
      KeSetSystemAffinityThread(1 << i);

      Idtr idtr = {};
      __sidt(&idtr);

      auto irql = KeGetCurrentIrql();
      __writecr8(2);
      auto cr0 = __readcr0();
      __writecr0(cr0 & 0xFFFFFFFFFFFEFFFF);
      _disable();
      cr0 = __readcr0();
      _enable();
      __writecr0(cr0 | 0x10000);
      __writecr8(irql);

      KeRevertToUserAffinityThread();
    }
  }

  dprintf("TriggerByHookIDT1 finished!\n");
}
