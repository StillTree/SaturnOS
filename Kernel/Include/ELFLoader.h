#pragma once

#include "Core.h"
#include "Result.h"
#include "Scheduler.h"

/// This function should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ProcessLoadELF(Process* process, const i8* elfPath);
