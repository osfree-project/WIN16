/*************************************************************************
 * ConfirmDialog				[internal]
 *
 * Put up a confirm box, return TRUE if the user confirmed
 */
static BOOL ConfirmDialog(HWND hWndOwner, UINT PromptId, UINT TitleId)
{
  WCHAR Prompt[256];
  WCHAR Title[256];

  LoadStringW(shell32_hInstance, PromptId, Prompt, ARRAY_SIZE(Prompt));
  LoadStringW(shell32_hInstance, TitleId, Title, ARRAY_SIZE(Title));
  return MessageBoxW(hWndOwner, Prompt, Title, MB_YESNO|MB_ICONQUESTION) == IDYES;
}


/*************************************************************************
 * RestartDialogEx				[SHELL32.730]
 */

int WINAPI RestartDialogEx(HWND hWndOwner, LPCWSTR lpwstrReason, DWORD uFlags, DWORD uReason)
{
    TRACE("(%p)\n", hWndOwner);

    /* FIXME: use lpwstrReason */
    if (ConfirmDialog(hWndOwner, IDS_RESTART_PROMPT, IDS_RESTART_TITLE))
    {
        HANDLE hToken;
        TOKEN_PRIVILEGES npr;

        /* enable the shutdown privilege for the current process */
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        {
            LookupPrivilegeValueA(0, "SeShutdownPrivilege", &npr.Privileges[0].Luid);
            npr.PrivilegeCount = 1;
            npr.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(hToken, FALSE, &npr, 0, 0, 0);
            CloseHandle(hToken);
        }
        ExitWindowsEx(EWX_REBOOT, uReason);
    }

    return 0;
}


/*************************************************************************
 * RestartDialog				[SHELL32.59]
 */

int WINAPI RestartDialog(HWND hWndOwner, LPCWSTR lpstrReason, DWORD uFlags)
{
    return RestartDialogEx(hWndOwner, lpstrReason, uFlags, 0);
}
