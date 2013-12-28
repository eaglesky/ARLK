#include "ARLK/SVMClassifier.h"

namespace ARLK {
	SVMClassifier::SVMClassifier()
		:SVMNodes(NULL),
		SVMmodel(NULL)
	{
		params.svm_type = C_SVC;

		params.kernel_type = RBF;

		params.degree = 3;

		params.gamma = 0.0001;

		params.coef0 = 0;

		params.nu = 0.5;

		params.cache_size = 100;

		params.C = 10;

		params.eps = 0.001;

		params.p = 0.1;

		params.shrinking = 1;

		params.probability = 0;

		params.nr_weight = 0;

		params.weight_label = NULL;

		params.weight = NULL;


		pro.l = dataNum;
		pro.x = NULL;
		pro.y = NULL;
	}

	SVMClassifier::~SVMClassifier()
	{
		CleanData();
	}

	void SVMClassifier::SetSVMParameters(double C, double gamma, double weight)
	{
		params.C = C;
		params.gamma = gamma;
		actWeight = weight;
	}

	svm_node** SVMClassifier::SVMFeatureExtraction(SkeData* inputData, int* pFrameNum, int* pFeatureNum, double wt)
	{
		*pFeatureNum = FEATURENUM;
		*pFrameNum = inputData->GetFrameSaved();
		svm_node** pNodes = new svm_node*[*pFrameNum];

		for (int i = 0; i < *pFrameNum; i++)
		{
			pNodes[i] = new svm_node[FEATURENUM + 1];
			for (int j = 0; j < FEATURENUM / 3; j++)
			{
				FEATURE_VEC* pVec = new FEATURE_VEC;
				if (!CalFeatureVector(inputData, i+1, j+1, pVec, wt))
					return NULL;

				pNodes[i][j*3].index = j*3 + 1;
				pNodes[i][j*3].value = pVec->x;

				pNodes[i][j*3 + 1].index = j*3 + 2;
				pNodes[i][j*3 + 1].value = pVec->y;

				pNodes[i][j*3 + 2].index = j*3 + 3;
				pNodes[i][j*3 + 2].value = pVec->z;

				delete pVec;
			}
			pNodes[i][FEATURENUM].index = -1;
			pNodes[i][FEATURENUM].value = 0;

		}
		return pNodes;
	}

	bool SVMClassifier::FeatureExtraction(SkeData* inputData, const double* dataLabels, int labelNum)
	{

		CleanNodes();
		int n;
		SVMNodes = SVMFeatureExtraction(inputData, &dataNum, &n, actWeight);

		if (labelNum != dataNum)
			return false;


		if(dataLabels)
		{
			delete []labels;
			labels = new double[dataNum];
			for (int i = 0; i < dataNum; i++)
				labels[i] = dataLabels[i];
		}

		if (SVMNodes)
			return true;
		else
			return false;
	}

	bool SVMClassifier::train()
	{
		pro.l = dataNum;
		pro.x = SVMNodes;
		pro.y = labels;

		if (svm_check_parameter(&pro,&params))
			return false;

		SVMmodel = svm_train(&pro, &params);

		svm_save_model("svm_model",SVMmodel);
		return true;
	}

	double* SVMClassifier::predict(SkeData* inputData, int* pLength)
	{
		int n, nFrames;
		svm_node** pNodes = SVMFeatureExtraction(inputData, &nFrames, &n, actWeight);
		double* ret = new double[nFrames];
		*pLength = nFrames;
		if(!SVMmodel) 
			SVMmodel = svm_load_model("svm_model");

		for (int i = 0; i < nFrames; i++)
		{
			ret[i] = svm_predict(SVMmodel, pNodes[i]);
		}
		return ret;
	}

	void SVMClassifier::CleanData()
	{
		CleanNodes();

		delete SVMmodel;
		SVMmodel = NULL;
		pro.l = 0;
		pro.x = NULL;
		pro.y = NULL;
		BaseClassifier::CleanData();

	}

	void SVMClassifier::CleanNodes()
	{
		for (int i = 0; i < dataNum; i++)
		{
			delete []SVMNodes[i];
			SVMNodes[i] = NULL;
		}
		delete []SVMNodes;
		SVMNodes = NULL;
		delete SVMmodel;
		SVMmodel = NULL;
	}

}