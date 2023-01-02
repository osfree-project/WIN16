
#include <win16.h>

extern void InitKernel();

void WINAPI kernelmain(void)
{
  InitKernel();
}
