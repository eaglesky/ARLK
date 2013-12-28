
// ActionInteract0.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "MainModel.h"  

// CActionInteract0App:
// See ActionInteract0.cpp for the implementation of this class
//

class CActionInteract0App : public CWinApp
{


public:
	CActionInteract0App();

// Overrides   
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	MainModel *m_pMainModel;
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CActionInteract0App theApp;