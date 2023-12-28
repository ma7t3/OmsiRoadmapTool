#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

private:
    Ui::MainWindow *ui;

    QSettings *settings;
    QDir *omsiDir;
};
#endif // MAINWINDOW_H
