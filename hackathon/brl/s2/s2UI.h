

#ifndef S2UI_H
#define S2UI_H

#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTcpSocket>
#include <QDir>
#include <QTabWidget>
#include <v3d_interface.h>
#include "s2Controller.h"
#include "s2plot.h"
#include "stackAnalyzer.h"
QT_BEGIN_NAMESPACE
class QWidget;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

//! [0]
class S2UI : public QDialog
{
    Q_OBJECT

public:
    S2UI(V3DPluginCallback2 &callback, QWidget *parent = 0 );
    QLineEdit *s2LineEdit;

    StackAnalyzer *myStackAnalyzer;
public slots:
    void pmStatusHandler(bool pmStatus);

signals:
    void startPM();
    void stopPM();
    void newImageData(Image4DSimple*);
private slots:
    void startS2();
    void startScan();
    void loadScan();
    void displayScan();
    void posMonButtonClicked();
    void updateS2Data(  QMap<int, S2Parameter> currentParameterMap);
    void updateString(QString broadcastedString);
    void updateFileString(QString inputString);
    void startingZStack();
    QString getFileString();
    void updateROIPlot(QString ignore);
private:
    V3DPluginCallback2 * cb;
    QLabel *s2Label;
    //QLabel *labeli;
    QThread *workerThread;

    QTabWidget * lhTabs;
    QGridLayout *mainLayout;
    QPushButton *startS2PushButton;
    QPushButton *startScanPushButton;
    QPushButton  *startZStackPushButton;
    QPushButton *loadScanPushButton;
    QPushButton *startPosMonButton;
    QDialogButtonBox *buttonBox1;
    QGroupBox *parameterBox;
    QVBoxLayout *vbox;
    S2Controller myController;
    S2Controller myPosMon;
    bool posMonStatus;
    QString fileString ;
    QString lastFile;
    bool waitingForFile;
    QGroupBox *createROIControls();
    QGroupBox *createS2Monitors();
    QDialogButtonBox *createButtonBox1();
    void hookUpSignalsAndSlots();
    QMap<int, S2Parameter> uiS2ParameterMap;
    void checkParameters(QMap<int, S2Parameter> currentParameterMap);
    QGroupBox *roiGroupBox;
    QGroupBox *createROIMonitor();
    QRectF roiRect;
    QLineEdit *roiXEdit;
    QLineEdit *roiYEdit;
    QLineEdit *roiZEdit;


    QLineEdit *roiXWEdit ;
    QLineEdit *roiYWEdit ;
    QLineEdit *roiZWEdit;
    QGridLayout * gl;
    QGraphicsScene * roiGS;
    QGraphicsView * roiGV;
    QGraphicsRectItem *newRect;
    QPushButton *centerGalvosPB ;
    QPushButton * startStackAnalyzerPB;
};
//! [0]

#endif
