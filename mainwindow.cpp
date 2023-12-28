#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("ma7t3", "OmsiRoadmapTool");

    if(!settings->contains("omsiPath"))
        omsiDir = new QDir("");
    else {
        omsiDir = new QDir(settings->value("omsiPath").toString());
        if(!omsiDir->exists())
            omsiDir->setPath("");
    }

    ui->leOmsiDir->setText(omsiDir->path());

    on_pbMapsReload_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbOmsiDirBrowse_clicked() {
    QString path = settings->value("omsiPath").toString();

    path = QFileDialog::getExistingDirectory(this, "", path, QFileDialog::ShowDirsOnly);
    if(path == "")
        return;

    QFile omsiExe(path + "/omsi.exe");
    QDir omsiMaps(path + "/maps");
    if(!omsiExe.exists() || !omsiMaps.exists()) {
        QMessageBox::critical(this, tr("Invalid Directory"), tr("<p><b>The selected directory is not valid.</b></p><p>No omsi.exe or maps folder found.</p>"));
        return;
    }

    ui->leOmsiDir->setText(path);
    omsiDir->setPath(path);
    settings->setValue("omsiPath", path);

    on_pbMapsReload_clicked();
}

void MainWindow::on_pbMapsReload_clicked() {
    ui->cbMaps->clear();

    QDir mapsDir = omsiDir->path() + "/maps";
    if(!mapsDir.exists())
        return;

    QStringList folders = mapsDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    foreach(QString current, folders) {
        QFile globalCfg(mapsDir.path() + "/" + current + "/global.cfg");
        if(!globalCfg.exists())
            continue;
        ui->cbMaps->addItem(current);
    }
}

void MainWindow::on_pbTargetPathBrowse_clicked() {
    QString path = QFileDialog::getSaveFileName(this, "", "Roadmap.png", "Portable Network Graphics (*.png)");
    if(path == "")
        return;
    ui->leTargetPath->setText(path);
}


void MainWindow::on_pbStart_clicked() {
    /*QPixmap pixmap(1024, 1024);
    pixmap.fill(Qt::green);

    pixmap.save(ui->leTargetPath->text());*/
}

