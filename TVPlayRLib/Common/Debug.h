#pragma once
#ifdef DEBUG
#define DebugPrint(s) OutputDebugStringA(s)
#else
#define DebugPrint(s)
#endif

#ifdef DEBUG
#define DebugPrintIf(c, s) \
if (c) \
	OutputDebugStringA(s)
#else
#define DebugPrintIf(c, s)
#endif
