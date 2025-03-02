#include "Memory/PageTable.h"

void InitEmptyPageTable(PageTableEntry* pageTable)
{
	for (usz i = 0; i < PAGE_TABLE_ENTRIES; i++) {
		pageTable[i] = 0;
	}
}
