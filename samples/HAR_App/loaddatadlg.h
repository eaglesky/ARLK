#ifndef LOADDATADLG_H
#define LOADDATADLG_H

#include <QDebug>
#include <QDialog>
#include <vector>
#include "QKinectWrapper.h"
#include "ARLK/SkeData.h"
#include <QMessageBox>

namespace Ui {
class LoadDataDlg;
}

class LoadDataDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoadDataDlg(QWidget *parent = 0);
    ~LoadDataDlg();
    void playSkeFile(const char* fileName);
    QStringList skdFiles;
private slots:
    void on_browseButton_clicked();

    void on_playButton_clicked();

private:
    Ui::LoadDataDlg *ui;
    void drawSkeleton(QPainter *painter, XnPoint3D *bodies);
    void drawLimb(QPainter *painter, XnPoint3D *body, QKinect::BodyJoints j1,QKinect::BodyJoints j2);

    void myDelay(clock_t ms);
};

#endif // LOADDATADLG_H
