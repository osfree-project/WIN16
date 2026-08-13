/* Replace "dll.h" with the name of your header */
#include "dll.h"
#include <windows.h>
#include <stdio.h>


int 
DllClassBase::virtual_method () const
{ 
    return -1; 
}


DllClass::DllClass (int i) : i_(i)
{
    ++instances;
}


DllClass::~DllClass ()
{
    --instances;
}


int 
DllClass::virtual_method () const 
{
    return i_ * i_ * i_; 
}


int 
DllClass::non_virtual_method () const 
{
    return i_ * i_;
}


int DllClass::instances;



BOOL APIENTRY
DllMain (
  HINSTANCE hInst     /* Library instance handle. */ ,
  DWORD reason        /* Reason this function is being called. */ ,
  LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        break;

      case DLL_PROCESS_DETACH:
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
