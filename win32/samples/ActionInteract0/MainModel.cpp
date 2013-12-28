#include "MainModel.h"
#include "stdafx.h"
#include <strsafe.h>
#include <fcntl.h>
#include <io.h>
MainModel::MainModel():
        m_nTrainDataSource(FROM_FILE),
		m_nTestDataSource(FROM_FILE),
		m_nAlgoChoice(ALGO_SVM),
		m_nTrainFiles(0),
		m_nTestFiles(0),
        m_pTrainSkeData(NULL),
		m_pTestSkeData(NULL),
		m_hMainDlg(NULL),
		m_hTrainDlg(NULL),
		m_hTestDlg(NULL),
		m_phTrainFiles(NULL),
		m_phTestFiles(NULL),
		m_pSVM(NULL),
		m_pKNN(NULL)
{

	SetSVMParams(10, 0.001, 3);
	SetKNNParams(10, 0, 3);
}

MainModel::~MainModel()
{

	ClearTrainData();
	ClearTestData();
	delete m_pKNN;
	delete m_pSVM;
}

BOOL MainModel::GetDlgHandle(HWND hDlg, int dlgType)
{
	switch(dlgType) {
	case 0:
		m_hMainDlg = hDlg;
		break;
	case 1:
		m_hTrainDlg = hDlg;
		break;
	case 2:
		m_hTestDlg = hDlg;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL MainModel::AddTrainFileHandle(HANDLE hFile)
{
	if (!m_phTrainFiles)
		m_phTrainFiles = new HANDLE[MAX_ACTION_TYPE_NUM];
	

	if (m_nTrainFiles >= MAX_ACTION_TYPE_NUM)
		return FALSE;
	else {
		TCHAR filePath[MAX_PATH];
		m_phTrainFiles[m_nTrainFiles] = hFile;
		GetFinalPathNameByHandle(hFile, filePath, MAX_PATH, VOLUME_NAME_NONE);

		
		std::string filePathStr(filePath);   
		int slashPos = filePathStr.find_last_of('\\');
		int dotPos = filePathStr.find_last_of('.');
		int len = dotPos - slashPos - 1;
	
		m_pLabelStrs[m_nTrainFiles].assign(filePathStr, slashPos+1, len);

		
		std::cout << m_nTrainFiles << ": " << m_pLabelStrs[m_nTrainFiles] << std::endl;
		m_nTrainFiles++;
		return TRUE;
	}
}

BOOL MainModel::AddTestFileHandle(HANDLE hFile)
{
	if (!m_phTestFiles)
		m_phTestFiles = new HANDLE[MAX_ACTION_TYPE_NUM];


	if (m_nTestFiles >= MAX_ACTION_TYPE_NUM)
		return FALSE;
	else {
		TCHAR filePath[MAX_PATH];
		m_phTestFiles[m_nTestFiles] = hFile;
		GetFinalPathNameByHandle(hFile, filePath, MAX_PATH, VOLUME_NAME_NONE);


		std::string filePathStr(filePath);   
		

		std::cout << m_nTestFiles << ": " << filePathStr << std::endl;
		m_nTestFiles++;
		return TRUE;
	}
}

BOOL MainModel::ClearTrainData()
{
	if(m_phTrainFiles) {
		for (int i = 0; i < m_nTrainFiles; i++)
		{
			if(m_phTrainFiles[i])
			  CloseHandle(m_phTrainFiles[i]);

			m_pLabelStrs[m_nTrainFiles].clear();
		}
		m_nTrainFiles = 0;
		delete []m_phTrainFiles;
		m_phTrainFiles = NULL;
	} else {
		for (int i = 0; i < m_nTrainFiles; i++)
		{
			m_pLabelStrs[m_nTrainFiles].clear();
		}
		m_nTrainFiles = 0;
	}

	if(m_pTrainSkeData) {
      delete m_pTrainSkeData;
	  m_pTrainSkeData = NULL;
	}

	trainVecLabels.clear();
	return TRUE;
}

BOOL MainModel::ClearTestData()
{
	if(m_phTestFiles) {
		for (int i = 0; i < m_nTestFiles; i++)
		{
			if (m_phTestFiles[i])
			  CloseHandle(m_phTestFiles[i]);
			
		}
		m_nTestFiles = 0;
		delete []m_phTestFiles;
		m_phTestFiles = NULL;
	}

	if(m_pTestSkeData) {
		delete m_pTestSkeData;
		m_pTestSkeData = NULL;
	}

	return TRUE;
}

void MainModel::WriteTrainFileLabelStrs()
{
	FILE* fp;
	if (fp = fopen("train_file_labels.txt", "wt+"))
	{
		fprintf(fp, "%d\n", m_nTrainFiles);
		for (int i = 0; i < m_nTrainFiles; i++)
		{
			fprintf(fp, "%s\n", m_pLabelStrs[i].c_str());
		}
	}
	fclose(fp);
}

void MainModel::ReadTrainFileLabelStrs()
{
	FILE* fp;
	if (fp = fopen("train_file_labels.txt", "rb+"))
	{
		fscanf(fp, "%d\n", &m_nTrainFiles);
		for (int i = 0; i < m_nTrainFiles; i++)
		{
			char labelStr[MAX_PATH];
			fscanf(fp, "%s\n", labelStr);
			std::string tempStr(labelStr);
			m_pLabelStrs[i] = tempStr;
		}
	}
	fclose(fp);
}

BOOL MainModel::ImportTrainFiles()
{
	m_pTrainSkeData = new ARLK::SkeData;

	for (int i = 0; i < m_nTrainFiles; i++)
	{
		FILE* pTempFile = ConvertHandle2FilePt(m_phTrainFiles[i], "rb");
		if(!m_pTrainSkeData->AddFramesFromFile(pTempFile))
			return FALSE;

		SKEFILEHEADER sfh = {0};
		SKEINFORHEADER sih = {0};
		int curFrameNum = 0;
		if (ARLK::SkeData::GetHeaders(pTempFile, &sfh, &sih))
			curFrameNum = sih.myiFrame;

		for (int j = 0; j < curFrameNum; j++)
			trainVecLabels.push_back(i);
			
		

	}

	WriteTrainFileLabelStrs();
	return TRUE;
}

BOOL MainModel::ImportTestFiles()
{
	m_pTestSkeData = new ARLK::SkeData;

	for (int i = 0; i < m_nTestFiles; i++)
	{
		FILE* pTempFile = ConvertHandle2FilePt(m_phTestFiles[i], "rb");
		if(!m_pTestSkeData->AddFramesFromFile(pTempFile))
		{

			return FALSE;
		}

	}
	return TRUE;
}

BOOL MainModel::SVMTrain()
{
	//Test ske data..
	/*TCHAR testName[MAX_PATH];
	TCHAR timeString[MAX_PATH];
	GetTimeFormat(NULL,TIME_FORCE24HOURFORMAT, NULL, "hh'-'mm'-'ss", timeString, _countof(timeString));
	StringCchPrintf(testName, _countof(testName), "..\\output\\KinectSkeletonVideo-%s.skd",timeString);
	if (m_pTrainSkeData->WriteFrames2File(testName))
		MYDEBUG("Test file saved successfully!\n");
	else 
		MYDEBUG("Failed to save test file!\n");*/

	if (m_pSVM)
		delete m_pSVM;

	m_pSVM = new ARLK::SVMClassifier();

	int labelNum = trainVecLabels.size();

	double* labels = new double[labelNum];
	for (int i = 0; i < labelNum; i++)
	{
		labels[i] = trainVecLabels.at(i);
	}

	m_pSVM->SetSVMParameters(m_trainParams.svmParams.C, m_trainParams.svmParams.gamma,
		m_trainParams.feaWt);
	m_pSVM->FeatureExtraction(m_pTrainSkeData, labels, labelNum);
	m_pSVM->train();

	return TRUE;
}

BOOL MainModel::SVMPredict()
{
	/*TCHAR testName[MAX_PATH];
	TCHAR timeString[MAX_PATH];
	GetTimeFormat(NULL,TIME_FORCE24HOURFORMAT, NULL, "hh'-'mm'-'ss", timeString, _countof(timeString));
	StringCchPrintf(testName, _countof(testName), "..\\output\\KinectSkeletonVideo-%s.skd",timeString);
	if (m_pTestSkeData->WriteFrames2File(testName))
		MYDEBUG("Test file saved successfully!\n");
	else 
		MYDEBUG("Failed to save test file!\n");*/

	if (!m_pSVM)
		m_pSVM = new ARLK::SVMClassifier();

	if (!m_pTestSkeData)
		return FALSE;

	int resultsNum = 0;
	double* results = m_pSVM->predict(m_pTestSkeData, &resultsNum);

	if (m_nTrainFiles == 0)
		ReadTrainFileLabelStrs();

	FILE* fp;
	if (fp = fopen("svm_predict.txt", "wt+"))
	{
		for (int i = 0; i < resultsNum; i++)
		{
			fprintf(fp, "%s\n", m_pLabelStrs[(int)results[i]].c_str());
		}
	}
	fclose(fp);

	/*if (fp = fopen("svm_confusion_matrix.txt", "wt+"))
	{
		int** svmConfusionMatrix = LibHAR::genConfusionMatrix()
	}*/
	return TRUE;
}

BOOL MainModel::GetSVMParams(double* pC, double* pGamma, double* pWT)
{
	*pC = m_trainParams.svmParams.C;
	*pGamma = m_trainParams.svmParams.gamma;
	*pWT = m_trainParams.feaWt;

	return TRUE;
}

BOOL MainModel::SetSVMParams(double C, double gamma, double wt)
{
	m_trainParams.svmParams.C = C;
	m_trainParams.svmParams.gamma = gamma;
	m_trainParams.feaWt = wt;

	return TRUE;
}

BOOL MainModel::KNNTrain()
{
	if (m_pKNN)
		delete m_pKNN;

	m_pKNN = new ARLK::KNNClassifier();

	int labelNum = trainVecLabels.size();

	double* labels = new double[labelNum];
	for (int i = 0; i < labelNum; i++)
	{
		labels[i] = trainVecLabels.at(i);
	}

	m_pKNN->SetKNNParameters(m_trainParams.knnParams.K, m_trainParams.knnParams.eps,
		m_trainParams.feaWt);

	m_pKNN->FeatureExtraction(m_pTrainSkeData, labels, labelNum);
	m_pKNN->train();
	return TRUE;
}

BOOL MainModel::KNNPredict()
{
	if (!m_pKNN)
		return FALSE;

	if (!m_pTestSkeData)
		return FALSE;

	int resultsNum = 0;
	double* results = m_pKNN->predict(m_pTestSkeData, &resultsNum);

	if (m_nTrainFiles == 0)
		ReadTrainFileLabelStrs();

	FILE* fp;
	if (fp = fopen("knn_predict.txt", "wt+"))
	{
		for (int i = 0; i < resultsNum; i++)
		{
			fprintf(fp, "%s\n", m_pLabelStrs[(int)results[i]].c_str());
		}
	}
	fclose(fp);

	return TRUE;
}

BOOL MainModel::GetKNNParams(int* pK, double* pEps, double* pWT)
{
	*pK = m_trainParams.knnParams.K;
	*pEps = m_trainParams.knnParams.eps;
	*pWT = m_trainParams.feaWt;

	return TRUE;
}

BOOL MainModel::SetKNNParams(int K, double eps, double wt)
{
	m_trainParams.knnParams.K = K;
	m_trainParams.knnParams.eps = eps;
	m_trainParams.feaWt = wt;

	return TRUE;
}

void MainModel::CloseTrainFiles()
{
	for (int i = 0; i < m_nTrainFiles; i++) {
		CloseHandle(m_phTrainFiles[i]);
		m_phTrainFiles[i] = NULL;
	}
}

void MainModel::CloseTestFiles()
{
	for (int i = 0; i < m_nTestFiles; i++) {
		CloseHandle(m_phTestFiles[i]);
		m_phTestFiles[i] = NULL;
	}
}

FILE* MainModel::ConvertHandle2FilePt(HANDLE hFile, const char* mode) {
	int nHandle = _open_osfhandle((long)hFile, _O_BINARY);
	if (nHandle != -1)
		return _fdopen(nHandle, mode);
	else
		return NULL;
}