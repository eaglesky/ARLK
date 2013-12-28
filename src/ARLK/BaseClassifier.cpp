#include "ARLK/BaseClassifier.h"
#include <math.h>

namespace ARLK {
	BaseClassifier::BaseClassifier():
		labels(NULL),
		dataNum(0),
		actWeight(3)
	{
		CleanData();
	}

	BaseClassifier::~BaseClassifier()
	{
		CleanData();
	}

	int** BaseClassifier::genConfusionMatrix(double* predictedLabels, double* trueLabels, int num, int labelTypeNum)
	{
		int** ret = new int*[labelTypeNum];

		for (int i = 0; i < labelTypeNum; i++) 
		{
			ret[i] = new int[labelTypeNum];
			for (int j = 0; j < labelTypeNum; j++)
				ret[i][j] = 0;
		}

		for (int i = 0; i < num; i++)
		{
			ret[(int)trueLabels[i]][(int)predictedLabels[i]]++;
		}

		return ret;
	}

	void BaseClassifier::CleanData()
	{
		delete []labels;
		labels = NULL;
		dataNum = 0;
	}

	bool BaseClassifier::CalFeatureVector(SkeData* inputData, int frameIdx, int vecIdx, FEATURE_VEC* pSkdPointOut, double wt)
	{
		if (!pSkdPointOut)
			pSkdPointOut = new FEATURE_VEC;

		int nFrames = inputData->GetFrameSaved();
		if (frameIdx > nFrames)
			return false;

		int ptid1, ptid2;
		switch(vecIdx)
		{
		case 1:
			ptid1 = NUI_SKELETON_POSITION_WRIST_LEFT;
			ptid2 = NUI_SKELETON_POSITION_ELBOW_LEFT;
			break;

		case 2:
			ptid1 = NUI_SKELETON_POSITION_WRIST_RIGHT;
			ptid2 = NUI_SKELETON_POSITION_ELBOW_RIGHT;
			break;

		case 3:
			ptid1 = NUI_SKELETON_POSITION_ELBOW_LEFT;
			ptid2 = NUI_SKELETON_POSITION_SHOULDER_LEFT;
			break;

		case 4:
			ptid1 = NUI_SKELETON_POSITION_ELBOW_RIGHT;
			ptid2 = NUI_SKELETON_POSITION_SHOULDER_RIGHT;
			break;

		case 5:
			ptid1 = NUI_SKELETON_POSITION_HIP_LEFT;
			ptid2 = NUI_SKELETON_POSITION_KNEE_LEFT;
			break;

		case 6:
			ptid1 = NUI_SKELETON_POSITION_HIP_RIGHT;
			ptid2 = NUI_SKELETON_POSITION_KNEE_RIGHT;
			break;

		case 7:
			ptid1 = NUI_SKELETON_POSITION_KNEE_LEFT;
			ptid2 = NUI_SKELETON_POSITION_ANKLE_LEFT;
			break;

		case 8:
			ptid1 = NUI_SKELETON_POSITION_KNEE_RIGHT;
			ptid2 = NUI_SKELETON_POSITION_ANKLE_RIGHT;
			break;

		case 9:
			ptid1 = NUI_SKELETON_POSITION_SHOULDER_CENTER;
			ptid2 = NUI_SKELETON_POSITION_SPINE;
			break;

		case 10:
			ptid1 = NUI_SKELETON_POSITION_WRIST_LEFT;
			ptid2 = NUI_SKELETON_POSITION_WRIST_LEFT;
			break;

		case 11:
			ptid1 = NUI_SKELETON_POSITION_WRIST_RIGHT;
			ptid2 = NUI_SKELETON_POSITION_WRIST_RIGHT;
			break;

		case 12:
			ptid1 = NUI_SKELETON_POSITION_KNEE_LEFT;
			ptid2 = NUI_SKELETON_POSITION_KNEE_LEFT;
			break;

		case 13:
			ptid1 = NUI_SKELETON_POSITION_KNEE_RIGHT;
			ptid2 = NUI_SKELETON_POSITION_KNEE_RIGHT;
			break;

		default:
			return false;
		}

		SKDPOINT* pSkdPoint1;
		SKDPOINT* pSkdPoint2;

		if (vecIdx < 10)
		{
			pSkdPoint1 = inputData->GetPoint(frameIdx, ptid1 + 1);
			pSkdPoint2 = inputData->GetPoint(frameIdx, ptid2 + 1);
			double x = pSkdPoint1->x - pSkdPoint2->x;
			double y = pSkdPoint1->y - pSkdPoint2->y;
			double z = pSkdPoint1->z - pSkdPoint2->z;
			double len = sqrt(x*x + y*y + z*z);

			if (len > 0)
			{
				pSkdPointOut->x = x / len;
				pSkdPointOut->y = y / len;
				pSkdPointOut->z = z / len;
			} else {
				pSkdPointOut->x = 0.0;
				pSkdPointOut->y = 0.0;
				pSkdPointOut->z = 0.0;
			}
		} else {
			pSkdPoint1 = inputData->GetPoint(frameIdx, ptid1 + 1);

			if (frameIdx > 1)
				pSkdPoint2 = inputData->GetPoint(frameIdx - 1, ptid2 + 1);
			else
				pSkdPoint2 = inputData->GetPoint(frameIdx, ptid2 + 1);

			double time = pSkdPoint1->t;
			if (time > 0)
			{
				pSkdPointOut->x = (pSkdPoint1->x - pSkdPoint2->x) / time * wt;
				pSkdPointOut->y = (pSkdPoint1->y - pSkdPoint2->y) / time * wt;
				pSkdPointOut->z = (pSkdPoint1->z - pSkdPoint2->z) / time * wt;
			} else {
				pSkdPointOut->x = 0.0;
				pSkdPointOut->y = 0.0;
				pSkdPointOut->z = 0.0;
			}

		}
	
	
		return true;
	}
}