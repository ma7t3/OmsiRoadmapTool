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

#include <QPixmap>
#include <QPainter>

#include <QDesktopServices>

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
         * [object]
         * 0
         * filePath
         * ID
         * x
         * y
         * z
         * rot
         * .
         * .
         * StrCount
         * Strings
         *
         * */

        while(!s.atEnd()) {
            QString line = s.readLine();
            if(line == "[spline]" || line == "[spline_h]") {
                float x, y, rot, rad, len;
                QString fileName, id;

                bool ok[6];

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

                fileName.replace("\\", "/");

                OmsiSpline *spline = new OmsiSpline(omsiDir->path() + "/" + fileName, x, y, rot, len, rad);
                tile->addSpline(spline);
            }

            if(line == "[object]") {
                float x, y, rot;
                QString fileName, id;
                int strCount;

                bool ok[4];

                s.readLine();
                fileName = s.readLine();
                id = s.readLine();
                x = s.readLine().toFloat(&ok[0]);
                y = s.readLine().toFloat(&ok[1]);
                s.readLine();
                rot = s.readLine().toFloat(&ok[2]);
                s.readLine();
                s.readLine();
                strCount = s.readLine().toFloat(&ok[3]);
                QStringList strings = {};
                if(ok[3]) {
                    for(int i = 0; i < strCount; i++) {
                        strings << s.readLine();
                    }
                }

                if(!ok[0] || !ok[1] || !ok[2]) {
                    log(tr("Bad Object - No. %1").arg(id));
                    continue;
                }

                fileName.replace("\\", "/");

                OmsiSceneryobject *object = new OmsiSceneryobject(omsiDir->path() + "/" + fileName, x, y, rot, strings);
                tile->addObject(object);
            }
        }
    }

    foreach(OmsiMapTile *tile, map->tiles()) {
        foreach(OmsiSceneryobject *object, tile->objects()) {
            if(object->fileName() == omsiDir->path() + "/Sceneryobjects/Generic/bus_stop.sco") {

                float x, y;
                x = (tile->x()) * 300 + object->x();
                y = (((map->height() - 1) - tile->y()) * 300) + (300 - object->y());

                QPoint p(x, y);
                QPoint lp(x + 15, y + 10);
                QString label = object->stringAt(0);
                map->addBusstop(p);
                map->addBusstopLabel(QPair<QPoint, QString>(lp, label));
            }
        }
    }

    // join busstop labels
    QList<QPair<QPoint, QString>> allLabels = map->busstopLabels();
    QList<QPair<QPoint, QString>> filteredLabels = {};

    QStringList uniqueNames;

    for(int i = 0; i < allLabels.count(); i++) {
        bool found = false;
        foreach(QString current, uniqueNames) {
            if(current == allLabels[i].second) {
                found = true;
                break;
            }
        }
        if(!found)
            uniqueNames << allLabels[i].second;
    }


    foreach(QString currentName, uniqueNames) {
        float newX = 0;
        QList<float> newY;

        for(int i = 0; i < allLabels.count(); i++) {
            QPair<QPoint, QString> currentLabel = allLabels[i];

            if(currentLabel.second != currentName)
                continue;

            if(newY.empty()) {
                newX = currentLabel.first.x();
            } else {
                if(currentLabel.first.x() > newX)
                    newX = currentLabel.first.x();
            }

            newY << currentLabel.first.y();
        }

        float newYSum = 0;
        foreach(int c, newY)
            newYSum += c;

        float newYAvg = newYSum / newY.count();

        filteredLabels << QPair<QPoint, QString>(QPoint(newX, newYAvg), currentName);
    }

    map->setBusstopLabels(filteredLabels);

    log("Finished loading map!");

    log("drawing paths...");

    ui->progressBar->setValue(0);

    qApp->processEvents();

    int width, height;
    width = map->width() * 300;
    height = map->height() * 300;

    QPixmap *pixmap = new QPixmap(width, height);
    pixmap->fill(Qt::black);

    QPainter *painter = new QPainter(pixmap);
    painter->setRenderHint(QPainter::Antialiasing);

    int i = 0;
    foreach(OmsiMapTile *tile, map->tiles()) {
        foreach(OmsiSpline *spline, tile->splines()) {
            drawSpline(painter, spline, tile, map->height());
        }

        foreach(OmsiSceneryobject *object, tile->objects()) {
            QList<OmsiPath *> pathes = object->pathList();
            foreach(OmsiPath *path, pathes) {

                // adjust coordinates
                float x = path->x(), y = path->y(), objRot = 360 - object->rot();

                float newX = (x * (qCos(qDegreesToRadians(objRot))) - (y * qSin(qDegreesToRadians(objRot))));
                float newY = (x * (qSin(qDegreesToRadians(objRot))) + (y * qCos(qDegreesToRadians(objRot))));

                path->setX(object->x() + newX);
                path->setY(object->y() + newY);
                path->setRot(path->rot() - objRot);

                drawPath(painter, path, tile, map->height());
            }
        }

        i++;
        ui->progressBar->setValue(i);
        qApp->processEvents();
    }

    log("drawing busstops...");

    foreach(QPoint busstop, map->busstops()) {
        drawBusstop(painter, busstop);
    }

    QList<QPair<QPoint, QString>> labels = map->busstopLabels();
    for(int i = 0; i < map->busstopLabels().count(); i++) {
        QPair<QPoint, QString> label = labels[i];
        drawBusstopLabel(painter, label.first, label.second);
    }

    log("saving image to file...");
    ui->progressBar->setMaximum(0);
    qApp->processEvents();

    pixmap->save(ui->leTargetPath->text());
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);

    log("saved image to PNG!");

    qApp->thread()->msleep(100);
    QDesktopServices::openUrl(ui->leTargetPath->text());
}

void MainWindow::drawSpline(QPainter *painter, OmsiSpline *spline, OmsiMapTile *tile, int mapHeight) {
    QList<QPair<int, float>> pathList = spline->pathList();

    for(int i = 0; i < pathList.count(); i++) {
        QPair<int, float> current = pathList[i];

        float newRad;
        if(spline->rad() == 0)
            newRad = 0;
        else
            newRad = spline->rad() - current.second;

        float newLen = spline->len();
        if(newRad != 0)
            newLen = spline->len() * (newRad / spline->rad());

        float x = current.second, y = 0, splRot = 360 - spline->rot();

        float newX = (x * (qCos(qDegreesToRadians(splRot))) - (y * qSin(qDegreesToRadians(splRot))));
        float newY = (x * (qSin(qDegreesToRadians(splRot))) + (y * qCos(qDegreesToRadians(splRot))));

        OmsiPath *path = new OmsiPath(spline->x() + newX, spline->y() + newY, - splRot, newLen, newRad, current.first);
        drawPath(painter, path, tile, mapHeight);
    }
}

void MainWindow::drawPath(QPainter *painter, OmsiPath *path, OmsiMapTile *tile, int mapHeight) {
    QPen streetPen(Qt::white, 4);
    QPen railPen(Qt::gray, 2);

    if(path->type() == 0)
        painter->setPen(streetPen);
    else if(path->type() == 2)
        painter->setPen(railPen);
    else
        return;

    // get all values from spline
    float x = path->x();
    float y = path->y();
    float rot = path->rot();
    float rad = path->rad();
    float len = path->len();

    // calculate start- and span-angle
    float startAngle, spanAngle;
    startAngle = - (rot * 16) + 180*16;
    spanAngle  = - ((len / rad) * (180 / 3.141592654) * 16);

    // if radius is not zero (that means, it's a curve)
    if(rad != 0) {

        // rotate by 180 deg (for left-curves)
        if(rad < 0)
            startAngle += 180*16;

        // calculate square
        float alpha = startAngle / 16;

        float centerX, centerY;
        centerX = x - (qCos(qDegreesToRadians(alpha)) * qAbs(rad));
        centerY = y - (qSin(qDegreesToRadians(alpha)) * qAbs(rad));
        float width = 2 * rad;
        float height = 2 * rad;

        // determine top left corner
        float topLeftX = centerX - rad;
        float topLeftY = centerY + rad;

        // define square
        QRect rect(
            (tile->x() * 300) + topLeftX,
            (((mapHeight - 1) - tile->y()) * 300) + (300 - topLeftY),
            width,
            height
            );

        // draw arc
        painter->drawArc(rect, startAngle, spanAngle);
    } else {
        // else (radius equals 0), means, we just want to draw a straight line

        // determine end coordinates
        float endX = x + (len * qSin(qDegreesToRadians(rot)));
        float endY = y + (len * qCos(qDegreesToRadians(rot)));

        // draw line
        painter->drawLine(
            (tile->x() * 300) + x,
            (((mapHeight - 1) - tile->y()) * 300) + (300 - y),
            (tile->x() * 300) + endX,
            (((mapHeight - 1) - tile->y()) * 300) + (300 - endY)
        );
    }
}

void MainWindow::drawBusstop(QPainter *painter, QPoint point) {
    QPen busstopPen(Qt::black, 6);
    QBrush busstopBrush (QColor("#ffc000"));
    painter->setPen(busstopPen);
    painter->setBrush(busstopBrush);

    painter->drawEllipse(point.x() - 10, point.y() - 10, 20, 20);
}

void MainWindow::drawBusstopLabel(QPainter *painter, QPoint point, QString label) {
    QPen busstopLabelOutlinePen(Qt::black);
    QPen busstopLabelPen(QColor("#ffc000"), 2);

    QFont busstopLabelOutlineFont("Open Sans", 26, 700);
    QFont busstopLabelFont("Open Sans", 26, 700);

    painter->setPen(busstopLabelOutlinePen);
    painter->setFont(busstopLabelOutlineFont);

    painter->drawText(point.x() + 2, point.y() + 2, label);
    painter->drawText(point.x() + 2, point.y() - 2, label);
    painter->drawText(point.x() - 2, point.y() + 2, label);
    painter->drawText(point.x() - 2, point.y() - 2, label);

    painter->setPen(busstopLabelPen);
    painter->setFont(busstopLabelFont);
    painter->drawText(point, label);
}

void MainWindow::log(QString message) {
    ui->lwLog->addItem(message);
    ui->lwLog->scrollToBottom();
}
























