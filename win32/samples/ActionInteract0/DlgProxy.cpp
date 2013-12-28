
// DlgProxy.cpp : implementation file
//

#include "stdafx.h"
#include "ActionInteract0.h"
#include "DlgProxy.h"
#include "ActionInteract0Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CActionInteract0DlgAutoProxy

IMPLEMENT_DYNCREATE(CActionInteract0DlgAutoProxy, CCmdTarget)

CActionInteract0DlgAutoProxy::CActionInteract0DlgAutoProxy()
{
	EnableAutomation();
	
	// To keep the application running as long as an automation 
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CActionInteract0Dlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CActionInteract0Dlg)))
		{
			m_pDialog = reinterpret_cast<CActionInteract0Dlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CActionInteract0DlgAutoProxy::~CActionInteract0DlgAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != NULL)
		m_pDialog->m_pAutoProxy = NULL;
	AfxOleUnlockApp();
}

void CActionInteract0DlgAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CActionInteract0DlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CActionInteract0DlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// Note: we add support for IID_IActionInteract0 to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {99B35469-4B32-4F5B-BE3C-7AAC96E3BA78}
static const IID IID_IActionInteract0 =
{ 0x99B35469, 0x4B32, 0x4F5B, { 0xBE, 0x3C, 0x7A, 0xAC, 0x96, 0xE3, 0xBA, 0x78 } };

BEGIN_INTERFACE_MAP(CActionInteract0DlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CActionInteract0DlgAutoProxy, IID_IActionInteract0, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in StdAfx.h of this project
// {F7A5A312-23EB-43BF-AE7B-B8858D8ED07B}
IMPLEMENT_OLECREATE2(CActionInteract0DlgAutoProxy, "ActionInteract0.Application", 0xf7a5a312, 0x23eb, 0x43bf, 0xae, 0x7b, 0xb8, 0x85, 0x8d, 0x8e, 0xd0, 0x7b)


// CActionInteract0DlgAutoProxy message handlers
