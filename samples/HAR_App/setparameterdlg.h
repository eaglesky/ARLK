#ifndef SETPARAMETERDLG_H
#define SETPARAMETERDLG_H

#include <QDialog>
#include <QDebug>

typedef struct {
    struct _SVMPARAMS{
        double C;
        double gamma;
    }svmParams;

    double feaWt;

    struct _KNNPARAMS {
        int K;
        double eps;
    }knnParams;

    int classifier;
}_TRAINPARAMS;

namespace Ui {
class SetParameterDlg;
}

class SetParameterDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit SetParameterDlg(QWidget *parent = 0);
    ~SetParameterDlg();



    void init(_TRAINPARAMS* params);
    _TRAINPARAMS m_trainParams;
private slots:
    void on_radioSVM_toggled(bool checked);

    void on_radioKNN_toggled(bool checked);

    void on_paramButtonBox_accepted();

private:
    Ui::SetParameterDlg *ui;


};

#endif // SETPARAMETERDLG_H
