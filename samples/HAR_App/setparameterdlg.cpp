#include "setparameterdlg.h"
#include "ui_setparameterdlg.h"



SetParameterDlg::SetParameterDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetParameterDlg)
{
    ui->setupUi(this);
}

SetParameterDlg::~SetParameterDlg()
{
    delete ui;
}

void SetParameterDlg::init(_TRAINPARAMS* params)
{
    m_trainParams = *params;
    ui->wt_edit->setText(QString::number(m_trainParams.feaWt));
    ui->svm_c_edit->setText(QString::number(m_trainParams.svmParams.C));
    ui->svm_gamma_edit->setText(QString::number(m_trainParams.svmParams.gamma));
    ui->knn_k_edit->setText(QString::number(m_trainParams.knnParams.K));
    ui->knn_eps_edit->setText(QString::number(m_trainParams.knnParams.eps));

    if (m_trainParams.classifier == 0)
        ui->radioSVM->setChecked(true);
    else if (m_trainParams.classifier == 1)
        ui->radioKNN->setChecked(true);
}

void SetParameterDlg::on_radioSVM_toggled(bool checked)
{
    if(checked) {
        qDebug() << "SVM checked!";
        m_trainParams.classifier = 0;
    }
}

void SetParameterDlg::on_radioKNN_toggled(bool checked)
{
    if (checked) {
        qDebug() << "KNN checked!";
        m_trainParams.classifier = 1;
    }
}

void SetParameterDlg::on_paramButtonBox_accepted()
{
    m_trainParams.feaWt = ui->wt_edit->text().toDouble();
    m_trainParams.svmParams.C = ui->svm_c_edit->text().toDouble();
    m_trainParams.svmParams.gamma = ui->svm_gamma_edit->text().toDouble();
    m_trainParams.knnParams.K = ui->knn_k_edit->text().toInt();
    m_trainParams.knnParams.eps = ui->knn_eps_edit->text().toDouble();
}
