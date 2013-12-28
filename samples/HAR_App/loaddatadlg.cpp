#include "loaddatadlg.h"
#include "ui_loaddatadlg.h"
#include <QFileDialog>
#include <QStringListModel>

LoadDataDlg::LoadDataDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadDataDlg)
{
    ui->setupUi(this);
}

LoadDataDlg::~LoadDataDlg()
{
    delete ui;
}

void LoadDataDlg::on_browseButton_clicked()
{
    skdFiles.clear();
    skdFiles = QFileDialog::getOpenFileNames(
            this,
            tr("Open skd files"),
               // QDir::currentPath() ,
                QString("./train_data"),
            tr("Skd files (*.skd)"));

    QStringListModel *model = new QStringListModel();
    model->setStringList(skdFiles);

    ui->listView->setModel(model);
    ui->listView->show();
}

void LoadDataDlg::on_playButton_clicked()
{
    QModelIndexList selectedIndexes = ui->listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() == 0) {
        QMessageBox msgBox;
        msgBox.setText("Please select one file!");
        msgBox.exec();
    } else {
        QByteArray tempStr = selectedIndexes.at(0).data().toString().toLocal8Bit();
        //const char* selectedFileName = selectedIndexes.at(0).data().toString().toStdString().c_str();
        const char* selectedFileName = tempStr.data();
        qDebug("selected file: %s\n", selectedFileName);

        playSkeFile(selectedFileName);
    }

}

void LoadDataDlg::playSkeFile(const char *fileName)
{
    int g_nXRes = 600;
    int g_nYRes = 600;


    ARLK::SkeData skePlayData;
    FILE* skeFile = fopen(fileName, "rb");

    if (skeFile) {
      skePlayData.GetFramesFromFile(skeFile);
      int frameNum = skePlayData.GetFrameSaved();
      qDebug("Total frame number: %d\n", frameNum);
      ui->playProgressBar->setRange(0, frameNum);
      ui->playProgressBar->setValue(0);

      clock_t sleepTime = 30;
      for (int i = 0; i < frameNum; i++)
      {
          XnPoint3D skdBody[15];
          for (int j = 0; j < 15; j++)
          {
            SKDPOINT* bodyJt = skePlayData.GetPoint(i+1, j+1);
            skdBody[j].X = bodyJt->x * 10;
            skdBody[j].Y = bodyJt->y * 10;
            skdBody[j].Z = bodyJt->z * 100;

            qDebug("Frame %d: (%f, %f, %f)\n",i+1, skdBody[j].X, skdBody[j].Y, skdBody[j].Z);
            if (j == 0)
                sleepTime = bodyJt->t;
          }

          QImage image(g_nXRes,g_nYRes,QImage::Format_RGB32);

          for (unsigned nY=0; nY<g_nYRes; nY++)
          {
             uchar *imageptr = image.scanLine(nY);

             for (unsigned nX=0; nX < g_nXRes; nX++)
             {
                imageptr[0] = 0x00;
                imageptr[1] = 0x00;
                imageptr[2] = 0x00;
                imageptr[3] = 0xff;

                imageptr+=4;
             }
          }
          QPainter painter;
          painter.begin(&image);
          painter.setPen(QPen(QBrush(Qt::white),5));
          drawSkeleton(&painter, skdBody);

          painter.end();

          ui->skeView->setPixmap(QPixmap::fromImage(image));
       //   ui->skeView->setFixedSize(image.size());
          ui->playProgressBar->setValue(i+1);
          myDelay(sleepTime);
      }

      fclose(skeFile);
    }

}

void LoadDataDlg::drawSkeleton(QPainter *painter, XnPoint3D *bodies)
{

      drawLimb(painter,bodies,QKinect::Head,QKinect::Neck);

      drawLimb(painter,bodies,QKinect::Neck,QKinect::LeftShoulder);
      drawLimb(painter,bodies,QKinect::LeftShoulder,QKinect::LeftElbow);
      drawLimb(painter,bodies,QKinect::LeftElbow,QKinect::LeftHand);

      drawLimb(painter,bodies,QKinect::Neck,QKinect::RightShoulder);
      drawLimb(painter,bodies,QKinect::RightShoulder,QKinect::RightElbow);
      drawLimb(painter,bodies,QKinect::RightElbow,QKinect::RightHand);

      drawLimb(painter,bodies,QKinect::LeftShoulder,QKinect::Torso);
      drawLimb(painter,bodies,QKinect::RightShoulder,QKinect::Torso);

      drawLimb(painter,bodies,QKinect::Torso,QKinect::LeftHip);
      drawLimb(painter,bodies,QKinect::LeftHip,QKinect::LeftKnee);
      drawLimb(painter,bodies,QKinect::LeftKnee,QKinect::LeftFoot);

      drawLimb(painter,bodies,QKinect::Torso,QKinect::RightHip);
      drawLimb(painter,bodies,QKinect::RightHip,QKinect::RightKnee);
      drawLimb(painter,bodies,QKinect::RightKnee,QKinect::RightFoot);

      drawLimb(painter,bodies,QKinect::LeftHip,QKinect::RightHip);

}


/**
  \brief Draws on painter the limb j1-j2 of body
**/
void LoadDataDlg::drawLimb(QPainter *painter, XnPoint3D *body, QKinect::BodyJoints j1,QKinect::BodyJoints j2)
{

   painter->drawLine(body[j1].X,body[j1].Y,body[j2].X,body[j2].Y);
}

void LoadDataDlg::myDelay(clock_t ms)
{
    clock_t start = clock();

    while ((clock() - start) * 1.0 / CLOCKS_PER_SEC * 1000 < ms)
        ;
}
