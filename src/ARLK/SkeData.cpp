#include "ARLK/SkeData.h"
#include <cstdio>

#ifdef _CONSOLEWIN
#define DEBUG(format, ...) printf(format, ## __VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

namespace ARLK {
	SkeData::SkeData() : 
		m_pOutputFile(NULL),
		m_cFrame(0),
		IsStopped(true),
		m_LastFrameTime(0),
		m_StartTime(0),
		m_EndTime(0),
		m_TimeLimit(0),
		m_iFrameLimit(0)
	{
		SKEFILEHEADER sfh = {0};
		SKEINFORHEADER sih = {0};

		m_pSFH = new SKEFILEHEADER;
		m_pSIH = new SKEINFORHEADER;

		*m_pSFH = sfh;
		*m_pSIH = sih;
	}

	SkeData::~SkeData()
	{
		ClearContent();
		delete m_pSFH;
		delete m_pSIH;
	}


	bool SkeData::AddOneFrame(const Vector4* SkeVector)
	{	
		if (!IsStopped) {

			clock_t currentTime = clock() * 1.0 / CLOCKS_PER_SEC * 1000;
			if (m_cFrame == 0)
			{
				m_StartTime = m_LastFrameTime = currentTime - 30;

			}
			if((m_iFrameLimit && (m_cFrame >= m_iFrameLimit))||
				(m_TimeLimit && ((currentTime - m_StartTime)/1000.0 >= m_TimeLimit))) {
				return SaveEnd();
			}


		
			for(int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
			{
				SKDPOINT *ptrpoint=new SKDPOINT;
				ptrpoint->x = static_cast<short>(((SkeVector+j)->x)*100);
				ptrpoint->y = static_cast<short>(((SkeVector+j)->y)*100);
				ptrpoint->z = static_cast<short>(((SkeVector+j)->z)*100);
				ptrpoint->t = static_cast<short>(currentTime - m_LastFrameTime);
				if(m_pOutputFile)
				{

					fwrite(ptrpoint, sizeof(SKDPOINT), 1, m_pOutputFile);
					delete ptrpoint;
				} else {
					m_SkePoints.push_back(ptrpoint);
				}
			}
			m_cFrame++;

			m_LastFrameTime = currentTime;

			return true;
		} else {
			return false;
		}
	}



	bool SkeData::SaveStart(const char* FileName, int FrameLimit, SKELONG TimeLimit)
	{
		m_cFrame = 0;
		m_iFrameLimit = FrameLimit;
		m_TimeLimit = TimeLimit;

		if (FileName) {

			if (!(m_pOutputFile = fopen(FileName, "wb")))
				return false;
		}

		if (m_pOutputFile)
		{
			SKEFILEHEADER sfh_blank = {0};
			SKEINFORHEADER sih_blank = {0};
			SKELONG dwBytes = 0;

			fwrite(&sfh_blank, sizeof(sfh_blank), 1, m_pOutputFile);
			fwrite(&sih_blank, sizeof(sih_blank), 1, m_pOutputFile);
	
		}
	
		IsStopped = false;

		return true;
	}

	bool SkeData::SaveEnd()
	{
		IsStopped = true;
		m_EndTime = m_LastFrameTime;


		m_pSFH->myfType = 0x5344;                                   //'S''D',indicates skeleton data
		m_pSFH->myfSize = sizeof(SKEFILEHEADER);                    //Size of the skelFeton file header
		m_pSFH->myfReserved = 0;
		m_pSFH->myfOffBits = m_pSFH->myfSize+sizeof(SKEINFORHEADER);    // Offset to the start of coordinates data

		m_pSIH->myiTime = m_EndTime - m_StartTime;                //Total time of the skeleton video
		m_pSIH->myiFrame = m_cFrame;                             //Total number of the skeleton frames
		m_pSIH->jointNum = NUI_SKELETON_POSITION_COUNT;                                     //Number of the joints in a skeleton
		m_pSIH->dimensionNum = 5;                                  //Dimension of the coordinate
		m_pSIH->myiSize = sizeof(SKEINFORHEADER);                  //Size of the skeleton information header
	
		if(m_pOutputFile)
		{
			fseek(m_pOutputFile, 0, SEEK_END);
			m_pSIH->myiAllsize = ftell(m_pOutputFile);           //Size of the whole skd file

			fseek(m_pOutputFile, 0, SEEK_SET);


			fwrite(m_pSFH, sizeof(SKEFILEHEADER), 1, m_pOutputFile);
			fwrite(m_pSIH, sizeof(SKEINFORHEADER), 1, m_pOutputFile);
			fclose(m_pOutputFile);


		}
		return true;
	}

	bool SkeData::SavingIsStopped()
	{
		return IsStopped;
	}

	int SkeData::GetFrameSaved()
	{ 
		return m_cFrame;
	}


	SKEFILEHEADER* SkeData::GetSFH()
	{
		SKEFILEHEADER* ret = new SKEFILEHEADER;
		ret->myfType = m_pSFH->myfType;
		ret->myfSize = m_pSFH->myfSize;
		ret->myfReserved = m_pSFH->myfReserved;
		ret->myfOffBits = m_pSFH->myfOffBits;

		return ret;
	}

	SKEINFORHEADER* SkeData::GetSIH()
	{
		SKEINFORHEADER* ret = new SKEINFORHEADER;
		ret->dimensionNum = m_pSIH->dimensionNum;
		ret->jointNum = m_pSIH->jointNum;
		ret->myiAllsize = m_pSIH->myiAllsize;
		ret->myiFrame = m_pSIH->myiFrame;
		ret->myiSize = m_pSIH->myiSize;
		ret->myiTime = m_pSIH->myiTime;

		return ret;
	}



	bool SkeData::GetHeaders(FILE* pFile, SKEFILEHEADER *psfh, SKEINFORHEADER *psih) 
	{
		fseek(pFile, 0, SEEK_SET);
		fread(psfh, sizeof(SKEFILEHEADER), 1, pFile);
		fread(psih, sizeof(SKEINFORHEADER), 1, pFile);

		return true;
	}

	bool SkeData::GetHeaders(FILE* pFile)
	{
		return SkeData::GetHeaders(pFile, m_pSFH, m_pSIH);
	}

	bool SkeData::GetFramesFromFile(FILE* pFile)
	{
		if (!pFile)
			return false;

		fseek(pFile, 0, SEEK_SET);
		ClearContent();

		if(!GetHeaders(pFile))
			return false;

		int nFrames = m_pSIH->myiFrame;
		int nTotalPoints = nFrames * m_pSIH->jointNum;

		m_cFrame = nFrames;
		for (int i = 0; i < nTotalPoints; i++)
		{
			SKDPOINT *ptrpoint=new SKDPOINT;

			fread(ptrpoint, sizeof(SKDPOINT), 1, pFile);
			m_SkePoints.push_back(ptrpoint);
		}
		return true;

	}

	bool SkeData::AddFramesFromFile(FILE* pFile)
	{
		fseek(pFile, 0, SEEK_SET);
		SKEFILEHEADER sfh = {0};
		SKEINFORHEADER sih = {0};

		if (!SkeData::GetHeaders(pFile, &sfh, &sih))
			return false;

		*m_pSFH = sfh;

		m_pSIH->myiTime += sih.myiTime;
		m_pSIH->myiFrame += sih.myiFrame;
		m_pSIH->jointNum = sih.jointNum;
		m_pSIH->dimensionNum = sih.dimensionNum;
		m_pSIH->myiSize = sih.myiSize;

		m_pSIH->myiAllsize += sih.myiFrame * sizeof(SKDPOINT) * sih.jointNum;

		int nFrames = sih.myiFrame;
		int nTotalPoints = nFrames * sih.jointNum;

		m_cFrame += nFrames;

		for (int i = 0; i < nTotalPoints; i++)
		{
			SKDPOINT *ptrpoint=new SKDPOINT;

			fread(ptrpoint, sizeof(SKDPOINT), 1, pFile);

			m_SkePoints.push_back(ptrpoint);
		}
		return true;

	}

	bool SkeData::WriteFrames2File(const char* FileName)
	{
		if (!FileName)
			return false;

		if (!SaveStart(FileName))
			return false;



		std::vector<SKDPOINT* >::iterator iter = m_SkePoints.begin();
		for (; iter != m_SkePoints.end(); iter++)
		{

			fwrite(*iter, sizeof(SKDPOINT), 1, m_pOutputFile);
		}
	
		if(m_pOutputFile)
		{

			fseek(m_pOutputFile, 0, SEEK_END);
			m_pSIH->myiAllsize = ftell(m_pOutputFile);           //Size of the whole skd file
			fseek(m_pOutputFile, 0, SEEK_SET);

			fwrite(m_pSFH, sizeof(SKEFILEHEADER), 1, m_pOutputFile);
			fwrite(m_pSIH, sizeof(SKEINFORHEADER), 1, m_pOutputFile);

			fclose(m_pOutputFile);
			return true;
		} else 
			return false;

	}

	void SkeData::ClearContent()
	{
		std::vector<SKDPOINT* >::iterator iter = m_SkePoints.begin();
		for (; iter != m_SkePoints.end(); iter++)
		{
			if ((*iter)!=NULL) {
				delete (*iter);
				*iter = NULL;
			}

		}
		m_SkePoints.clear();

		m_cFrame = 0;
		delete m_pSFH;
		m_pSFH = new SKEFILEHEADER;
		delete m_pSIH;
		m_pSIH = new SKEINFORHEADER;

		SKEFILEHEADER sfh = {0};
		SKEINFORHEADER sih = {0};

		*m_pSFH = sfh;
		*m_pSIH = sih;
	}

	SKDPOINT* SkeData::GetPoint(int frameIdx, int jointIdx)
	{
		if ((frameIdx > 0) && (frameIdx <= m_cFrame) 
			&& (jointIdx > 0) && (jointIdx <= NUI_SKELETON_POSITION_COUNT))
		  return m_SkePoints.at((frameIdx - 1) * NUI_SKELETON_POSITION_COUNT + jointIdx - 1);

		return NULL;
	}

}