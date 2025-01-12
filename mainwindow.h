#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "OmsiMap.h"
#include "OmsiSpline.h"
#include "OmsiMapTile.h"

#include <QMainWindow>
#include <QDir>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbOmsiDirBrowse_clicked();
    void on_pbMapsReload_clicked();
    void on_pbTargetPathBrowse_clicked();
    void on_pbStart_clicked();

    void drawSpline(QPainter *, OmsiSpline *, OmsiMapTile *, int height);
    void drawPath(QPainter *, OmsiPath *, OmsiMapTile *, int height);
    void drawBusstop(QPainter *, QPoint);
    void drawBusstopLabel(QPainter *, QPoint, QString);

    void log(QString);

    void on_pbStreetColor_clicked();

    void on_pbRailColor_clicked();

    void on_pbBackgroundColor_clicked();

    void on_pbBusstopColor_clicked();

    void on_cbDrawBusstops_stateChanged(int arg1);

    void on_cbDrawBusstopsNames_stateChanged(int arg1);

    void on_pbWaterColor_clicked();

private:
    Ui::MainWindow *ui;

    QSettings *settings;
    QDir *omsiDir;

    int RESOLUTION = 300;

    QFont busstopLabelFont;
};
#endif // MAINWINDOW_H
