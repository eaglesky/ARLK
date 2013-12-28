#ifndef _ARLK_KNNCLASSIFIER_H
#define _ARLK_KNNCLASSIFIER_H

#include "ARLK/BaseClassifier.h"
#include "ANN.h"

namespace ARLK {
	class KNNClassifier : public BaseClassifier
	{
	public:
		KNNClassifier();
		~KNNClassifier();

		///By default K = 10, eps = 0, weight = 3
		void SetKNNParameters(int K, double eps, double weight);

		virtual bool FeatureExtraction(SkeData* inputData, const double* dataLabels = NULL, int labelNum = 0);

		virtual bool train();

		virtual double* predict(SkeData* inputData, int* pLength);

		virtual void CleanData();

	private:

		///Extract features and store it in ANNpointArray structure
		///@param[in] inputData SkeData containing all training data information
		///@param[out] pFrameNum Number of training data
		///@param[out] pFeatureNum Number of features for each training data
		///@param[in] wt  Weight of action features
		///@return  An ANNpointArray structure containing extracted features
		static ANNpointArray KNNFeatureExtraction(SkeData* inputData, int* pFrameNum, int* pFeatureNum, double wt);

		double GetMostId(ANNidxArray idArray, int K, double* labelArray);

		ANNpointArray		dataPts;				// data points
					
		
		ANNkd_tree*			kdTree;					// search structure

		struct _KNNPARAMS {
			int K;
			double eps;
		}knnParams;

	};
}
#endif
