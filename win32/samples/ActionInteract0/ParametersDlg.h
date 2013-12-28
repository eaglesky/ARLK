#pragma once

#include "MainModel.h"
// CParametersDlg dialog

class CParametersDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CParametersDlg)

public:
	CParametersDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParametersDlg();

// Dialog Data
	enum { IDD = IDD_PARAMETERS };

private:
	MainModel *m_pMainModel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
