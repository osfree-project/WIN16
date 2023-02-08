/*************************************************************************
*
* TWINHELP.H
* General defines for entire project.     
*
* %W% %G% %U% %P%
*
* Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
**************************************************************************/

#ifndef TWINHELP_H
#define TWINHELP_H


/***********************************
**
**  System Includes
**
***********************************/
#include <stdlib.h>


/*************************************
*
*  Shared Defines
*
**************************************/

/* Private messages. */
#define HELPWM_CLOSE            (WM_USER + 1)         
#define PRIVMSG_SHOWNEWTOPIC    (WM_USER + 2)
#define PRIVMSG_SHOWHISTORY     (WM_USER + 3)
#define PRIVMSG_ADDHISTTOPIC    (WM_USER + 4)
#define PRIVMSG_UPDATEHISTPOS   (WM_USER + 5)
#define PRIVMSG_CLEARHISTPOS    (WM_USER + 6)
#define PRIVMSG_RESIZE          (WM_USER + 7)
#define PRIVMSG_USEBGCOLOR      (WM_USER + 8)
#define PRIVMSG_HELP_CONTENTS   (WM_USER + 9)

/* Force exiting of app. on WM_CLOSE message. */
#define EXIT_APP  1

/* 
** When telling secondary windows to close, this message
** is you to tell them not to report that they are closing.
*/
#define DONT_REPORT_CLOSING 1

/* 
** Structure passed in the PRIVMSG_SHOWNEWTOPIC message's
** lParam parameter.
*/
typedef struct tagNEWTOPICSTRUCT
{
  HGLOBAL hNewTopicsData;     /* Handle to new topic's data. */
  BOOL    bUpdateBackList;    /* Update the "Back" list with the topic data. */
  BOOL    bFromHistoryBack;   /* Is this a topic from the back list or history list. */
  WORD    wVertPos;           /* Initial vertical scroll position. */
  COLORREF ScrollColor;       /* Background color to use for scrolling secondary window. */
  COLORREF NonScrollColor;    /* Background color to use for non-scrolling secondary window. */
} 
NEWTOPICSTRUCT;
typedef NEWTOPICSTRUCT __far * FPNEWTOPICSTRUCT;


/*
**
** Message atoms strings.
**
*/
/* String used when registering the WM_WINDOC message. */
#define MSWIN_DOC_MSG_STRING  "WM_WINDOC"


/* 
** Window class names for a visible topic window and a visible 
** popup window. 
*/
#define MAINSEC_CLASS      "WILL_HELP_MAINSEC"
#define TOPICSEC_CLASS     "WILL_HELP_TOPICSEC"
#define JUMPSCROLL_CLASS   "WILL_HELP_JUMPSCROLL"
#define JUMPNOSCROLL_CLASS "WILL_HELP_JUMPNOSCROLL"
#define POPUP_CLASS        "WILL_HELP_POPUP"

/* System dependent defines. */
#define DIR_SEP_CHAR  '\\'
#define DIR_SEP_STR   "\\"
#define EXT_CHAR '.'           


/* Button bar button values. */
#define CONTENTS_BUTTON_ID  "btn_contents"
#define SEARCH_BUTTON_ID    "btn_search"
#define BACK_BUTTON_ID      "btn_back"
#define HISTORY_BUTTON_ID   "btn_history"
#define PREV_BUTTON_ID      "btn_previous"
#define NEXT_BUTTON_ID      "btn_next"


/* Default menu values. */
#define FILE_MENU_ID              "mnu_file"
#define FILE_OPEN_MENU_ID         "mnu_file_open"
#define FILE_PRINTTOPIC_MENU_ID   "mnu_file_print"
#define FILE_PRINTSETUP_MENU_ID   "mnu_file_printsetup"
#define FILE_EXIT_MENU_ID         "mnu_file_exit"

#define EDIT_MENU_ID              "mnu_edit"
#define EDIT_COPY_MENU_ID         "mnu_edit_copy"
#define EDIT_ANNOTATE_MENU_ID     "mnu_edit_annotate"

#define BOOKMARK_MENU_ID          "mnu_bookmark"
#define BOOKMARK_DEFINE_MENU_ID   "mnu_bookmark_define"
                            
#define HELP_MENU_ID              "mnu_help"
#define HELP_HELPONHELP_MENU_ID   "mnu_help_helponhelp"
#define HELP_ALWAYSONTOP_MENU_ID  "mnu_help_alwaysontop"
#define HELP_ABOUT_MENU_ID        "mnu_help_about"

/* Name of the default "Help on Help" help file's name. */
#define DEFAULT_HELPONHELP_FILENAME  "twinhelp.hlp"


/* Page position info. used in various places. */
typedef struct tagPOSRECT 
{    
  long int left;
  long int top;
  long int right;
  long int bottom;
} 
POSRECT;
typedef POSRECT __far * FPPOSRECT;

/* Page position info. used in various places. */
typedef struct tagPOSPOINT 
{    
  long int x;
  long int y;
} 
POSPOINT;
typedef POSPOINT __far * FPPOSPOINT;

/* Name of main icon in resource file. */
#define MAIN_ICON "TWINHELP"

/* Secondary window strings max. sizes. */
#define MAX_SEC_TYPE     10
#define MAX_SEC_NAME      9
#define MAX_SEC_CAPTION  51

/* A path to a help file can contain a path, '>', and the name. */
#define MAX_HLPPATH  ( _MAX_PATH + sizeof(char) + MAX_SEC_NAME )
            
/* 
** Separator token used to specify, in a filepath or after a topic, a target window
** to show the topic in.
*/
#define WINDOW_SEP_TOKEN_STR  ">"
#define WINDOW_SEP_TOKEN_CHAR '>'

/* Maximum length of a help class name. */
#define MAX_CLASS_NAME   20                   


/*************************************
*
*  Shared Functions
*
**************************************/

long __far __pascal __export MainWndProc( HWND hWnd, UINT Msg, WORD wParam, LONG lParam );
long __far __pascal __export TopicWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
long __far __pascal __export NoScrollTopicWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
long __far __pascal __export PopupWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
long __far __pascal __export MainSecWndProc ( HWND hWnd, UINT Msg, WORD wParam, LONG lParam);
long __far __pascal __export TopicSecWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
void __far __pascal SendShowNewTopicMsg
( 
  HWND    hTopicWnd, 
  HGLOBAL hNewTopicsData,
  BOOL    bUpdateBackList,
  BOOL    bFromHistoryBack,
  WORD    wVertPos
);

BOOL __far __pascal ShowBackTopic( HWND hDataWnd );

void __far __pascal SetBrowseButtonsTopics( HWND hDataWnd );

BOOL __far __pascal DoHelpContentsButton( HWND hDataWnd );

void __far __pascal ShowHistory( HWND hDataWnd ); 

BOOL __far __pascal DoPrevButton( HWND hDataWnd );
BOOL __far __pascal DoNextButton( HWND hDataWnd );

BOOL __far __pascal DoHelpContentsMsg( HWND hDataWnd, char __far * HelpFilePathPtr );
BOOL __far __pascal DoHelpSetContentsMsg( HWND hDataWnd, char __far * ContentsHelpFilePath, long int nContentsContext );

void __far __pascal OpenFileMenuItem( HWND hWnd );

BOOL __far __pascal DoHelpOnHelpMsg( HWND hWnd );

void __far __pascal ShowAboutDialogBox( HWND hParentWnd, HWND hMainWnd );

BOOL __far __pascal DoHelpContextMsg
( 
  HWND hDataWnd,
  char __far * szHelpFilePath, 
  unsigned long int nContext
);
   
BOOL __far __pascal JumpToContextHashTopic
( 
  HWND hDataWnd,
  char __far * szHelpFilePath, 
  DWORD dwHashValue
);
   
BOOL __far __pascal JumpKeyword
( 
  HWND hDataWnd, 
  char __far * szHelpFilePath, 
  char __far * szKeyword
);

BOOL __far __pascal DoHelpContextPopupMsg
( 
  HWND hDataWnd,
  HWND hParentWnd,
  char __far * szHelpFilePath, 
  unsigned long int nMapValue
);

BOOL __far __pascal PopupContextHashTopic
( 
  HWND hDataWnd,
  HWND hParentWnd,
  char __far * szHelpFilePath, 
  DWORD dwHashValue
);


#endif
                         
