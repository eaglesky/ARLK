#ifndef _ARLK_SKEDATA_H
#define _ARLK_SKEDATA_H

#include "ARLK/LibTypeDefs.h"
#include <ctime>
#include <vector>
#include <stdio.h>


namespace ARLK {
	class SkeData
	{
	public:
		SkeData();
		~SkeData();

		///Add one frame data to the SkeData variable
		///@param[in] SkeVector Pointer to Vector4 variable containing the joint coordinates in a frame
		///@return    Return false if it add no more frame(SkeData::IsStopped == true)
		bool AddOneFrame(const Vector4* SkeVector);

		///Initializing saving process
		///@param[in]   FileName     Full path of the file to be saved as
		///@param[in]   FrameLimit   Max frame number to be saved, 0 if unlimited
		///@param[in]   TimeLimit    Limit of the time, 0 if unlimited
		///@return                   True if there's no error
		bool SaveStart(const char* FileName = NULL, int FrameLimit = 0, SKELONG TimeLimit = 0);

		///Ending the saving process and generate the output file if required
		///@return  True if there's no error
		bool SaveEnd();

		///Return true if the saving process is stopped
		bool SavingIsStopped() ;

		///Get the number of frames saved
		int GetFrameSaved();
	
		///Copy SKEFILEHEADER content of the SkeData object and return a pointer to it
		///@return A pointer to SKEFILEHEADER structure containing the file header
		SKEFILEHEADER* GetSFH();

		///Copy SKEINFORHEADER content of the SkeData object and return a pointer to it
		///@return A pointer to SKEINFORHEADER structure containing the file header
		SKEINFORHEADER* GetSIH();

		///Get headers from the skd file
		///@param[in]  pFile   Pointer to the skd file
		///@param[out] psfh    Pointer to the SKEFILEHEADER structure containing the file header
		///@param[out] psih    Pointer to the SKEINFORHEADER structure containing the info header
		///@return             Return true if succeeds
		static bool GetHeaders(FILE* pFile, SKEFILEHEADER *psfh, SKEINFORHEADER *psih);

		///Get headers from the skd file and store them in this SkeData structure
		///@param[in]  pFile   Pointer to the skd file
		///@return             True if succeeds
		bool GetHeaders(FILE* pFile) ;

		///Get all the information from one skd file
		///@param[in]  pFile   Pointer to the skd file
		///@return             True if succeeds
		bool GetFramesFromFile(FILE* pFile);

		///Add the frame information from a skd file
		///@param[in]  pFile   Pointer to the skd file
		///@return             True if succeeds
		bool AddFramesFromFile(FILE* pFile);

		///Write the SkeData content to a skd file
		///@param[in]  FileName  Name of the skd file to be generated
		///@return               True if succeeds
		bool WriteFrames2File(const char* FileName);

		///Clear the content of m_SkePoints, m_cFrame, m_pSFH and m_pSIH
		void ClearContent();

		///Get the position information of a certain skeleton joint
		///@param[in] frameIdx  Frame index, counting from 1
		///@param[in] jointIdx  Joint index, counting from 1
		///@return  A pointer to SKEPOINT structure containing the 3D position information
		///of the joint; NULL if not found
		SKDPOINT* GetPoint(int frameIdx, int jointIdx);

	private:
	
		std::vector<SKDPOINT*> m_SkePoints;

		///Number of Frames that have been saved
		int m_cFrame;

		///Whether current storing is saved or not
		bool IsStopped;


		SKEFILEHEADER* m_pSFH;
		SKEINFORHEADER* m_pSIH;

		clock_t       m_LastFrameTime;

		clock_t       m_StartTime;
		clock_t       m_EndTime;
		SKELONG       m_TimeLimit;

		int         m_iFrameLimit;

		FILE* m_pOutputFile;

	};
}
#endif
