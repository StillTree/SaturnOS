#pragma once

#include "Core.h"

typedef struct KernelParams {
	const i8* InitProcess;
	bool ASLR;
} KernelParams;

void ParseKernelParams();

extern KernelParams g_parameters;
