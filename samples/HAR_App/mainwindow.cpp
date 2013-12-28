#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cio.h"
#include <stdio.h>
#include <math.h>
#include <QInputDialog>
#include "loaddatadlg.h"
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setup the kinect reader
    kreaderMode = 0;  //kinect reader is collecting data by default
    kreader.setSkeletonPen(QPen(QBrush(Qt::green),10));
    kreader.setTextPen(QPen(QBrush(Qt::red),50));
   // connect(&kreader,SIGNAL(dataNotification()),this,SLOT(kinectData()));
    connect(&kreader,SIGNAL(statusNotification(QKinect::KinectStatus)),this,SLOT(kinectStatus(QKinect::KinectStatus)));
    connect(&kreader,SIGNAL(userNotification(unsigned,bool)),this,SLOT(kinectUser(unsigned,bool)));
    connect(&kreader,SIGNAL(poseNotification(unsigned,QString)),this,SLOT(kinectPose(unsigned,QString)));
    connect(&kreader,SIGNAL(calibrationNotification(unsigned,QKinect::CalibrationStatus)),this,SLOT(kinectCalibration(unsigned,QKinect::CalibrationStatus)));


    sbKinectStatus = new QLabel(statusBar());
    sbKinectFrame = new QLabel(statusBar());
    sbKinectTime = new QLabel(statusBar());
    sbKinectFPS = new QLabel(statusBar());
    sbKinectNumBody = new QLabel(statusBar());
    ui->statusBar->addWidget(sbKinectStatus);
    ui->statusBar->addWidget(sbKinectFrame);
    ui->statusBar->addWidget(sbKinectTime);
    ui->statusBar->addWidget(sbKinectFPS);
    ui->statusBar->addWidget(sbKinectNumBody);

    delayTimeDouble = delayTime = 10;
    ui->delayTimeEdit->setText("10");
    delayFrames = 0;
    maxFrames = 100;
    ui->framesEdit->setText("100");

    trainActionCount = 0;
    pSkeTrainData = NULL;
    pSkeTrainDatas = NULL;
    //labelIdx = new double[MAX_ACTION_NUM];

    m_trainParams.classifier = 0;  // 0 is SVM; 1 is KNN
    m_trainParams.svmParams.C = 10;
    m_trainParams.svmParams.gamma = 0.001;
    m_trainParams.feaWt = 3;
    m_trainParams.knnParams.K = 10;
    m_trainParams.knnParams.eps = 0;

    m_pSVM = NULL;
    m_pKNN = NULL;

}

MainWindow::~MainWindow()
{
    delete ui;
    //delete []labelIdx;
    //delete []skeTrainDatas;
    delete pSkeTrainData;
    delete m_pSVM;
    delete m_pKNN;
}

void MainWindow::kinectData()
{

    //qDebug() << "Kinect Data Slot";
    int delayTimeDisplayed = 0;
    int curFrame = 0;
    if (kreader.isRunning()) {
        if (curTrueFrame < maxFrames) {
            if (delayTimeDouble >= 0) {
               double pastTime = kreader.getTimestamp();
               delayTimeDouble = delayTime - pastTime;
               delayTimeDisplayed = ceil(delayTimeDouble);

               //printf("Past Time: %lf\n", pastTime);
               //printf("Double delay time: %lf\n", delayTimeDouble);
               delayFrames = kreader.getFrameID();
            } else {
                //printf("%lf\n",delayTimeDouble);
               curFrame = kreader.getFrameID() - delayFrames;
               curTrueFrame++;
               sbKinectFrame->setText(QString("Frame Id %1, Frame %2").arg(curFrame).arg(curTrueFrame));
               sbKinectTime->setText(QString("Timestamp %1").arg(kreader.getTimestamp() - delayTime));
               ui->progressBar->setValue(curTrueFrame);
            }


           QImage img = kreader.getDepth();
           ui->label->setPixmap(QPixmap::fromImage(img));

           img = kreader.getCamera();
           if (delayTimeDouble >= 0) {
               //Add the delay time to the depth image
               QPainter painter;
               painter.begin(&img);
               painter.setPen(Qt::red);
               painter.setFont(QFont("Arial", 400));

               //qDebug() << QString::number(delayTimeDisplayed);
               painter.drawText(img.rect(), Qt::AlignCenter, QString::number(delayTimeDisplayed));
               painter.end();
           }
           ui->labelImage->setPixmap(QPixmap::fromImage(img));

           QKinect::Bodies bodies = kreader.getBodies();
           sbKinectNumBody->setText(QString("Body: %1").arg(bodies.size()));

           if (delayTimeDouble < 0) {
               for (int j = 0; j < bodies.size(); j++)
               {
                   if (bodies.at(j).status == QKinect::Tracking) {
                       Vector4 tempVector[NUI_SKELETON_POSITION_COUNT];
                       for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
                       {
                           tempVector[i].w = 1;
                           tempVector[i].x = bodies.at(0).proj_joints[i].X / 1000;
                           tempVector[i].y = bodies.at(0).proj_joints[i].Y / 1000;
                           tempVector[i].z = bodies.at(0).proj_joints[i].Z / 10000;

                           qDebug("Collected Frame: %d, (%f, %f, %f)\n", curTrueFrame, tempVector[i].x, tempVector[i].y, tempVector[i].z);
                       }
                    //   skeTrainDatas[trainActionCount].AddOneFrame(tempVector);
                       pSkeTrainData->AddOneFrame(tempVector);
                       //printf("Add one frame!\n");
                       break;
                   }
               }
           }
           xnFPSMarkFrame(&xnFPS);
           sbKinectFPS->setText(QString("FPS: %1").arg(xnFPSCalc(&xnFPS)));
       } else {
            kreader.stop();
            //skeTrainDatas[trainActionCount].SaveEnd();
            pSkeTrainData->SaveEnd();
            QString defaultName = QString::number(trainActionCount + 1);
            bool ok;
            QString text = QInputDialog::getText(this, tr("Input label name"),
                                                      tr("Label name:"), QLineEdit::Normal,
                                                      defaultName, &ok);
            if (ok && !text.isEmpty()) {

                //labelNames.push_back(text);
                QString trainfileName = QString("./train_data/") + text + QString(".skd");

                QByteArray tempStr = trainfileName.toLocal8Bit();
                const char* trainfileNameStr = tempStr.data();
                //const char* trainfileNameStr = trainfileName.toStdString().c_str();
                qDebug("saved as: %s\n", trainfileNameStr);
               // skeTrainDatas[trainActionCount].WriteFrames2File(trainfileNameStr);
                pSkeTrainData->WriteFrames2File(trainfileNameStr);

            //    labelIdx[trainActionCount] = (double)(trainActionCount + 1);
                trainActionCount++;
            }
            //else
             //   labelNames.push_back(defaultName);


        }
    }
   //printf("Available bodies: ");
  // for(unsigned i=0;i<bodies.size();i++)
  //    if(bodies[i].status == QKinect::Tracking)
        // printf("%d ",i);
   //printf("\n");


 //  printf("Kinect data end\n");
}

void MainWindow::kinectPredictData()
{
   // qDebug() << "kreader mode:" << kreaderMode;
   // qDebug() << "Kinect Predict Data Slot!";
    int delayTimeDisplayed = 0;
    int curFrame = 0;
    if (kreader.isRunning()) {
        if ((curTrueFrame < maxFrames) || (maxFrames == 0)) {

            if (delayTimeDouble >= 0) {
               double pastTime = kreader.getTimestamp();
               delayTimeDouble = delayTime - pastTime;
               delayTimeDisplayed = ceil(delayTimeDouble);

               //printf("Past Time: %lf\n", pastTime);
               //printf("Double delay time: %lf\n", delayTimeDouble);
               delayFrames = kreader.getFrameID();
            } else {
                //printf("%lf\n",delayTimeDouble);
               curFrame = kreader.getFrameID() - delayFrames;
               curTrueFrame++;
               sbKinectFrame->setText(QString("Frame Id %1, Frame %2").arg(curFrame).arg(curTrueFrame));
               sbKinectTime->setText(QString("Timestamp %1").arg(kreader.getTimestamp() - delayTime));

            }


           QImage img = kreader.getDepth();
           ui->label->setPixmap(QPixmap::fromImage(img));

           img = kreader.getCamera();
           if (delayTimeDouble >= 0) {
               //Add the delay time to the depth image
               QPainter painter;
               painter.begin(&img);
               painter.setPen(Qt::red);
               painter.setFont(QFont("Arial", 400));

               //qDebug() << QString::number(delayTimeDisplayed);
               painter.drawText(img.rect(), Qt::AlignCenter, QString::number(delayTimeDisplayed));
               painter.end();
           }


           QKinect::Bodies bodies = kreader.getBodies();
           sbKinectNumBody->setText(QString("Body: %1").arg(bodies.size()));

           if (delayTimeDouble < 0) {

               if (maxFrames > 0)
                  ui->progressBar->setValue(curTrueFrame);

              if ((bodies.size() == 1) && (bodies.at(0).status == QKinect::Tracking))
              {
                  Vector4 tempVector[NUI_SKELETON_POSITION_COUNT];
                  for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
                  {
                      tempVector[i].w = 1;
                      tempVector[i].x = bodies.at(0).proj_joints[i].X / 1000;
                      tempVector[i].y = bodies.at(0).proj_joints[i].Y / 1000;
                      tempVector[i].z = bodies.at(0).proj_joints[i].Z / 10000;

                      //qDebug("Collected Frame: %d, (%f, %f, %f)\n", curTrueFrame, tempVector[i].x, tempVector[i].y, tempVector[i].z);
                  }

                  delete pSkeTrainData;
                  pSkeTrainData = new ARLK::SkeData;
                  pSkeTrainData->SaveStart(NULL, 1, 0);
                  pSkeTrainData->AddOneFrame(tempVector);
                  pSkeTrainData->SaveEnd();

                  int n;
                  double* pPredictLabel;

                  if ((m_trainParams.classifier == 0) && (m_pSVM)) {
                    pPredictLabel = m_pSVM->predict(pSkeTrainData, &n);
                    qDebug() << curTrueFrame << ": SVM Predicted: " << labelNames[(int)(*pPredictLabel)];
                  } else if ((m_trainParams.classifier == 1) && (m_pKNN)) {
                      pPredictLabel = m_pKNN->predict(pSkeTrainData, &n);
                      qDebug() << curTrueFrame << ": KNN Predicted: " << labelNames[(int)(*pPredictLabel)];
                  }



                  QPainter painter;
                  painter.begin(&img);
                  painter.setPen(Qt::red);
                  painter.setFont(QFont("Arial", 50));

                  //qDebug() << QString::number(delayTimeDisplayed);
                  painter.drawText(img.rect(), Qt::AlignBottom, labelNames[(int)(*pPredictLabel)]);
                  painter.end();

              }

           }

           ui->labelImage->setPixmap(QPixmap::fromImage(img));
           xnFPSMarkFrame(&xnFPS);
           sbKinectFPS->setText(QString("FPS: %1").arg(xnFPSCalc(&xnFPS)));
       } else
            kreader.stop();
    }
}

void MainWindow::kinectStatus(QKinect::KinectStatus s)
{

   QString str("Kinect: ");
   qDebug() << "Kinect Status!";
   if(s==QKinect::Idle)
      str += "Idle";
   if(s==QKinect::Initializing)
      str += "Initializing";
   if(s==QKinect::OkRun)
      str += "Running";
   if(s==QKinect::ErrorStop)
      str += "Error";
   sbKinectStatus->setText(str);

   if(s==QKinect::ErrorStop || s==QKinect::OkRun || s==QKinect::Idle)
      ui->getDataButton->setEnabled(true);
   if(s==QKinect::OkRun) {
      if (kreaderMode == 0)
        ui->getDataButton->setText("Stop");
      else
        ui->predictButton->setText("Stop");
   }
   if(s==QKinect::ErrorStop || s==QKinect::Idle) {
      if (kreaderMode == 0)
        ui->getDataButton->setText("Get Data");
      else
        ui->predictButton->setText("Predict");
   }
}

void MainWindow::kinectPose(unsigned id,QString pose)
{
  // printf("User %u: Pose '%s' detected\n",id,pose.toStdString().c_str());
}

void MainWindow::kinectUser(unsigned id, bool found)
{
  // printf("User %u: %s\n",id,found?"found":"lost");
}

void MainWindow::kinectCalibration(unsigned id, QKinect::CalibrationStatus s)
{
  /* printf("User %u: Calibration ",id);
   if(s==QKinect::CalibrationStart)
      printf("started\n");
   if(s==QKinect::CalibrationEndSuccess)
      printf("successful\n");
   if(s==QKinect::CalibrationEndFail)
      printf("failed\n");*/
}



void MainWindow::on_getDataButton_clicked()
{


    ui->getDataButton->setEnabled(false);
    kreaderMode = 0;
    kreader.disconnect(SIGNAL(dataNotification()));
    connect(&kreader,SIGNAL(dataNotification()),this,SLOT(kinectData()));

    if(kreader.isRunning())
    {
       kreader.stop();
    }
    else
    {


           delayTime = ui->delayTimeEdit->text().toInt();
           delayTimeDouble = delayTime;
           //printf("Delay Time = %d\n",delayTime);

           maxFrames = ui->framesEdit->text().toInt();
           curTrueFrame = 0;

           delete pSkeTrainData;
           pSkeTrainData = new ARLK::SkeData;
           //skeTrainDatas[trainActionCount].SaveStart(NULL, maxFrames, 0);
           pSkeTrainData->SaveStart(NULL, maxFrames, 0);

           ui->progressBar->setRange(0, maxFrames);
           ui->progressBar->setValue(0);
           kreader.start();
           xnFPSInit(&xnFPS, 180);

    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionLoad_data_triggered()
{
    LoadDataDlg loadDataDlg(this);
    ui->trainButton->setEnabled(false);
    QDialog::DialogCode retCode = (QDialog::DialogCode)loadDataDlg.exec();

    if (retCode == QDialog::Accepted) {

            qDebug("Load OK!\n");
            int num = loadDataDlg.skdFiles.size();
           // delete []skeTrainDatas;
           // skeTrainDatas = new ARLK::SkeData[num];
            delete pSkeTrainDatas;
            pSkeTrainDatas = new ARLK::SkeData;

            trainLabelVector.clear();
            labelNames.clear();

            QStringList::Iterator it = loadDataDlg.skdFiles.begin();

            for (int i = 0; it != loadDataDlg.skdFiles.end(); i++, it++)
            {
               // const char* fileName = (*it).toStdString().c_str();
                QByteArray tempStr = (*it).toLocal8Bit();
                const char* fileName = tempStr.data();
                qDebug() << "Loaded skd files: " << fileName;
                FILE* fp = fopen(fileName, "rb");
                //skeTrainDatas[i].GetFramesFromFile(fp);
                pSkeTrainDatas->AddFramesFromFile(fp);

                SKEFILEHEADER sfh = {0};
                SKEINFORHEADER sih = {0};
                int curFrameNum = 0;
                if (ARLK::SkeData::GetHeaders(fp, &sfh, &sih))
                      curFrameNum = sih.myiFrame;

                for (int j = 0; j < curFrameNum; j++)
                   trainLabelVector.push_back(i);

                QFileInfo fi(*it);
                QString base = fi.baseName();
                labelNames.push_back(base);
                qDebug() << "Base: " << base << "\n";
                fclose(fp);
            }



    } else {
        qDebug("Load Failed!\n");
        loadDataDlg.skdFiles.clear();
    }

    ui->trainButton->setEnabled(true);
}

void MainWindow::on_actionParameters_triggered()
{
    SetParameterDlg setParameterDlg(this);
    setParameterDlg.init(&m_trainParams);

    QDialog::DialogCode retCode = (QDialog::DialogCode)setParameterDlg.exec();

    if (retCode == QDialog::Accepted) {
        qDebug("Parameter set ok!");
        m_trainParams.feaWt = setParameterDlg.m_trainParams.feaWt;
        m_trainParams.svmParams.C = setParameterDlg.m_trainParams.svmParams.C;
        m_trainParams.svmParams.gamma = setParameterDlg.m_trainParams.svmParams.gamma;
        m_trainParams.knnParams.K = setParameterDlg.m_trainParams.knnParams.K;
        m_trainParams.knnParams.eps = setParameterDlg.m_trainParams.knnParams.eps;

        m_trainParams.classifier = setParameterDlg.m_trainParams.classifier;
    } else {
        qDebug("Parameter set canceled!");
    }
}

void MainWindow::on_trainButton_clicked()
{
    if (!pSkeTrainDatas) {
        QMessageBox msgBox;
        msgBox.setText("Please Load data file first!");
        msgBox.exec();
    } else {
        if (m_trainParams.classifier == 0) { //SVM Training
           SVMTrain();

        } else if (m_trainParams.classifier == 1) { //KNN Training
           KNNTrain();
        }
    }
    WriteTrainFileLabelStrs();

}

void MainWindow::SVMTrain()
{
    qDebug() << "SVM Train Begin!";
    if (m_pSVM)
            delete m_pSVM;

     m_pSVM = new ARLK::SVMClassifier();

    int labelNum = trainLabelVector.size();

    double* labels = new double[labelNum];
    for (int i = 0; i < labelNum; i++)
    {
        labels[i] = trainLabelVector.at(i);
    }

    m_pSVM->SetSVMParameters(m_trainParams.svmParams.C, m_trainParams.svmParams.gamma,
        m_trainParams.feaWt);
    m_pSVM->FeatureExtraction(pSkeTrainDatas, labels, labelNum);
    m_pSVM->train();

    QMessageBox msgBox;
    msgBox.setText("SVM Train finished!");
    msgBox.exec();
    qDebug() << "SVM Train End！";

}

void MainWindow::KNNTrain()
{
    qDebug() << "KNN Train!";

    if (m_pKNN)
            delete m_pKNN;

    m_pKNN = new ARLK::KNNClassifier();

    int labelNum = trainLabelVector.size();

    if (labelNum == 0) {
        QMessageBox msgBox;
        msgBox.setText("Please load data first!");
        msgBox.exec();
    } else {
        double* labels = new double[labelNum];
        for (int i = 0; i < labelNum; i++)
        {
            labels[i] = trainLabelVector.at(i);
        }

        m_pKNN->SetKNNParameters(m_trainParams.knnParams.K, m_trainParams.knnParams.eps,
            m_trainParams.feaWt);

        m_pKNN->FeatureExtraction(pSkeTrainDatas, labels, labelNum);
        m_pKNN->train();

        QMessageBox msgBox;
        msgBox.setText("KNN Train finished!");
        msgBox.exec();
    }

}

void MainWindow::on_predictButton_clicked()
{
    kreaderMode = 1;
    kreader.disconnect(SIGNAL(dataNotification()));
    connect(&kreader,SIGNAL(dataNotification()),this,SLOT(kinectPredictData()));

    if (m_trainParams.classifier == 0) {
        if (!m_pSVM)
                m_pSVM = new ARLK::SVMClassifier();

        if (labelNames.size() == 0)
            ReadTrainFileLabelStrs();

        /*for (int i = 0; i < labelNames.size(); i++)
            {
            qDebug() << "Loaded Label Name: " << labelNames.at(i);
        }*/

    } else if (m_trainParams.classifier == 1) {
        if (!m_pKNN) {
            QMessageBox msgBox;
            msgBox.setText("Please train first!");
            msgBox.exec();
        } else {
            if (labelNames.size() == 0)
                ReadTrainFileLabelStrs();


        }
    }
    if(kreader.isRunning())
    {
       kreader.stop();
    }
    else
    {
       delayTime = ui->delayTimeEdit->text().toInt();
       delayTimeDouble = delayTime;
       //printf("Delay Time = %d\n",delayTime);
       maxFrames = ui->framesEdit->text().toInt();
       curTrueFrame = 0;
       ui->progressBar->setRange(0, maxFrames);
       ui->progressBar->setValue(0);

       kreader.start();
       xnFPSInit(&xnFPS, 180);

    }
}

void MainWindow::WriteTrainFileLabelStrs()
{
    FILE* fp;
    if (fp = fopen("train_file_labels.txt", "wt+"))
    {
        fprintf(fp, "%d\n", (int)labelNames.size());
        for (int i = 0; i < labelNames.size(); i++)
        {
            QByteArray tempStr = labelNames[i].toLocal8Bit();
            const char* fileName = tempStr.data();
            fprintf(fp, "%s\n", fileName);
        }
    }
    fclose(fp);
}

void MainWindow::ReadTrainFileLabelStrs()
{
    FILE* fp;
    int count;
    if (fp = fopen("train_file_labels.txt", "rb+"))
    {
        fscanf(fp, "%d\n", &count);
        for (int i = 0; i < count; i++)
        {
            char labelStr[50];
            fscanf(fp, "%s\n", labelStr);
            QString tempStr(labelStr);
            labelNames.push_back(tempStr);
        }
    }
    fclose(fp);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox;
    msgBox.about(this, "About", "Human Action Recognition application\nVersion: 0.1\nAuthor: Yalun Qin");

}
