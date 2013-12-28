//------------------------------------------------------------------------------
// <copyright file="DataCollection.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

//#include "SkeletonFormat.h"


#include "DataCollection.h"
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
    CDataCollection application;
    application.Run(hInstance, nCmdShow);
}

/// <summary>
/// Constructor
/// </summary>
CDataCollection::CDataCollection() :
    m_pD2DFactory(NULL),
    m_pDrawColor(NULL),
    m_hNextColorFrameEvent(INVALID_HANDLE_VALUE),
    m_pColorStreamHandle(INVALID_HANDLE_VALUE),
    
	m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
	m_pSkeletonStreamHandle(INVALID_HANDLE_VALUE),

	m_pRenderTarget(NULL),
	m_pBrushJointTracked(NULL),
	m_pBrushJointInferred(NULL),
	m_pBrushBoneTracked(NULL),
	m_pBrushBoneInferred(NULL),
	
	m_bSaveSkeStart(false),
	m_bSaveSkeStop(true),
	m_bIsInDelay(false),
    m_pNuiSensor(NULL)
{
	m_pSkeData = new ARLK::SkeData;
	// create heap storage for depth pixel data in RGBX format
	ZeroMemory(m_Points,sizeof(m_Points));
}

/// <summary>
/// Destructor
/// </summary>
CDataCollection::~CDataCollection()
{
    if (m_pNuiSensor)
    {
        m_pNuiSensor->NuiShutdown();
    }

    if (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hNextColorFrameEvent);
    }

	

	if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonEvent);
	}

	// clean up Direct2D objects
	DiscardDirect2DResources();

    // clean up Direct2D renderer
    delete m_pDrawColor;
    m_pDrawColor = NULL;

	
	

	// done with depth pixel data
	//delete[] m_depthRGBX;

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    SafeRelease(m_pNuiSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int CDataCollection::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProc;
    wc.lpszClassName = L"DataCollectionAppDlgWndClass";

    if (!RegisterClass(&wc))
    {
        return 0;
    }

    // Create main application window
    HWND hWndApp = CreateDialogParam(
        hInstance,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)CDataCollection::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Show window
    ShowWindow(hWndApp, nCmdShow);

    const int eventCount = 2;
	HANDLE hEvents[eventCount]={m_hNextColorFrameEvent, m_hNextSkeletonEvent};

	
    // Main message loop
    while (WM_QUIT != msg.message)
    {
        

        // Check to see if we have either a message (by passing in QS_ALLINPUT)
        // Or a Kinect event (hEvents)
        // Update() will check for Kinect events individually, in case more than one are signalled
        DWORD dwEvent = MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);
	    
        // Check if this is an event we're waiting on and not a timeout or message
        if ((WAIT_OBJECT_0 == dwEvent)||(WAIT_OBJECT_0+1)==dwEvent)
        {
			
            Update();
        }
		
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if ((hWndApp != NULL) && IsDialogMessage(hWndApp, &msg))
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
/// Main processing function
/// </summary>
void CDataCollection::Update()
{
    if (NULL == m_pNuiSensor)
    {
        return;
    }

    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextColorFrameEvent, 0) )
    {
        ProcessColor();
    }
	
	else if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0) )
	{
		ProcessSkeleton();
	}
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CDataCollection::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDataCollection* pThis = NULL;
    CDataCollection* pThis2 = NULL;
    if (WM_INITDIALOG == uMsg)
    {
        pThis2=pThis = reinterpret_cast<CDataCollection*>(lParam);
		
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<CDataCollection*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
LRESULT CALLBACK CDataCollection::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int chour,cmin,csec;
	double dTimeInSeconds;
	WORD wDelayCharNum;
	TCHAR DelayTime[3];
	TCHAR ConstraintBuf[10];
	BROWSEINFO   bf;   
    LPITEMIDLIST   lpitem;   
	WCHAR comboItem[MAX_PATH];
//	WCHAR fileSavedMessage[50];

    switch (message)
    {
        case WM_INITDIALOG:
        {
			LOGFONT lf;
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Create and initialize a new Direct2D image renderer (take a look at ImageRenderer.h)
            // We'll use this to draw the data we receive from the Kinect to the screen
            m_pDrawColor = new ImageRenderer();
			//m_pDrawDepth = new ImageRenderer();

            HRESULT hr = m_pDrawColor->Initialize(GetDlgItem(m_hWnd, IDC_COLOR), m_pD2DFactory, cColorWidth, cColorHeight, cColorWidth * sizeof(long));
            if (FAILED(hr))
            {
                SetStatusMessage(L"Failed to initialize the Direct2D draw device.");
            }

            // Look for a connected Kinect, and create it if found
            CreateFirstConnected();

			if(!SetCurrentDirectory(L"..\\output"))
				CreateDirectory(L"..\\output",NULL);
			
			//Set the font for recording time display
			GetObject( (HFONT) GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf );
			lf.lfHeight *= 3;
			SendDlgItemMessage( hWnd, IDC_SEC, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );
			SendDlgItemMessage( hWnd, IDC_MIN, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );
			SendDlgItemMessage( hWnd, IDC_HOUR, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );
			SendDlgItemMessage( hWnd, IDC_FRAME_SHOW, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );
			SendDlgItemMessage( hWnd, IDC_SEP, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );
			SendDlgItemMessage( hWnd, IDC_TIME_COUNT, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0 );

			SetWindowText(GetDlgItem(hWnd,IDC_DELAY),L"0");

			StringCchPrintfW(comboItem, _countof(comboItem), L"No constraints");
			SendDlgItemMessage(hWnd, IDC_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(comboItem));
			StringCchPrintfW(comboItem, _countof(comboItem), L"Time length(s)");
			SendDlgItemMessage(hWnd, IDC_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(comboItem));
			StringCchPrintfW(comboItem, _countof(comboItem), L"Frame number");
			SendDlgItemMessage(hWnd, IDC_MODE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(comboItem));

			SendDlgItemMessage(hWnd, IDC_MODE, CB_SETCURSEL,0, 0);
			m_iRecordingModeIndex=DEFAULT_MODE;
			
		
			lastframetime=GetTickCount();
        }
        break;

        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Quit the main message pump
		
		
            PostQuitMessage(0);
            break;

        case WM_TIMER:                      //Handle timer message
			if(m_bSaveSkeStart){
			switch (wParam)  
			{  
			case TIMER_SEC:  
			
				currenttime=GetTickCount();
				dTimeInSeconds=(currenttime-starttime)/1000.0;
				csec=((currenttime-starttime)/1000)%60;
				cmin=((currenttime-starttime)/60000)%60;
				chour=((currenttime-starttime)/3600000)%24;
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_SEC),csec, FALSE );
                ::SetDlgItemInt(hWnd, static_cast<int>(IDC_MIN),cmin, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_HOUR),chour, FALSE );

				
				break ;  
		
			} 
			}
			else if (m_bIsInDelay){
				m_iDelayTime--;
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_TIME_COUNT),m_iDelayTime, FALSE );
				if(m_iDelayTime==0)
				{
					//DEBUG("Start!\n");
					
				    USES_CONVERSION;
				    const char* ascName = T2A(skefname);
					
					m_pSkeData->SaveStart(ascName,m_iFrameConstraint,m_dwTimeConstraint);
					m_bIsInDelay = false;
					m_bSaveSkeStart = true;
					starttime = GetTickCount();
					
				}
				else{
					m_bIsInDelay = true;
				}
			}
			break;

        // Handle button press
        case WM_COMMAND:
            
			

			// If it was for the start control and a button clicked event, start saving the skeleton frame
			if(IDC_START== LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
			{
                TCHAR timeString[MAX_PATH];
				WORD wSavePathNum;
                
			    m_dwTimeConstraint = 0;
				m_iFrameConstraint = 0;
				
				GetTimeFormatEx(NULL,TIME_FORCE24HOURFORMAT, NULL, L"hh'-'mm'-'ss", timeString, _countof(timeString));
				wSavePathNum=(WORD) SendDlgItemMessage(hWnd,IDC_SAVE_PATH, EM_LINELENGTH, (WPARAM) 0, (LPARAM) 0);
				if(wSavePathNum==0)
				{
                    StringCchPrintfW(skefname, _countof(skefname), L"..\\output\\KinectSkeletonVideo-%s.skd",timeString);
				}
				else{
					StringCchPrintfW(skefname, _countof(skefname), L"%s\\KinectSkeletonVideo-%s.skd",m_SkeFilePath,timeString);
				}

				//hskf=CreateFile(skefname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				//SetFilePointer(hskf,sizeof(SKEFILEHEADER)+sizeof(SKEINFORHEADER),NULL,FILE_CURRENT);
				wDelayCharNum=(WORD) SendDlgItemMessage(hWnd,IDC_DELAY, EM_LINELENGTH, (WPARAM) 0, (LPARAM) 0); 
				if (wDelayCharNum >= 3) 
				{ 
					MessageBox(hWnd, L"Too many characters!The number of characters should not be more than 3!", 
						L"Error", 
						MB_OK); 
					break;
				} 
				else if (wDelayCharNum == 0) 
				{ 
					MessageBox(hWnd, L"No characters entered.", 
						L"Error", 
						MB_OK);
					break;
				} 
				SendDlgItemMessage(hWnd,IDC_DELAY,EM_GETLINE, 
					(WPARAM) 0,       // line 0 
					(LPARAM) DelayTime);

				m_iDelayTime=::StrToInt(DelayTime);
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_TIME_COUNT),m_iDelayTime, FALSE );
				

                SendDlgItemMessage(hWnd,IDC_CONSTRAINT,EM_GETLINE, 
						(WPARAM) 0,       // line 0 
						(LPARAM) ConstraintBuf);

				if(m_iRecordingModeIndex==TIME_MODE)
				{
					m_dwTimeConstraint=::StrToInt(ConstraintBuf);
				}
				else if(m_iRecordingModeIndex==FRAME_MODE)
				{
					m_iFrameConstraint=::StrToInt(ConstraintBuf);
				}

				::SetDlgItemInt(hWnd, static_cast<int>(IDC_SEC),0, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_MIN),0, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_HOUR),0, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_FRAME_SHOW),0, FALSE );
				 
				if(m_iDelayTime==0)
				{
					//DEBUG("Start!\n");
					m_pSkeData->SaveStart((const char*)skefname,m_iFrameConstraint,m_dwTimeConstraint);
					m_bSaveSkeStart = true;
					

				} else {
				   
					m_bIsInDelay = true;
				}
				SetTimer (hWnd, TIMER_SEC, 1000, NULL) ;
			}

			// If it was for the stop control and a button clicked event, stop saving the skeleton frame
			if(IDC_STOP== LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
			{
				if(m_bSaveSkeStart)
				{
					KillTimer (hWnd, TIMER_SEC) ;
					m_bSaveSkeStart=false;
				    m_pSkeData->SaveEnd();
					//DEBUG("Stop!\n");
				}
				else if(m_bIsInDelay){
					KillTimer (hWnd, TIMER_SEC) ;
					::SetDlgItemInt(hWnd, static_cast<int>(IDC_TIME_COUNT),0, FALSE );
                   // CloseHandle(hskf);
					DeleteFile(skefname);
					
				}
				
					
			}

			if(IDC_BROWSE== LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
			{
				memset(&bf,0,sizeof(BROWSEINFO));   
                bf.hwndOwner=hWnd;   
                bf.lpszTitle=L"Choose the directory";   
                bf.ulFlags=BIF_RETURNONLYFSDIRS;    
                lpitem=SHBrowseForFolder(&bf);
				SHGetPathFromIDList(lpitem,m_SkeFilePath); 
				SetDlgItemText(hWnd,IDC_SAVE_PATH,m_SkeFilePath);
				

			}

			// If it was for the reset time control and a button clicked event, reset the time displayed to 0:0:0
			if(IDC_Reset== LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
			{
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_SEC),0, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_MIN),0, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_HOUR),0, FALSE );
				::SetDlgItemInt(hWnd, static_cast<int>(IDC_FRAME_SHOW),0, FALSE );
			}

			// If there is a change of selection in the combobox
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				 m_iRecordingModeIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, 
					(WPARAM) 0, (LPARAM) 0);
				// DEBUG("Current mode:%d\n",m_iRecordingModeIndex);
				 if(m_iRecordingModeIndex==TIME_MODE)
				 {
					 SetWindowText(GetDlgItem(hWnd,IDC_CONSTRAINT),L"5");
				 }
				 else if(m_iRecordingModeIndex==FRAME_MODE)
				 {
					 SetWindowText(GetDlgItem(hWnd,IDC_CONSTRAINT),L"100");
				 }
				 else{
					 SetWindowText(GetDlgItem(hWnd,IDC_CONSTRAINT),L"");
				 }
			}
			
            break;
    }

    return FALSE;
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT CDataCollection::CreateFirstConnected()
{
    INuiSensor * pNuiSensor;
    HRESULT hr,hr3;

    int iSensorCount = 0;
    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_pNuiSensor = pNuiSensor;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }

    if (NULL != m_pNuiSensor)
    {
        // Initialize the Kinect and specify that we'll be using color and skeleton
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_SKELETON); 
        if (SUCCEEDED(hr))
        {
            // Create an event that will be signaled when color data is available
            m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			// Create an event that will be signaled when depth data is available
			//m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			// Create an event that will be signaled when skeleton data is available
			m_hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            // Open a color image stream to receive color frames
            hr = m_pNuiSensor->NuiImageStreamOpen(
                NUI_IMAGE_TYPE_COLOR,
                NUI_IMAGE_RESOLUTION_640x480,
                0,
                2,
                m_hNextColorFrameEvent,
                &m_pColorStreamHandle);
			// Open a depth image stream to receive depth frames
		/*	hr2 = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_DEPTH,
				NUI_IMAGE_RESOLUTION_640x480,
				0,
				2,
				m_hNextDepthFrameEvent,
				&m_pDepthStreamHandle);*/

			// Open a skeleton stream to receive skeleton data
			hr3 = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0); 
        }
    }

    if (NULL == m_pNuiSensor || FAILED(hr) || FAILED(hr3))
    {
        SetStatusMessage(L"No ready Kinect found!");
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Get the name of the file where screenshot will be stored.
/// </summary>
/// <param name="screenshotName">
/// [out] String buffer that will receive screenshot file name.
/// </param>
/// <param name="screenshotNameSize">
/// [in] Number of characters in screenshotName string buffer.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetScreenshotFileName(wchar_t *screenshotName, UINT screenshotNameSize,int shotType)
{
   wchar_t *knownPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &knownPath);
	
      knownPath=L"..\\output";
    if (SUCCEEDED(hr))
    {
        // Get the time
        wchar_t timeString[MAX_PATH];
        GetTimeFormatEx(NULL,TIME_FORCE24HOURFORMAT, NULL, L"hh'-'mm'-'ss", timeString, _countof(timeString));

		// File name will be KinectColorShot-HH-MM-SS(shotType==0)
		//                or KinectDepthShot-HH-MM-SS(shotType==1)
        if(shotType==0)
        StringCchPrintfW(screenshotName, screenshotNameSize, L"%s\\KinectColorShot-%s.bmp", knownPath, timeString);
		else if(shotType==1)
        StringCchPrintfW(screenshotName, screenshotNameSize, L"%s\\KinectDepthShot-%s.bmp", knownPath, timeString);
    }

   // CoTaskMemFree(knownPath);
    return hr;
}

/// <summary>
/// Handle new color data
/// </summary>
/// <returns>indicates success or failure</returns>
void CDataCollection::ProcessColor()
{
    HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;
	//DEBUG("Process color!\n");
    // Attempt to get the color frame
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        return;
    }
	

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        // Draw the data with Direct2D
        m_pDrawColor->Draw(static_cast<BYTE *>(LockedRect.pBits), LockedRect.size);

      
        
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);
}



/// <summary>
/// Handle new skeleton data
/// </summary>
void CDataCollection::ProcessSkeleton()
{
	NUI_SKELETON_FRAME skeletonFrame = {0};

	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);

	
	if(m_bSaveSkeStart)
	{
		int framecount = m_pSkeData->GetFrameSaved();
		::SetDlgItemInt(m_hWnd, static_cast<int>(IDC_FRAME_SHOW),framecount, FALSE );
	}
	
	if ( FAILED(hr) )
	{
		return;
	}

	// smooth out the skeleton data
	m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

	//m_pSkeData->AddOneFrame(skeletonFrame);
	for (int i = 0 ; i < NUI_SKELETON_COUNT; i++)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;
		if (NUI_SKELETON_TRACKED ==trackingState)
		{
			m_pSkeData->AddOneFrame(skeletonFrame.SkeletonData[i].SkeletonPositions);
			break;
		}
	}

	if(m_bSaveSkeStart) {
	if (m_pSkeData->SavingIsStopped()) {
	
	    KillTimer (m_hWnd, TIMER_SEC) ;
	    StringCchPrintf( statusmsg, cStatusMessageMaxLen, L"Skeleton video saved to %s", skefname);
	    MessageBox(m_hWnd, statusmsg, L"File saved!", MB_OK); 

		
		m_bSaveSkeStart=false;
	}
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
	GetClientRect( GetDlgItem( m_hWnd, IDC_SKELETON ), &rct);
	int width = rct.right;
	int height = rct.bottom;
//	WCHAR statusMessage[cStatusMessageMaxLen];
	
	
	int i;
	//currentframetime=GetTickCount();
	for ( i = 0 ; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;
		

		if (NUI_SKELETON_TRACKED == trackingState)
		{
			// We're tracking the skeleton, draw it
			DrawSkeleton(skeletonFrame.SkeletonData[i], width, height);
			break;
		}
		else if (NUI_SKELETON_POSITION_ONLY == trackingState)
		{
	
			
			// we've only received the center point of the skeleton, draw that
			D2D1_ELLIPSE ellipse = D2D1::Ellipse(
				SkeletonToScreen(skeletonFrame.SkeletonData[i].Position, width, height),
				g_JointThickness,
				g_JointThickness
				);

			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
		}

		

		
	}
	
	
	hr = m_pRenderTarget->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
		DiscardDirect2DResources();
	}
	
}



/// <summary>
/// Draws a skeleton
/// </summary>
/// <param name="skel">skeleton to draw</param>
/// <param name="windowWidth">width (in pixels) of output buffer</param>
/// <param name="windowHeight">height (in pixels) of output buffer</param>
void CDataCollection::DrawSkeleton(const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight)
{      
	int i;

	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		m_Points[i] = SkeletonToScreen(skel.SkeletonPositions[i], windowWidth, windowHeight);
	}

	// Render Torso
	DrawBone(skel, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	DrawBone(skel, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	// Left Arm
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	// Right Arm
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	// Left Leg
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	// Right Leg
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	// Draw the joints in a different color
	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse( m_Points[i], g_JointThickness, g_JointThickness );

		if ( skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED )
		{
			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointInferred);
		}
		else if ( skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED )
		{
			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
		}
	}
}

/// <summary>
/// Draws a bone line between two joints
/// </summary>
/// <param name="skel">skeleton to draw bones from</param>
/// <param name="joint0">joint to start drawing from</param>
/// <param name="joint1">joint to end drawing at</param>
void CDataCollection::DrawBone(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
{
	NUI_SKELETON_POSITION_TRACKING_STATE joint0State = skel.eSkeletonPositionTrackingState[joint0];
	NUI_SKELETON_POSITION_TRACKING_STATE joint1State = skel.eSkeletonPositionTrackingState[joint1];

	// If we can't find either of these joints, exit
	if (joint0State == NUI_SKELETON_POSITION_NOT_TRACKED || joint1State == NUI_SKELETON_POSITION_NOT_TRACKED)
	{
		
		
		return;
	}

	// Don't draw if both points are inferred
	if (joint0State == NUI_SKELETON_POSITION_INFERRED && joint1State == NUI_SKELETON_POSITION_INFERRED)
	{
		
		return;
	}

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if (joint0State == NUI_SKELETON_POSITION_TRACKED && joint1State == NUI_SKELETON_POSITION_TRACKED)
	{
		m_pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], m_pBrushBoneTracked, g_TrackedBoneThickness);
	}
	else
	{
		m_pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], m_pBrushBoneInferred, g_InferredBoneThickness);
	}
}

/// <summary>
/// Converts a skeleton point to screen space
/// </summary>
/// <param name="skeletonPoint">skeleton point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F CDataCollection::SkeletonToScreen(Vector4 skeletonPoint, int width, int height)
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
HRESULT CDataCollection::EnsureDirect2DResources()
{
	HRESULT hr = S_OK;

	// If there isn't currently a render target, we need to create one
	if (NULL == m_pRenderTarget)
	{
		RECT rc;
		GetWindowRect( GetDlgItem( m_hWnd, IDC_SKELETON ), &rc );  

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;
		D2D1_SIZE_U size = D2D1::SizeU( width, height );
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		// Create a Hwnd render target, in order to render to the window set in initialize
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(GetDlgItem( m_hWnd, IDC_SKELETON), size),
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
void CDataCollection::DiscardDirect2DResources( )
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
void CDataCollection::SetStatusMessage(WCHAR * szMessage)
{
    SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}

/// <summary>
/// Save passed in image data to disk as a bitmap
/// </summary>
/// <param name="pBitmapBits">image data to save</param>
/// <param name="lWidth">width (in pixels) of input image data</param>
/// <param name="lHeight">height (in pixels) of input image data</param>
/// <param name="wBitsPerPixel">bits per pixel of image data</param>
/// <param name="lpszFilePath">full file path to output bitmap to</param>
/// <returns>indicates success or failure</returns>
HRESULT CDataCollection::SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath)
{
    DWORD dwByteCount = lWidth * lHeight * (wBitsPerPixel / 8);

    BITMAPINFOHEADER bmpInfoHeader = {0};

    bmpInfoHeader.biSize        = sizeof(BITMAPINFOHEADER);  // Size of the header
    bmpInfoHeader.biBitCount    = wBitsPerPixel;             // Bit count
    bmpInfoHeader.biCompression = BI_RGB;                    // Standard RGB, no compression
    bmpInfoHeader.biWidth       = lWidth;                    // Width in pixels
    bmpInfoHeader.biHeight      = -lHeight;                  // Height in pixels, negative indicates it's stored right-side-up
    bmpInfoHeader.biPlanes      = 1;                         // Default
    bmpInfoHeader.biSizeImage   = dwByteCount;               // Image size in bytes

    BITMAPFILEHEADER bfh = {0};

    bfh.bfType    = 0x4D42;                                           // 'M''B', indicates bitmap
    bfh.bfOffBits = bmpInfoHeader.biSize + sizeof(BITMAPFILEHEADER);  // Offset to the start of pixel data
    bfh.bfSize    = bfh.bfOffBits + bmpInfoHeader.biSizeImage;        // Size of image + headers

    // Create the file on disk to write to
    HANDLE hFile = CreateFileW(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
    // Return if error opening file
    if (NULL == hFile) 
    {
        return E_ACCESSDENIED;
    }

    DWORD dwBytesWritten = 0;
    
    // Write the bitmap file header
	if ( !WriteFile(hFile, &bfh, sizeof(bfh), &dwBytesWritten, NULL) )
	{
		CloseHandle(hFile);
		return E_FAIL;
	}
    
    // Write the bitmap info header
    if ( !WriteFile(hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        return E_FAIL;
    }
    
    // Write the RGB Data
    if ( !WriteFile(hFile, pBitmapBits, bmpInfoHeader.biSizeImage, &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        return E_FAIL;
    }    

    // Close the file
    CloseHandle(hFile);
    return S_OK;
}