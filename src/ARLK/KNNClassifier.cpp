#include "ARLK/KNNClassifier.h"


#ifdef __gnu_linux__
#include <ext/hash_map>
using  namespace __gnu_cxx;
#else
#include <hash_map>
using namespace std;

#endif

namespace ARLK {
	KNNClassifier::KNNClassifier()
		:dataPts(NULL),
		 kdTree(NULL)
	{
		knnParams.K = 10;
		knnParams.eps = 0;
	}

	KNNClassifier::~KNNClassifier()
	{
		annDeallocPts(dataPts);
		delete kdTree;
		annClose();
	}

	void KNNClassifier::SetKNNParameters(int K, double eps, double weight)
	{
		knnParams.K = K;
		knnParams.eps = eps;
		actWeight = weight;
	}

	ANNpointArray KNNClassifier::KNNFeatureExtraction(SkeData* inputData, int* pFrameNum, int* pFeatureNum, double wt)
	{
		*pFeatureNum = FEATURENUM;
		*pFrameNum = inputData->GetFrameSaved();

	
		ANNpointArray ret = annAllocPts(*pFrameNum, FEATURENUM);

		for (int i = 0; i < *pFrameNum; i++)
		{
			for (int j = 0; j < FEATURENUM / 3; j++)
			{
				FEATURE_VEC* pVec = new FEATURE_VEC;
				if (!CalFeatureVector(inputData, i+1, j+1, pVec, wt))
					return NULL;

				ret[i][j*3] = pVec->x;
				ret[i][j*3 + 1] = pVec->y;
				ret[i][j*3 + 2] = pVec->z;

				delete pVec;
			}
		}
		return ret;
	}

	bool KNNClassifier::FeatureExtraction(SkeData* inputData, const double* dataLabels, int labelNum)
	{

		//CleanNodes();
		int n;
		dataPts = KNNFeatureExtraction(inputData, &dataNum, &n, actWeight);

		if (labelNum != dataNum)
			return false;


		if(dataLabels)
		{
			delete []labels;
			labels = new double[dataNum];
			for (int i = 0; i < dataNum; i++)
				labels[i] = dataLabels[i];
		}

		if (dataPts)
			return true;
		else
			return false;
	
	}

	bool KNNClassifier::train()
	{
		if (!dataPts)
			return false;

		kdTree = new ANNkd_tree(					// build search structure
			dataPts,					// the data points
			dataNum,						// number of points
			FEATURENUM);						// dimension of space

		return true;

	}

	double* KNNClassifier::predict(SkeData* inputData, int* pLength)
	{
		int n, nFrames;
		ANNpointArray testPts = KNNFeatureExtraction(inputData, &nFrames, &n, actWeight);

		double* ret = new double[nFrames];
		*pLength = nFrames;

		ANNidxArray			nnIdx = new ANNidx[knnParams.K];	
		ANNdistArray		dists = new ANNdist[knnParams.K];	

		for (int i = 0; i < nFrames; i++)
		{
			kdTree->annkSearch(						// search
				testPts[i],						// query point
				knnParams.K,								// number of near neighbors
				nnIdx,							// nearest neighbors (returned)
				dists,							// distance (returned)
				knnParams.eps);							// error bound
		
		
			ret[i] = GetMostId(nnIdx, knnParams.K, labels);
		}
		delete []nnIdx;							// clean things up
		delete []dists;
		return ret;
	}

	void KNNClassifier::CleanData()
	{
		annDeallocPts(dataPts);
		delete kdTree;

		BaseClassifier::CleanData();
	}

	double KNNClassifier::GetMostId(ANNidxArray idArray, int K, double* labelArray)
	{
		hash_map<int,int> countMap;

		hash_map<int,int>::iterator iter, iter_temp;
		for (int i = 0; i < K; i++)
		{
			int curLabel = labelArray[idArray[i]];
			iter = countMap.find(curLabel);
			if (iter != countMap.end())
				countMap[curLabel] = iter->second + 1;
			else
				countMap[curLabel] = 0;
		}

		int max_count = 0;
		for (iter_temp = iter = countMap.begin(); iter != countMap.end(); iter++)
		{
			if (iter->second > max_count) {
				max_count = iter->second;
				iter_temp = iter;
			}
		}

		double ret = iter_temp->first;
	
		return ret;
	}
}