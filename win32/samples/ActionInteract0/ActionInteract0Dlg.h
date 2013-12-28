
// ActionInteract0Dlg.h : header file
//

#pragma once    
#include "ActionInteract0.h"
#include "ParametersDlg.h"
class CActionInteract0DlgAutoProxy;


// CActionInteract0Dlg dialog
class CActionInteract0Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CActionInteract0Dlg);
	friend class CActionInteract0DlgAutoProxy;

// Construction
public:
	CActionInteract0Dlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CActionInteract0Dlg();

// Dialog Data
	enum { IDD = IDD_ACTIONINTERACT0_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CActionInteract0DlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedTrainData();
	afx_msg void OnBnClickedTrain();

private:
	MainModel *m_pMainModel;
public:
	afx_msg void OnBnClickedTestData();
	afx_msg void OnBnClickedPredict();
	afx_msg void OnSettingsSetparameters();
	afx_msg void OnAboutVersion();
};
