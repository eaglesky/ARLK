#ifndef _ARLK_SVMCLASSIFIER_H
#define _ARLK_SVMCLASSIFIER_H

#include "ARLK/BaseClassifier.h"
#include "svm.h"

namespace ARLK {
	class SVMClassifier : public BaseClassifier
	{
	public:
		SVMClassifier();
		~SVMClassifier();

		///Set SVM training parameters
		///By default, C = 10, gamma = 0.001, weight = 3
		///@param[in] C SVM training parameter C in svm_parameter
		///@param[in] gamma SVM training parameter gamma in svm_parameter
		///@param[in] weight Factor of the action features
		void SetSVMParameters(double C, double gamma, double weight);

	

		virtual bool FeatureExtraction(SkeData* inputData, const double* dataLabels = NULL, int labelNum = 0);

		///Create the svm model and save it as svm_model
		virtual bool train();

		virtual double* predict(SkeData* inputData, int* pLength);

		///Clean all the data except parameters and set data number to 0
		virtual void CleanData();

	private:
		///Extract features and store then in SVM nodes
		///@param[in] inputData SkeData containing all training data information
		///@param[out] pFrameNum Number of training data
		///@param[out] pFeatureNum Number of features for each training data
		///@param[in] wt  Weight of action features
		///@return  A double pointer to two-dimensional svm_node
		static svm_node** SVMFeatureExtraction(SkeData* inputData, int* pFrameNum, int* pFeatureNum, double wt);

		///Clear information about libsvm nodes---SVMNodes and SVMmodel
		void CleanNodes();
	
		svm_node** SVMNodes;
		svm_parameter params;
		svm_problem pro;
		svm_model* SVMmodel;

	
	};
}
#endif
