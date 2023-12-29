#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "OmsiMap.h"
#include "OmsiMapTile.h"
#include "OmsiSpline.h"

#include <QMessageBox>
#include <QFileDialog>

#include <QSettings>

#include <QPixmap>
#include <QPainter>

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

    ui->cbMaps->setCurrentText("Testmap");
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
    QFile globalCfg(mapDir.path() + "/global.cfg");;
    if(!globalCfg.exists()) {
        QMessageBox::critical(this, tr("Invalid Map"), tr("<p><b>File global.cfg not found.</b></p>"));
        return;
    }

    OmsiMap *map = new OmsiMap;

    globalCfg.open(QIODevice::ReadOnly);
    QTextStream s(&globalCfg);
    s.setEncoding(QStringConverter::Utf16LE);

    log("Reading tiles...");

    int tileCount = -1;
    while(!s.atEnd()) {
        QString line = s.readLine();
        if(line == "[map]") {
            tileCount++;
            QString strX, strY, path;
            strX = s.readLine();
            strY = s.readLine();
            path = s.readLine();

            bool okX, okY;
            int x = strX.toInt(&okX);
            int y = strY.toInt(&okY);
            if(okX && okY) {
                map->addTile(x, y, path);
            } else
                log(tr("Tile No. %1 is bad!").arg(QString::number(tileCount)));
        }
    }

    // adjust tiles
    int minX = 0, minY = 0;
    foreach(OmsiMapTile *tile, map->tiles()) {
        if(minX > tile->x()) minX = tile->x();
        if(minY > tile->y()) minY = tile->y();
    }
    foreach(OmsiMapTile *tile, map->tiles()) {
        tile->setX(tile->x() - minX);
        tile->setY(tile->y() - minY);
    }


    log("Finished reading tiles!");

    ui->progressBar->setMaximum(tileCount);

    log("Loading map...");

    int currentTile = -1;
    foreach(OmsiMapTile *tile, map->tiles()) {
        currentTile++;
        ui->progressBar->setValue(currentTile);

        QFile f(mapDir.path() + "/" + tile->fileName());
        if(!f.exists()) {
            log(tr("Tile \"%1\" - file not found!").arg(tile->fileName()));
            qApp->processEvents();
            continue;
        }

        log(tr("Loading map (%1/%2)").arg(QString::number(currentTile), QString::number(tileCount)));
        qApp->processEvents();

        f.open(QIODevice::ReadOnly);
        QTextStream s(&f);
        s.setEncoding(QStringConverter::Utf16LE);

        /*
         * [spline]
         * 0
         * filePath
         * ID
         * ???
         * ???
         * x
         * z
         * y
         * rot
         * length
         * radius
         *
         * */

        while(!s.atEnd()) {
            QString line = s.readLine();
            if(line == "[spline]" || line == "[spline_h]") {
                float x, y, rot, rad, len;
                QString fileName, id;

                bool ok[5];

                s.readLine();
                fileName = s.readLine();
                id = s.readLine();
                s.readLine();
                s.readLine();
                x = s.readLine().toFloat(&ok[0]);
                s.readLine();
                y = s.readLine().toFloat(&ok[1]);
                rot = s.readLine().toFloat(&ok[2]);
                len = s.readLine().toFloat(&ok[3]);
                rad = s.readLine().toFloat(&ok[4]);

                if(!ok[0] || !ok[1] || !ok[2] || !ok[3] || !ok[4]) {
                    log(tr("Bad Spline - No. %1").arg(id));
                    continue;
                }

                if(id == "22639") {
                    qDebug() << rot;
                }

                OmsiSpline *spline = new OmsiSpline(fileName, x, y, rot, len, rad);
                tile->addSpline(spline);
            }
        }
    }

    log("Finished loading map!");

    log("drawing image...");

    ui->progressBar->setValue(0);

    qApp->processEvents();

    int width, height;
    width = map->width() * 300;
    height = map->height() * 300;

    QPen streetPen(Qt::white, 5);

    QPixmap *pixmap = new QPixmap(width, height);
    pixmap->fill(Qt::lightGray);

    QPainter *painter = new QPainter(pixmap);
    painter->setPen(streetPen);

    int i = 0;
    foreach(OmsiMapTile *tile, map->tiles()) {
        foreach(OmsiSpline *spline, tile->splines()) {
            drawSpline(painter, spline, tile, map->height());
        }
        i++;
        ui->progressBar->setValue(i);
        qApp->processEvents();
    }

    log("saving image to file...");
    ui->progressBar->setMaximum(0);
    qApp->processEvents();

    pixmap->save(ui->leTargetPath->text());
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);

    log("saved image to PNG!");
}

void MainWindow::drawSpline(QPainter *painter, OmsiSpline *spline, OmsiMapTile *tile, int mapHeight) {
    float x = spline->x();
    float y = spline->y();
    float rot = spline->rot();
    float rad = spline->rad();
    float len = spline->len();

    float startAngle, spanAngle;
    startAngle = - (rot * 16) + 180*16;
    spanAngle  = - ((len / rad) * (180 / 3.141592654) * 16);

    if(rad != 0) {

        // rotate by 180 deg for left-curves
        if(rad < 0)
            startAngle += 180*16;

        float alpha = startAngle / 16;

        float centerX, centerY;
        centerX = x - (qCos(qDegreesToRadians(alpha)) * qAbs(rad));
        centerY = y - (qSin(qDegreesToRadians(alpha)) * qAbs(rad));

        float topLeftX = centerX - rad;
        float topLeftY = centerY + rad;
        float width = 2 * rad;
        float height = 2 * rad;

        QRect rect(
            (tile->x() * 300) + topLeftX,
            (((mapHeight - 1) - tile->y()) * 300) + (300 - topLeftY),
            width,
            height
        );

        //painter->drawRect(rect);
        painter->drawArc(rect, startAngle, spanAngle);
    } else {
        float endX = x + (len * qSin(qDegreesToRadians(rot)));
        float endY = y + (len * qCos(qDegreesToRadians(rot)));
        painter->drawLine(
            (tile->x() * 300) + x,
            (((mapHeight - 1) - tile->y()) * 300) + (300 - y),
            (tile->x() * 300) + endX,
            (((mapHeight - 1) - tile->y()) * 300) + (300 - endY)
        );
    }
}

void MainWindow::log(QString message) {
    ui->lwLog->addItem(message);
    ui->lwLog->scrollToBottom();
}
























