#ifndef _MAINMODEL_H
#define _MAINMODEL_H

#include "ARLK/ARLK.h"
#include "stdafx.h"
#include "MyDebug.h"
#include <string>
#define FROM_FILE 0
#define REAL_TIME 1
#define ALGO_SVM 0
#define ALGO_KNN 1
#define MAX_ACTION_TYPE_NUM 100

class MainModel 
{
public:
	MainModel();
	~MainModel();
	BOOL GetDlgHandle(HWND hDlg, int dlgType);
	BOOL AddTrainFileHandle(HANDLE hFile);
	BOOL AddTestFileHandle(HANDLE hFile);
	BOOL ClearTrainData();
	BOOL ClearTestData();
	BOOL ImportTrainFiles();
	BOOL ImportTestFiles();

	BOOL SVMTrain();
	BOOL SVMPredict();
	
	BOOL GetSVMParams(double* pC, double* pGamma, double* pWT);
	BOOL SetSVMParams(double C, double gamma, double wt);

	BOOL KNNTrain();
	BOOL KNNPredict();

	BOOL GetKNNParams(int* pK, double* pEps, double* pWT);
	BOOL SetKNNParams(int K, double eps, double wt);

	void CloseTrainFiles();
	void CloseTestFiles();

private:   
	void WriteTrainFileLabelStrs();
	void ReadTrainFileLabelStrs();
	
	FILE* ConvertHandle2FilePt(HANDLE hFile, const char* mode);
	HWND m_hMainDlg;
	HWND m_hTrainDlg;
	HWND m_hTestDlg;

	int m_nTrainDataSource;
	int m_nTestDataSource;
	int m_nAlgoChoice;

	int m_nTrainFiles;
	
	std::vector<double> trainVecLabels;
	int m_nTestFiles;
	ARLK::SkeData* m_pTrainSkeData;
	ARLK::SkeData* m_pTestSkeData;

	HANDLE* m_phTrainFiles;
	HANDLE* m_phTestFiles;
	
	std::string m_pLabelStrs[MAX_PATH];

	struct _TRAINPARAMS{
		struct _SVMPARAMS{
			double C;
			double gamma;
		}svmParams;

		double feaWt;

		struct _KNNPARAMS {
			int K;
			double eps;
		}knnParams;

	}m_trainParams;

	ARLK::SVMClassifier* m_pSVM;
	ARLK::KNNClassifier* m_pKNN;
};

#endif