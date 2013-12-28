// ParametersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ActionInteract0.h"
#include "ParametersDlg.h"
#include "afxdialogex.h"


// CParametersDlg dialog

IMPLEMENT_DYNAMIC(CParametersDlg, CDialogEx)

CParametersDlg::CParametersDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CParametersDlg::IDD, pParent)
{

}

CParametersDlg::~CParametersDlg()
{
}

void CParametersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CParametersDlg, CDialogEx)
END_MESSAGE_MAP()


// CParametersDlg message handlers


BOOL CParametersDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CFont *titleFont = new CFont;
	titleFont->CreatePointFont(160, "Arial");
	CStatic* titleStaticText = (CStatic *)GetDlgItem(IDC_SVM_PARAM);
	titleStaticText->SetFont(titleFont);

	titleStaticText = (CStatic *)GetDlgItem(IDC_KNN_PARAM);
	titleStaticText->SetFont(titleFont);

	delete titleFont;

	m_pMainModel = ((CActionInteract0App *)AfxGetApp())->m_pMainModel;

	
	double svm_C, svm_gamma, featureWt, knn_eps;
	int knn_K;
	CString svm_CStr, svm_gammaStr, featureWtStr, knn_KStr, knn_epsStr;
	m_pMainModel->GetSVMParams(&svm_C, &svm_gamma, &featureWt);
	m_pMainModel->GetKNNParams(&knn_K, &knn_eps, &featureWt);

	svm_CStr.Format(_T("%.8g"), svm_C);
	svm_gammaStr.Format(_T("%.8g"), svm_gamma);
	featureWtStr.Format(_T("%.8g"), featureWt);
	knn_KStr.Format(_T("%d"), knn_K);
	knn_epsStr.Format(_T("%.8g"), knn_eps);
	
	/*svm_CStr.TrimRight('0');
	svm_gammaStr.TrimRight('0');
	featureWtStr.TrimRight('0');*/

	CEdit* editControl = (CEdit *)GetDlgItem(IDC_SVM_C_VAL);
	editControl->SetWindowText(svm_CStr);

	editControl = (CEdit *)GetDlgItem(IDC_SVM_GAMMA_VAL);
	editControl->SetWindowText(svm_gammaStr);

	editControl = (CEdit *)GetDlgItem(IDC_WT_VAL);
	editControl->SetWindowText(featureWtStr);

	editControl = (CEdit *)GetDlgItem(IDC_KNN_K_VAL);
	editControl->SetWindowText(knn_KStr);

	editControl = (CEdit *)GetDlgItem(IDC_KNN_EPS_VAL);
	editControl->SetWindowText(knn_epsStr);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CParametersDlg::OnOK()
{
	double svm_C, svm_gamma, featureWt, knn_eps;
	int knn_K;
	CString svm_CStr, svm_gammaStr, featureWtStr, knn_KStr, knn_epsStr;

	CEdit* editControl = (CEdit *)GetDlgItem(IDC_SVM_C_VAL);
	editControl->GetWindowText(svm_CStr);
	svm_C = atof(svm_CStr);

	editControl = (CEdit *)GetDlgItem(IDC_SVM_GAMMA_VAL);
	editControl->GetWindowText(svm_gammaStr);
	svm_gamma = atof(svm_gammaStr);

	editControl = (CEdit *)GetDlgItem(IDC_WT_VAL);
	editControl->GetWindowText(featureWtStr);
	featureWt = atof(featureWtStr);

	m_pMainModel->SetSVMParams(svm_C, svm_gamma, featureWt);

	editControl = (CEdit *)GetDlgItem(IDC_KNN_K_VAL);
	editControl->GetWindowText(knn_KStr);
	knn_K = atof(knn_KStr);

	editControl = (CEdit *)GetDlgItem(IDC_KNN_EPS_VAL);
	editControl->GetWindowText(knn_epsStr);
	knn_eps = atof(knn_epsStr);

	m_pMainModel->SetKNNParams(knn_K, knn_eps, featureWt);

	CDialogEx::OnOK();
}
