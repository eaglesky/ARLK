CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	SET( SRCS
		stdafx.h
		targetver.h
		resource.h
		MyDebug.h
		ParametersDlg.h
		MainModel.h
		DlgProxy.h
		ActionInteract0Dlg.h
		ActionInteract0_h.h
		ActionInteract0.h
		stdafx.cpp
		ParametersDlg.cpp
		MainModel.cpp
		DlgProxy.cpp
		ConsoleWindowsApp.cpp
		ActionInteract0Dlg.cpp
		ActionInteract0.cpp
		ActionInteract0.rc
		res/ActionInteract0.rc2
		ActionInteract0.idl
		)
	
	ADD_DEFINITIONS(-D_AFXDLL)
	SET(CMAKE_MFC_FLAG 1)

	#Absolute path of ARLK include directory
	INCLUDE_DIRECTORIES("E:/ustc/CS/HI/kinect/GradPaper/ARLK_LIB/include")
	
	#Absolute path of Microsoft Kinect SDK inc directory
	INCLUDE_DIRECTORIES("C:/Program Files/Microsoft SDKs/Kinect/v1.6/inc")
	
	INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/$(IntDir))
	
	#Absolute path of ARLK lib directory
	LINK_DIRECTORIES("E:/ustc/CS/HI/kinect/GradPaper/ARLK_LIB/win32/lib")
	
	#Absolute path of Microsoft Kinect SDK lib directory
	LINK_DIRECTORIES("C:/Program Files/Microsoft SDKs/Kinect/v1.6/lib/x86")
	
	ADD_EXECUTABLE(ActionInteract0 WIN32 ${SRCS})
	
	#Link the lib files
	TARGET_LINK_LIBRARIES(ActionInteract0 Kinect10 Shlwapi ANN BaseClassifier
	KNNClassifier SVMClassifier SkeData svm)

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")