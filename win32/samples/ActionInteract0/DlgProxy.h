
// DlgProxy.h: header file
//

#pragma once

class CActionInteract0Dlg;


// CActionInteract0DlgAutoProxy command target

class CActionInteract0DlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CActionInteract0DlgAutoProxy)

	CActionInteract0DlgAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	CActionInteract0Dlg* m_pDialog;

// Operations
public:

// Overrides
	public:
	virtual void OnFinalRelease();

// Implementation
protected:
	virtual ~CActionInteract0DlgAutoProxy();

	// Generated message map functions

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CActionInteract0DlgAutoProxy)

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

