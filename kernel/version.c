#include <windows.h>
#include <win_private.h>

extern WORD pascal wVersion;

DWORD WINAPI GetVersion( void )
{
  WORD HostVersion;
  SaveDS();
  SetKernelDS();
  HostVersion=wVersion;
  RestoreDS();
// Win    Host
  return 0x0003<<16+HostVersion;
// AX     DX
}

