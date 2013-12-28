
// ActionInteract0Dlg.cpp : implementation file
//

#include "stdafx.h"

#include "ActionInteract0Dlg.h"   
#include "DlgProxy.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
	EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CActionInteract0Dlg dialog




IMPLEMENT_DYNAMIC(CActionInteract0Dlg, CDialogEx);

CActionInteract0Dlg::CActionInteract0Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CActionInteract0Dlg::IDD, pParent)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

CActionInteract0Dlg::~CActionInteract0Dlg()
{
	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to NULL, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;
}

void CActionInteract0Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CActionInteract0Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_TRAIN_DATA, &CActionInteract0Dlg::OnBnClickedTrainData)
	ON_BN_CLICKED(IDC_TRAIN, &CActionInteract0Dlg::OnBnClickedTrain)
	ON_BN_CLICKED(IDC_TEST_DATA, &CActionInteract0Dlg::OnBnClickedTestData)
	ON_BN_CLICKED(IDC_PREDICT, &CActionInteract0Dlg::OnBnClickedPredict)
	ON_COMMAND(ID_SETTINGS_SETPARAMETERS, &CActionInteract0Dlg::OnSettingsSetparameters)
	ON_COMMAND(ID_ABOUT_VERSION, &CActionInteract0Dlg::OnAboutVersion)
END_MESSAGE_MAP()


// CActionInteract0Dlg message handlers

BOOL CActionInteract0Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}   

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CMenu* menu =new CMenu;
	menu->LoadMenu(MAKEINTRESOURCE(IDR_MAIN_MENU));      //红色部分为对应自己添加的菜单资源的名称
	SetMenu(menu);

	m_pMainModel = ((CActionInteract0App *)AfxGetApp())->m_pMainModel;

	CComboBox *pComboSource = (CComboBox *)GetDlgItem(IDC_TRAIN_SOURCE); 
	pComboSource->AddString("From Files");
	pComboSource->AddString("Real Time");
	
	pComboSource->SetCurSel(0);

	pComboSource = (CComboBox *)GetDlgItem(IDC_PREDICT_SOURCE);
	pComboSource->AddString("From Files");
	pComboSource->AddString("Real Time");
	pComboSource->SetCurSel(0);

	CComboBox *pComboAlgo = (CComboBox *)GetDlgItem(IDC_ALGO);
	pComboAlgo->AddString("SVM");
	pComboAlgo->AddString("KNN");
	pComboAlgo->SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CActionInteract0Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CActionInteract0Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CActionInteract0Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CActionInteract0Dlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void CActionInteract0Dlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CActionInteract0Dlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CActionInteract0Dlg::CanExit()
{
	// If the proxy object is still around, then the automation
	//  controller is still holding on to this application.  Leave
	//  the dialog around, but hide its UI.
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}



void CActionInteract0Dlg::OnBnClickedTrainData()
{
	// TODO: Add your control notification handler code here

	int dataSource = ((CComboBox *)GetDlgItem(IDC_TRAIN_SOURCE))->GetCurSel();
	

	

	
	if (dataSource == FROM_FILE) {
		MYDEBUG("Train data: from file!\n");
		CListBox *pTrainListBox = (CListBox *) GetDlgItem(IDC_TRAIN_FILES);  
		
		CFileDialog fileDlg(TRUE,"skd","*.skd",OFN_FILEMUSTEXIST| OFN_ALLOWMULTISELECT, "SKD Files(*.skd)|*.skd|All Files(*.*)|*.*||", this);
		if(fileDlg.DoModal() == IDOK) {
			m_pMainModel->ClearTrainData();
			pTrainListBox->ResetContent();
			
			POSITION pos = fileDlg.GetStartPosition();
			while (pos!=NULL) {
				CString filePath = fileDlg.GetNextPathName(pos);
			
				HANDLE hSkdFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				
				m_pMainModel->AddTrainFileHandle(hSkdFile);
				pTrainListBox->AddString(filePath);
				
			}
			m_pMainModel->ImportTrainFiles();
			m_pMainModel->CloseTrainFiles();
		}
	} else {
		MYDEBUG("Train data: real time!\n");
	}
}


void CActionInteract0Dlg::OnBnClickedTrain()
{
	int algo = ((CComboBox *)GetDlgItem(IDC_ALGO))->GetCurSel();

	
	if (algo == ALGO_SVM) {
		MYDEBUG("SVM train begin!\n");
	    m_pMainModel->SVMTrain();
	 
		MYDEBUG("SVM train end!\n");
    } else if (algo == ALGO_KNN) {
		MYDEBUG("KNN train begin!\n");
		m_pMainModel->KNNTrain();
		MYDEBUG("KNN train end!\n");
	}
}


void CActionInteract0Dlg::OnBnClickedTestData()
{
	// TODO: Add your control notification handler code here
	int dataSource = ((CComboBox *)GetDlgItem(IDC_PREDICT_SOURCE))->GetCurSel();


	


	if (dataSource == FROM_FILE) {
		MYDEBUG("Test data: from file!\n");
		CListBox *pTestListBox = (CListBox *) GetDlgItem(IDC_PREDICT_FILES);  

		CFileDialog fileDlg(TRUE,"skd","*.skd",OFN_FILEMUSTEXIST| OFN_ALLOWMULTISELECT, "SKD Files(*.skd)|*.skd|All Files(*.*)|*.*||", this);
		if(fileDlg.DoModal() == IDOK) {
			m_pMainModel->ClearTestData();
			pTestListBox->ResetContent();

			POSITION pos = fileDlg.GetStartPosition();
			while (pos!=NULL) {
				CString filePath = fileDlg.GetNextPathName(pos);

				HANDLE hSkdFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				m_pMainModel->AddTestFileHandle(hSkdFile);
				pTestListBox->AddString(filePath);

			}
			m_pMainModel->ImportTestFiles();
			m_pMainModel->CloseTestFiles();
		}
	} else {
		MYDEBUG("Test data: real time!\n");
	}
}


void CActionInteract0Dlg::OnBnClickedPredict()
{
	// TODO: Add your control notification handler code here
	int algo = ((CComboBox *)GetDlgItem(IDC_ALGO))->GetCurSel();

	if (algo == ALGO_SVM) {
		MYDEBUG("SVM predict begin!\n");
		m_pMainModel->SVMPredict();
		MYDEBUG("SVM predict end!\n");
	} else if (algo == ALGO_KNN) {
		MYDEBUG("KNN predict begin!\n");
		m_pMainModel->KNNPredict();
		MYDEBUG("KNN predict end!\n");
	}

}


void CActionInteract0Dlg::OnSettingsSetparameters()
{
	// TODO: Add your command handler code here
	CParametersDlg paramDlg;
	INT_PTR nResponse = paramDlg.DoModal();

	if (nResponse == IDOK)
	{
		MYDEBUG("Parameters Set!\n");
	}
	else if (nResponse == IDCANCEL)
	{
		MYDEBUG("Parameters Not Set!\n");
	}
	
}


void CActionInteract0Dlg::OnAboutVersion()
{
	// TODO: Add your command handler code here
	CAboutDlg aboutDlg;
	INT_PTR nResponse = aboutDlg.DoModal();
}
