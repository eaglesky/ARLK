#ifndef _ARLK_BASECLASSIFIER_H
#define _ARLK_BASECLASSIFIER_H



#include "ARLK/SkeData.h"

#define FEATURENUM 39  // = 13 * 3


namespace ARLK {
	class BaseClassifier
	{
	public:
		BaseClassifier();
		~BaseClassifier();
	
		///Extract features from SkeData
		///@param[in]  inputData    SkeData containing all training data information
		///@param[in]  dataLabels   An array containing the data labels for each training data
		///@param[in]  labelNum     Number of labels/training data
		///@return                  True if succeeds
		virtual bool FeatureExtraction(SkeData* inputData, const double* dataLabels = NULL, int labelNum = 0) {return true;}

		///Return true if the training process succeeds
		virtual bool train() {return true;}

		///Classify the data in inputData, store the results in the buffer pointed by labelBuf
		///@param[in]  inputData  SkeData containing all test data information
		///@param[out] pLength    Pointer to the integer indicating the length of the buffer
		///of the predicted labels
		///@return  A pointer to the buffer containing the predicted double labels, should be
		///deleted by the user
		virtual double* predict(SkeData* inputData, int* pLength) { return NULL;}

		///Generate the confusion matrix according to the predicted results
		///Assuming the label numbers can be casted to integers
		///@param[in] predictedLabels A pointer to the buffer containing the predicted labels of the test data
		///@param[in] trueLabels A pointer to the buffer containing the true labels of the test data
		///@param[in] num     Number of the test data
		///@param[in] labelTypeNum A pointer to a integer containing the number of label types
		///@return  A double pointer to the confusion matrix, should be deleted by the user
		static int** genConfusionMatrix(double* predictedLabels, double* trueLabels, int num, int labelTypeNum);

		///Clean all the data except actWeight and set datanum to 0
		virtual void CleanData();

	protected:

		///Calculate a feature vector for one frame.
		///@param[in]  inputData    SkeData containing all training data information
		///@param[in]  frameIdx     Frame index, counting from 1
		///@param[in]  vecIdx       Vector index, counting from 1
		///@param[out] pSkdPointOut A FEATURE_VEC structure containing the calculated vector
		///@param[in]  wt           Weight of the velocity features
		///@return A bool value, true if succeeds
		static bool CalFeatureVector(SkeData* inputData, int frameIdx, int vecIdx, FEATURE_VEC* pSkdPointOut, double wt);

		///Number of the data
		int dataNum;

		double* labels;

		double actWeight;
	};
}
#endif
