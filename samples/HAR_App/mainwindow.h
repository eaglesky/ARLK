#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef _NITE_
#define _NITE_
#endif

#include <QDebug>
#include <QMainWindow>
#include <QLabel>
#include <QMessageBox>
#include "QKinectWrapper.h"
#include "ARLK/ARLK.h"
#include "setparameterdlg.h"
#include <XnFPSCalculator.h>
#include <vector>
#define MAX_ACTION_NUM 50



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
   void kinectData();
   void kinectPredictData();
   void kinectStatus(QKinect::KinectStatus);
   void kinectUser(unsigned,bool);
   void kinectPose(unsigned,QString);
   void kinectCalibration(unsigned,QKinect::CalibrationStatus);

   void SVMTrain();
   void KNNTrain();
private slots:
   void on_getDataButton_clicked();

   void on_actionExit_triggered();

   void on_actionLoad_data_triggered();

   void on_actionParameters_triggered();

   void on_trainButton_clicked();

   void on_predictButton_clicked();

   void on_actionAbout_triggered();

private:
    void WriteTrainFileLabelStrs();
    void ReadTrainFileLabelStrs();
    Ui::MainWindow *ui;

    int kreaderMode; // 0 is Collecting data and 1 is predicting
    QKinect::QKinectWrapper kreader;
    QLabel *sbKinectStatus;
    QLabel *sbKinectFrame;
    QLabel *sbKinectTime;
    QLabel *sbKinectFPS;
    QLabel *sbKinectNumBody;
    XnFPSData xnFPS;

    int delayTime;

    //If delayTimeDouble < 0, the app begins collecting data
    double delayTimeDouble;

    int delayFrames;
    int maxFrames;
    int curTrueFrame;


  //  ARLK::SkeData* skeTrainDatas;
    ARLK::SkeData* pSkeTrainData;
    ARLK::SkeData* pSkeTrainDatas;
    std::vector<QString> labelNames;
    //double* labelIdx;
    std::vector<double> trainLabelVector;  //zero based
    int trainActionCount;

    _TRAINPARAMS m_trainParams;

    ARLK::SVMClassifier* m_pSVM;
    ARLK::KNNClassifier* m_pKNN;
};

#endif // MAINWINDOW_H
