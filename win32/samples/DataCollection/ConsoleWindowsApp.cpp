// ConsoleWinApp.cpp : implementation file
//

#include "stdafx.h"
#include <tchar.h>
#ifdef _CONSOLEWIN
//Set subsystem to console
#pragma comment ( linker, "/subsystem:console" )

BOOL WINAPI ConsoleWinHandlerRoutine( DWORD dwCtrlType )
{
   // Signal type
   switch( dwCtrlType )
   {
   case CTRL_C_EVENT:
   case CTRL_BREAK_EVENT:
   case CTRL_CLOSE_EVENT:
   case CTRL_LOGOFF_EVENT:
   case CTRL_SHUTDOWN_EVENT:
      // You can stop here gracefully:
      //
      // AfxGetMainWnd()->SendMessage( WM_CLOSE, 0, 0 );
      // WaitForSingleObject( AfxGetThread(), INFINITE );
      //
      ExitProcess(0);
      break;
   }

   return TRUE;
}

// Console main function
int _tmain( DWORD, TCHAR**, TCHAR** )
{
#define SPACECHAR   _T(' ')
#define DQUOTECHAR  _T('\"')

   // Set the new handler
   SetConsoleCtrlHandler( ConsoleWinHandlerRoutine, TRUE );

   // Get command lin
	LPTSTR lpszCommandLine = ::GetCommandLine();

	if(lpszCommandLine == NULL)
		return -1;

	// Skip past program name (first token in command line).
	// Check for and handle quoted program name.
	if(*lpszCommandLine == DQUOTECHAR)
	{
		// Scan, and skip over, subsequent characters until
		// another double-quote or a null is encountered.
		do
		{
			lpszCommandLine = ::CharNext(lpszCommandLine);
		}
		while((*lpszCommandLine != DQUOTECHAR) && (*lpszCommandLine != _T('\0')));

		// If we stopped on a double-quote (usual case), skip over it.
		if(*lpszCommandLine == DQUOTECHAR)
			lpszCommandLine = ::CharNext(lpszCommandLine);
	}
	else
	{
		while(*lpszCommandLine > SPACECHAR)
			lpszCommandLine = ::CharNext(lpszCommandLine);
	}

	// Skip past any white space preceeding the second token.
	while(*lpszCommandLine && (*lpszCommandLine <= SPACECHAR))
		lpszCommandLine = ::CharNext(lpszCommandLine);

	STARTUPINFO StartupInfo;
	StartupInfo.dwFlags = 0;
	::GetStartupInfo(&StartupInfo);

	return _tWinMain(::GetModuleHandle(NULL), NULL, lpszCommandLine,
		(StartupInfo.dwFlags & STARTF_USESHOWWINDOW) ?
		StartupInfo.wShowWindow : SW_SHOWDEFAULT);
}

#endif // _CONSOLEWIN

