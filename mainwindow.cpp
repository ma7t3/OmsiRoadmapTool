#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "OmsiMap.h"
#include "OmsiMapTile.h"
#include "OmsiSpline.h"
#include "OmsiSceneryobject.h"
#include "OmsiPath.h"

#include <QMessageBox>
#include <QFileDialog>

#include <QSettings>

#include <QColorDialog>

#include <QPixmap>
#include <QPainter>
#include <QPen>

#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    busstopLabelFont("Open Sans", 26, 700),
    _workerThread(new WorkerThread(this)) {
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

    connect(_workerThread, &WorkerThread::progressMaximumChanged, ui->progressBar, &QProgressBar::setMaximum);
    connect(_workerThread, &WorkerThread::progressValueChanged, ui->progressBar, &QProgressBar::setValue);
    connect(_workerThread, &WorkerThread::log, this, &MainWindow::log);

    on_pbMapsReload_clicked();
}

MainWindow::~MainWindow() {
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
    ui->lwLog->clear();

    // load tiles
    QDir mapDir = omsiDir->path() + "/maps/" + ui->cbMaps->currentText();
    QFile globalCfg(mapDir.path() + "/global.cfg");
    if(!globalCfg.exists()) {
        QMessageBox::critical(this, tr("Invalid Map"), tr("<p><b>File global.cfg not found.</b></p>"));
        return;
    }

    _workerThread->setOmsiDir(*omsiDir);
    _workerThread->setMapName(ui->cbMaps->currentText());
    _workerThread->setDrawBusstops(ui->cbDrawBusstops->isChecked());
    _workerThread->setDrawBusstopNames(ui->cbDrawBusstopsNames->isChecked());
    _workerThread->setDrawWater(ui->cbDrawWater->isChecked());
    _workerThread->setDrawTerrainHeight(ui->rbTerrainHeight->isChecked());
    _workerThread->setBusstopLabelFont(busstopLabelFont);
    _workerThread->setStreetPen(QPen(QColor(ui->pbStreetColor->styleSheet().remove("background-color: ")), ui->sbPathWidthStreet->value()));
    _workerThread->setRailPen(QPen(QColor(ui->pbRailColor->styleSheet().remove("background-color: ")), ui->sbPathWidthRail->value()));
    _workerThread->setBusstopColor(ui->pbBusstopColor->styleSheet().remove("background-color: "));
    _workerThread->setTerrainBackgroundColor(ui->pbBackgroundColor->styleSheet().remove("background-color: "));
    _workerThread->setWaterColor(ui->pbWaterColor->styleSheet().remove("background-color: "));
    _workerThread->setTerrainFactor(ui->hsTerrainFactor->value());
    _workerThread->setTargetImageFilePath(ui->leTargetPath->text());
    _workerThread->start();
}

void MainWindow::log(QString message) {
    ui->lwLog->addItem(message);
    ui->lwLog->scrollToBottom();
}

void MainWindow::on_pbStreetColor_clicked() {
    QColor newColor = QColorDialog::getColor(ui->pbStreetColor->styleSheet().remove("background-color: "), this);
    ui->pbStreetColor->setStyleSheet("background-color: " + newColor.name(QColor::HexRgb));
}


void MainWindow::on_pbRailColor_clicked() {
    QColor newColor = QColorDialog::getColor(ui->pbRailColor->styleSheet().remove("background-color: "), this);
    ui->pbRailColor->setStyleSheet("background-color: " + newColor.name(QColor::HexRgb));
}


void MainWindow::on_pbBackgroundColor_clicked() {
    QColor newColor = QColorDialog::getColor(ui->pbBackgroundColor->styleSheet().remove("background-color: "), this);
    ui->pbBackgroundColor->setStyleSheet("background-color: " + newColor.name(QColor::HexRgb));
}


void MainWindow::on_pbBusstopColor_clicked() {
    QColor newColor = QColorDialog::getColor(ui->pbBusstopColor->styleSheet().remove("background-color: "), this);
    ui->pbBusstopColor->setStyleSheet("background-color: " + newColor.name(QColor::HexRgb));
}

void MainWindow::on_pbWaterColor_clicked() {
    QColor newColor = QColorDialog::getColor(ui->pbWaterColor->styleSheet().remove("background-color: "), this);
    ui->pbWaterColor->setStyleSheet("background-color: " + newColor.name(QColor::HexRgb));
}

void MainWindow::on_cbDrawBusstops_stateChanged(int arg1) {
    Q_UNUSED(arg1);
    ui->cbDrawBusstopsNames->setEnabled(ui->cbDrawBusstops->isChecked());
    on_cbDrawBusstopsNames_stateChanged(0);
}


void MainWindow::on_cbDrawBusstopsNames_stateChanged(int arg1) {
    Q_UNUSED(arg1);
    ui->fcbBusstopFont->setEnabled(ui->cbDrawBusstopsNames->isChecked());
}



