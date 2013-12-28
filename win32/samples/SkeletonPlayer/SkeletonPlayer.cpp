#include "stdafx.h"
#include "SkeletonPlayer.h"
#include <strsafe.h>



static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	CSkeletonPlayer application;
	application.Run(hInstance, nCmdShow);
}

/// <summary>
/// Constructor
/// </summary>
CSkeletonPlayer::CSkeletonPlayer() :
    m_pD2DFactory(NULL),
	m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
	m_pSkeletonStreamHandle(INVALID_HANDLE_VALUE),
	//m_bSeatedMode(false),
	m_pRenderTarget(NULL),
	m_pBrushJointTracked(NULL),
	m_pBrushJointInferred(NULL),
	m_pBrushBoneTracked(NULL),
	m_pBrushBoneInferred(NULL),
	skdfile(NULL),
	m_hOutputFile(NULL),
	m_bPlayEnabled(false),
	m_bSliderMoveEnabled(true),
	m_bSelectBegin(false),
	m_bAutoPlayDisabled(false),
	m_pNuiSensor(NULL)
{
	ZeroMemory(m_Points,sizeof(m_Points));
}

/// <summary>
/// Destructor
/// </summary>
CSkeletonPlayer::~CSkeletonPlayer()
{
	if (m_pNuiSensor)
	{
		m_pNuiSensor->NuiShutdown();
	}

	if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonEvent);
	}

	// clean up Direct2D objects
	DiscardDirect2DResources();

	// clean up Direct2D
	SafeRelease(m_pD2DFactory);

	SafeRelease(m_pNuiSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int CSkeletonPlayer::Run(HINSTANCE hInstance, int nCmdShow)
{
	MSG       msg = {0};
	WNDCLASS  wc  = {0};

	// Dialog custom window class
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.cbWndExtra    = DLGWINDOWEXTRA;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
	wc.lpfnWndProc   = DefDlgProcW;
	wc.lpszClassName = L"SkeletonPlayerAppDlgWndClass";

	if (!RegisterClassW(&wc))
	{
		return 0;
	}

	// Create main application window
	HWND hWndApp = CreateDialogParamW(
		hInstance,
		MAKEINTRESOURCE(IDD_APP),
		NULL,
		(DLGPROC)CSkeletonPlayer::MessageRouter, 
		reinterpret_cast<LPARAM>(this));

	// Show window
	ShowWindow(hWndApp, nCmdShow);

	const int eventCount = 1;
	//HANDLE hEvents[eventCount];

	// Main message loop
	while (WM_QUIT != msg.message)
	{
		

		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// If a dialog message will be taken care of by the dialog proc
			if ((hWndApp != NULL) && IsDialogMessageW(hWndApp, &msg))
			{
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return static_cast<int>(msg.wParam);
}



/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CSkeletonPlayer::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSkeletonPlayer* pThis = NULL;

	if (WM_INITDIALOG == uMsg)
	{
		pThis = reinterpret_cast<CSkeletonPlayer*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else
	{
		pThis = reinterpret_cast<CSkeletonPlayer*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pThis)
	{
		return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CSkeletonPlayer::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;
	TCHAR   szFile[MAX_PATH];
	TCHAR   totalTime[256];
	TCHAR   startTime[256];
	TCHAR   currentTime[256];
	DWORD dwBytesWritten;
	int iVideoTime;
	int iSec,iMin,iHour;
	LOGFONT lf;
	bool bOutputFileSuccess=true;
	SKDPOINT tSkeCordBuf;
	DWORD dwWrittenFrameNum;
	DWORD dwFrameCount=0;
	SKEINFORHEADER tsih;
	DWORD highfilesize;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			// Bind application window handle
			m_hWnd = hWnd;

			// Init Direct2D
			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

			// Look for a connected Kinect, and create it if found
			//CreateFirstConnected();
			GetObject( (HFONT) GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf );
			lf.lfHeight *= 3;
			SendDlgItemMessage( hWnd, IDC_FPS, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf),0);
			SetWindowText(GetDlgItem(hWnd,IDC_PLAY_STEP),L"1");
		}
		break;

		// If the titlebar X is clicked, destroy app
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		//CloseHandle(skdfile);
		fclose(skdfile);
		// Quit the main message pump
		PostQuitMessage(0);
		break;

		// Handle button press
	case WM_COMMAND:
		if (IDC_BROWSE == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))   
		{
			ZeroMemory(&ofn,sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = TEXT('\0');
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = TEXT("skd\0*.skd\0");
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.hwndOwner =hWnd;
			ofn.Flags = OFN_EXPLORER |OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn))
			{		

				SetDlgItemText(hWnd,IDC_PATH,szFile);
				StringCchPrintfW(skdname, _countof(skdname),szFile);
				if(skdfile)
					fclose(skdfile);
				//skdfile=CreateFileW(skdname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				USES_CONVERSION;
				const char* ascName = T2A(skdname);
				skdfile = fopen(ascName, "rb");

				if(GetLastError()==ERROR_FILE_NOT_FOUND )
				{
					StringCchPrintf( StatusMessage, cStatusMessageMaxLen, L"Failed to open the file!");
					SetStatusMessage(StatusMessage);
					//CloseHandle(skdfile);
					fclose(skdfile);
				}
				else{
					if(ARLK::SkeData::GetHeaders(skdfile,&sfh,&sih))
					{
						StringCchPrintf( StatusMessage, cStatusMessageMaxLen, L"Successfully opened the file!");
						SetStatusMessage(StatusMessage);

						//DEBUG("sfh:%x,%x,%x,%x\n",sfh.myfType,sfh.myfSize,sfh.myfReserved,sfh.myfOffBits);
						//DEBUG("sih:%d,%d,%d,%d,%d,%d\n",sih.myiTime,sih.myiFrame,sih.jointNum,sih.dimensionNum,sih.myiSize,sih.myiAllsize);

						iVideoTime=sih.myiTime;
						iSec=((int)(iVideoTime/1000))%60;
						iMin=((int)(iVideoTime/60000))%60;
						iHour=((int)(iVideoTime)/3600000)%24;
						StringCchPrintf(totalTime,_countof(totalTime),TEXT("%02d:%02d:%02d/%d"),iHour,iMin,iSec,sih.myiFrame);
						SetDlgItemText(hWnd,IDC_TIME,totalTime);
						StringCchPrintf(startTime,_countof(startTime),TEXT("00:00:00/0"));
						SetDlgItemText(hWnd,IDC_CURTIME,startTime);
						SendDlgItemMessage(hWnd,IDC_SLIDER,TBM_SETRANGEMAX,false,(LPARAM)sih.myiFrame);
						SendDlgItemMessage(hWnd,IDC_SLIDER,TBM_SETRANGEMIN,false,(LPARAM)1); 
				
						m_iCurrentTime=0;
						m_dwCurrentFrame=0;
						m_bPlayEnabled=true;					
					}
					else{
						StringCchPrintf( StatusMessage, cStatusMessageMaxLen, L"Failed to read the file!");
						SetStatusMessage(StatusMessage);
					}
				}
			}

		}
		else if(IDC_START == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			if(m_bPlayEnabled&&(m_dwCurrentFrame<sih.myiFrame))
			{
				
                m_iSliderStepTime=sih.myiTime/sih.myiFrame;   //ms as unit
				SetTimer (hWnd,1,m_iSliderStepTime, NULL) ;
			    m_bSliderMoveEnabled=true;

			}
			
		}
		else if(IDC_PAUSE == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam)) 
		{
			m_bSliderMoveEnabled=false;
		}
		else if (IDC_STOP == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam)) 
		{
			if(!m_bSelectBegin)
			{
				m_bSliderMoveEnabled=false;

				m_iCurrentTime=0;
				m_dwCurrentFrame=0;
				StringCchPrintf(currentTime,_countof( currentTime),TEXT("00:00:00/0"));
				SetDlgItemText(hWnd,IDC_CURTIME,currentTime);
				SendDlgItemMessage(hWnd,IDC_SLIDER,TBM_SETPOS,true,(LPARAM)m_dwCurrentFrame);
				//SetFilePointer(skdfile,sfh.myfOffBits,NULL,FILE_BEGIN);
				fseek(skdfile, sfh.myfOffBits, SEEK_SET);
			}
			else{
				m_bSelectBegin=false;
				m_bSliderMoveEnabled=false;
				SetStatusMessage(L"");
			}
			
		}
		else if (IDC_BEGIN == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam)) 
		{
			m_bSelectBegin=true;
			m_dwSelectBeginFrame=m_dwCurrentFrame;
			m_iSelectBeginTime=m_iCurrentTime;
			StringCchPrintf( StatusMessage, cStatusMessageMaxLen, L"Selected begin frame:%d!",m_dwSelectBeginFrame);
			SetStatusMessage(StatusMessage);
		}
		else if (IDC_END == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam)) 
		{
			if(m_bSelectBegin)
			{
				KillTimer(hWnd,1);
                m_bSelectBegin=false;
				m_dwSelectEndFrame=m_dwCurrentFrame;
				m_iSelectEndTime=m_iCurrentTime;
				dwWrittenFrameNum=m_dwSelectEndFrame-m_dwSelectBeginFrame+1;
				StringCchPrintf( StatusMessage, cStatusMessageMaxLen, L"Selected end frame:%d!",m_dwSelectEndFrame);
				SetStatusMessage(StatusMessage);
				
				//SetFilePointer(skdfile,sfh.myfOffBits+(m_dwSelectBeginFrame-1)*sizeof(SKDPOINT)*20,NULL,FILE_BEGIN);
				fseek(skdfile, sfh.myfOffBits+(m_dwSelectBeginFrame-1)*sizeof(SKDPOINT)*20, SEEK_SET);
				ZeroMemory(&ofn,sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = TEXT('\0');
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = TEXT("skd\0*.skd\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.hwndOwner =hWnd;
				ofn.lpstrDefExt=TEXT("skd");
				ofn.Flags = OFN_EXPLORER |OFN_PATHMUSTEXIST;
				if (GetSaveFileName(&ofn))
				{		
					StringCchPrintfW(skdname, _countof(skdname),szFile);
					
					m_hOutputFile=CreateFileW(skdname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					SetFilePointer(m_hOutputFile,sizeof(SKEFILEHEADER)+sizeof(SKEINFORHEADER),NULL,FILE_CURRENT);
					

					while(dwFrameCount<dwWrittenFrameNum)
					{
						for(int i=0;i<20;i++)
						{
                             //ReadFile(skdfile,&tSkeCordBuf,sizeof(tSkeCordBuf),&bytesread,NULL);
							fread(&tSkeCordBuf,sizeof(tSkeCordBuf), 1, skdfile);
							 if(!WriteFile(m_hOutputFile,&tSkeCordBuf,sizeof(tSkeCordBuf),&dwBytesWritten,NULL))
							 {
								 bOutputFileSuccess=false;
								 break;
							 }

						}
                       dwFrameCount++;
					   
					}

					SetFilePointer(m_hOutputFile,0,NULL,FILE_BEGIN);
					WriteFile(m_hOutputFile,&sfh,sizeof(sfh),&dwBytesWritten,NULL);
					tsih.myiTime=m_iSelectEndTime-m_iSelectBeginTime;
					tsih.myiFrame=m_dwSelectEndFrame-m_dwSelectBeginFrame+1;
					tsih.jointNum=20;
					tsih.dimensionNum=5;
					tsih.myiSize=sizeof(tsih);
					tsih.myiAllsize=GetFileSize(m_hOutputFile,&highfilesize);     //Size of the whole skd file
					if(highfilesize)                                     //If the file size is larger than 2^32, set it to 0 to indicate an error!
						tsih.myiAllsize=0;
					WriteFile(m_hOutputFile,&tsih,sizeof(tsih),&dwBytesWritten,NULL);

					DEBUG("Sizeof tsih:%d\n",sizeof(tsih));
					if(bOutputFileSuccess)
					{
						StringCchPrintf( StatusMessage, cStatusMessageMaxLen, L"File  saved successfully:%.3f s, %d frames",tsih.myiTime/1000.0,tsih.myiFrame);
						SetStatusMessage(StatusMessage);
					}
					else{
						SetStatusMessage(L"Failed to save the selected frames!");
					}
				    CloseHandle(m_hOutputFile);
				}
				//SetFilePointer(skdfile,sfh.myfOffBits+m_dwCurrentFrame*sizeof(SKDPOINT)*20,NULL,FILE_BEGIN);
				fseek(skdfile, sfh.myfOffBits+m_dwCurrentFrame*sizeof(SKDPOINT)*20, SEEK_SET);
				SetTimer (hWnd,1,m_iSliderStepTime, NULL) ;
			}
			
		}
		else if (IDC_CHECK_AUTOPLAY == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
		{
			m_bAutoPlayDisabled=!m_bAutoPlayDisabled;
			if(m_bAutoPlayDisabled)
			{
                Button_Enable(GetDlgItem( hWnd, IDC_START),false);
				Button_Enable(GetDlgItem( hWnd, IDC_BEGIN),false);
				Button_Enable(GetDlgItem( hWnd, IDC_END),false);
				Button_Enable(GetDlgItem( hWnd, IDC_STOP),false);
				KillTimer(hWnd,1);
			}
			
			else {
				Button_Enable(GetDlgItem( hWnd, IDC_START),true);
				Button_Enable(GetDlgItem( hWnd, IDC_BEGIN),true);
				Button_Enable(GetDlgItem( hWnd, IDC_END),true);
				Button_Enable(GetDlgItem( hWnd, IDC_STOP),true);
				SetTimer (hWnd,1,m_iSliderStepTime, NULL) ;
			}

		}
		break;
	case WM_TIMER:
		if(m_bSliderMoveEnabled)
		{
			m_iCurrentTime+=m_iSliderStepTime;
			m_dwCurrentFrame++;
			iSec=((int)(m_iCurrentTime/1000))%60;
			iMin=((int)(m_iCurrentTime/60000))%60;
			iHour=((int)(m_iCurrentTime)/3600000)%24;
			StringCchPrintf(currentTime,_countof( currentTime),TEXT("%02d:%02d:%02d/%d"),iHour,iMin,iSec,m_dwCurrentFrame);
			SetDlgItemText(hWnd,IDC_CURTIME,currentTime);

			//Play the current frame
			ProcessSkeleton(hWnd);

			//Move the slider forward by one step
			SendDlgItemMessage(hWnd,IDC_SLIDER,TBM_SETPOS,true,(LPARAM)m_dwCurrentFrame);

			if(m_dwCurrentFrame==sih.myiFrame)
				m_bSliderMoveEnabled=false;
		}
		break;
    HANDLE_MSG(hWnd, WM_HSCROLL, Main_OnHScroll);
	}

	return FALSE;
}

void CSkeletonPlayer::Main_OnHScroll(HWND hwnd,HWND hwndCtl,UINT nSBCode,UINT nPos)
{
	int iSec,iMin,iHour;
	TCHAR   currentTime[256];
	bool flag=true;
	if(nSBCode==SB_THUMBTRACK)
	{
		KillTimer(hwnd,1);
		m_dwCurrentFrame=(DWORD)nPos;
		m_iCurrentTime=m_iSliderStepTime*m_dwCurrentFrame;
		iSec=((int)(m_iCurrentTime/1000))%60;
		iMin=((int)(m_iCurrentTime/60000))%60;
		iHour=((int)(m_iCurrentTime)/3600000)%24;
		StringCchPrintf(currentTime,_countof( currentTime),TEXT("%02d:%02d:%02d/%d"),iHour,iMin,iSec,m_dwCurrentFrame);
		SetDlgItemText(hwnd,IDC_CURTIME,currentTime);
		//SetFilePointer(skdfile,sfh.myfOffBits+(m_dwCurrentFrame-1)*sizeof(SKDPOINT)*20,NULL,FILE_BEGIN);
		fseek(skdfile, sfh.myfOffBits+(m_dwCurrentFrame-1)*sizeof(SKDPOINT)*20, SEEK_SET);
		ProcessSkeleton(hwnd);
	}
	else if(nSBCode==SB_THUMBPOSITION)
	{
		m_dwCurrentFrame=(DWORD)nPos;
		m_iCurrentTime=m_iSliderStepTime*m_dwCurrentFrame;
		//SetFilePointer(skdfile,sfh.myfOffBits+(m_dwCurrentFrame-1)*sizeof(SKDPOINT)*20,NULL,FILE_BEGIN);
		fseek(skdfile, sfh.myfOffBits+(m_dwCurrentFrame-1)*sizeof(SKDPOINT)*20, SEEK_SET);
		SetTimer (hwnd,1,m_iSliderStepTime, NULL) ;
	}
	else if((nSBCode==SB_LINERIGHT)||(nSBCode==SB_LINELEFT))
	{
		TCHAR playStep[3];
		DWORD dwPlayStep;
		WORD wPlayStepNum=(WORD) SendDlgItemMessage(hwnd,IDC_PLAY_STEP, EM_LINELENGTH, (WPARAM) 0, (LPARAM) 0); 
		if (wPlayStepNum >= 3) 
		{ 
			MessageBox(hwnd, L"Too many characters!The number of characters should not be more than 3!", 
				L"Error", 
				MB_OK); 
			flag=false;
		} 
		else if (wPlayStepNum == 0) 
		{ 
			MessageBox(hwnd, L"No characters entered.", 
				L"Error", 
				MB_OK);
			flag=false;
		} 
		
        SendDlgItemMessage(hwnd,IDC_PLAY_STEP,EM_GETLINE, 
				(WPARAM) 0,       // line 0 
				(LPARAM) playStep);
        dwPlayStep=::StrToInt(playStep);

		SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_SETPAGESIZE, 0, (LPARAM)dwPlayStep);		
		SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_SETLINESIZE, 0, (LPARAM)dwPlayStep);
		
		    m_dwCurrentFrame=(int)SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_GETPOS, 0,0);
		    m_iCurrentTime=m_iSliderStepTime*m_dwCurrentFrame;
			
			iSec=((int)(m_iCurrentTime/1000))%60;
			iMin=((int)(m_iCurrentTime/60000))%60;
			iHour=((int)(m_iCurrentTime)/3600000)%24;
			StringCchPrintf(currentTime,_countof( currentTime),TEXT("%02d:%02d:%02d/%d"),iHour,iMin,iSec,m_dwCurrentFrame);
			SetDlgItemText(hwnd,IDC_CURTIME,currentTime);
			//SetFilePointer(skdfile,sfh.myfOffBits+(m_dwCurrentFrame-1)*sizeof(SKDPOINT)*20,NULL,FILE_BEGIN);
			fseek(skdfile, sfh.myfOffBits+(m_dwCurrentFrame-1)*sizeof(SKDPOINT)*20, SEEK_SET);
			ProcessSkeleton(hwnd);
		
		
	}
	
}



/// <summary>
/// Handle new skeleton data
/// </summary>
void CSkeletonPlayer::ProcessSkeleton(HWND hwnd)
{
	NUI_SKELETON_FRAME skeletonFrame = {0};
	SKDPOINT skeletonBuf[20];
	Vector4 skeletonPoint[20];
//	DWORD bytesread;
	HRESULT hr;
	SKDPOINT *pSkeBuf;
	

	for(int i=0;i<20;i++)
	{
		pSkeBuf=skeletonBuf+i;
      // ReadFile(skdfile,pSkeBuf,sizeof(SKDPOINT),&bytesread,NULL);
		fread(pSkeBuf,sizeof(SKDPOINT),1, skdfile);
	   (skeletonPoint+i)->x=static_cast<float>((pSkeBuf->x)/100.0);
	   (skeletonPoint+i)->y=static_cast<float>((pSkeBuf->y)/100.0);
	   (skeletonPoint+i)->z=static_cast<float>((pSkeBuf->z)/100.0);
	   (skeletonPoint+i)->w=static_cast<float>((pSkeBuf->w)/100.0);
	   
	}
	
	
	// Endure Direct2D is ready to draw
	hr = EnsureDirect2DResources( );
	if ( FAILED(hr) )
	{
		return;
	}

	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear( );

	RECT rct;
	GetClientRect( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), &rct);
	int width = rct.right;
	int height = rct.bottom;

	
			// We're tracking the skeleton, draw it
			DrawSkeleton(skeletonPoint, width, height);
		
	

	hr = m_pRenderTarget->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
		DiscardDirect2DResources();
	}

	//Show the frame rate
	m_iCurrentFrameRate=1000/(skeletonBuf[0].t);
	
	
	::SetDlgItemInt(hwnd, static_cast<int>(IDC_FPS),m_iCurrentFrameRate, FALSE );
	
}

/// <summary>
/// Draws a skeleton
/// </summary>
/// <param name="skel">skeleton to draw</param>
/// <param name="windowWidth">width (in pixels) of output buffer</param>
/// <param name="windowHeight">height (in pixels) of output buffer</param>
void CSkeletonPlayer::DrawSkeleton(Vector4 *pSkel, int windowWidth, int windowHeight)
{      
	int i;

	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		m_Points[i] = SkeletonToScreen(*(pSkel+i), windowWidth, windowHeight);
	}

	// Render Torso
	DrawBone(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	DrawBone(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	DrawBone(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	DrawBone(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	DrawBone(NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
	DrawBone(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	DrawBone(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	// Left Arm
	DrawBone(NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	DrawBone(NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	DrawBone(NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	// Right Arm
	DrawBone(NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	DrawBone(NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	DrawBone(NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	// Left Leg
	DrawBone(NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	DrawBone(NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	DrawBone(NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	// Right Leg
	DrawBone(NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	DrawBone(NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	DrawBone(NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	// Draw the joints in a different color
	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse( m_Points[i], g_JointThickness, g_JointThickness );

		
			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
		
	}
}

/// <summary>
/// Draws a bone line between two joints
/// </summary>
/// <param name="skel">skeleton to draw bones from</param>
/// <param name="joint0">joint to start drawing from</param>
/// <param name="joint1">joint to end drawing at</param>
void CSkeletonPlayer::DrawBone(NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
{
	

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	
		m_pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], m_pBrushBoneTracked, g_TrackedBoneThickness);
	
	
}

/// <summary>
/// Converts a skeleton point to screen space
/// </summary>
/// <param name="skeletonPoint">skeleton point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F CSkeletonPlayer::SkeletonToScreen(Vector4 skeletonPoint, int width, int height)
{
	LONG x, y;
	USHORT depth;

	// Calculate the skeleton's position on the screen
	// NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
	NuiTransformSkeletonToDepthImage(skeletonPoint, &x, &y, &depth);

	float screenPointX = static_cast<float>(x * width) / cScreenWidth;
	float screenPointY = static_cast<float>(y * height) / cScreenHeight;

	return D2D1::Point2F(screenPointX, screenPointY);
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT CSkeletonPlayer::EnsureDirect2DResources()
{
	HRESULT hr = S_OK;

	// If there isn't currently a render target, we need to create one
	if (NULL == m_pRenderTarget)
	{
		RECT rc;
		GetWindowRect( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), &rc );  

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;
		D2D1_SIZE_U size = D2D1::SizeU( width, height );
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		// Create a Hwnd render target, in order to render to the window set in initialize
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(GetDlgItem( m_hWnd, IDC_VIDEOVIEW), size),
			&m_pRenderTarget
			);
		if ( FAILED(hr) )
		{
			SetStatusMessage(L"Couldn't create Direct2D render target!");
			return hr;
		}

		//light green
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(68, 192, 68), &m_pBrushJointTracked);

		//yellow
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(255, 255, 0), &m_pBrushJointInferred);

		//green
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 128, 0), &m_pBrushBoneTracked);

		//gray
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(128, 128, 128), &m_pBrushBoneInferred);
	}

	return hr;
}

/// <summary>
/// Dispose Direct2d resources 
/// </summary>
void CSkeletonPlayer::DiscardDirect2DResources( )
{
	SafeRelease(m_pRenderTarget);

	SafeRelease(m_pBrushJointTracked);
	SafeRelease(m_pBrushJointInferred);
	SafeRelease(m_pBrushBoneTracked);
	SafeRelease(m_pBrushBoneInferred);
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void CSkeletonPlayer::SetStatusMessage(WCHAR * szMessage)
{
	SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}