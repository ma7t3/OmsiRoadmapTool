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

#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    busstopLabelFont("Open Sans", 26, 700)
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
        QFile fterrain(mapDir.path() + "/" + tile->fileName() + ".terrain");
        QFile fwater(mapDir.path() + "/" + tile->fileName() + ".water");

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
         * [splineAttachement]
         * 0
         * filePath
         * ID
         * ????
         * x
         * y
         * z
         * rot
         * .
         * .
         * Distance
         * Range
         * ?
         * StrCount
         * Strings
         *
         * */

        while(!s.atEnd()) {
            QString line = s.readLine();

            if(line == "[water]")
                tile->setWater({-5, -5, -5, -5});

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

        f.close();

        if(fterrain.exists()) {
            fterrain.open(QIODevice::ReadOnly);
            QDataStream in(&fterrain);

            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            in.setByteOrder(QDataStream::LittleEndian);

            float nul;
            in >> nul;

            QList<float> values;
            while(!in.atEnd()) {
                float value;
                in >> value;
                values << value;
            }

            tile->setTerrain(values);

            fterrain.close();
        } else {
            log(tr("Tile \"%1\" - terrain file not found!").arg(tile->fileName() + ".terrain"));
            qApp->processEvents();
        }

        if(fwater.exists() && tile->hasWater()) {
            fwater.open(QIODevice::ReadOnly);
            QDataStream in(&fwater);

            in.setFloatingPointPrecision(QDataStream::SinglePrecision);
            in.setByteOrder(QDataStream::LittleEndian);

            float nul;
            in >> nul;

            QList<float> values;
            while(!in.atEnd()) {
                float value;
                in >> value;
                values << value;
            }

            tile->setWater(values);

            fwater.close();
        } else {
            if(tile->hasWater())
                log(tr("Tile \"%1\" - water file not found!").arg(tile->fileName() + ".water"));

            qApp->processEvents();
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

    log("Finished loading map!");
    log("placing busstop labels...");
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(filteredLabels.count());

    qApp->processEvents();

    // move labels (anti collision)
    // for each label
    for(int i = 0; i < filteredLabels.count(); i++) {
        ui->progressBar->setValue(i);
        qApp->processEvents();
        QPair<QPoint, QString> currentLabel = filteredLabels[i];

        // define new points
        QPoint new1 = currentLabel.first, new2 = currentLabel.first;

        // as long as it collides with anything...
        while(true) {

            bool collision[2] = {false, false};

            // check collision with every other label
            for(int j = 0; j < filteredLabels.count(); j++) {
                QPair<QPoint, QString> currentCompareLabel = filteredLabels[j];

                // don't compare with itself
                if(i == j)
                    continue;

                QFontMetrics fm(ui->fcbBusstopFont->currentFont());

                // nährungsweise (schnell)
                int currentWidth = fm.averageCharWidth() * currentLabel.second.length();
                int currentCompareWidth = fm.averageCharWidth() * currentLabel.second.length();

                // exakt (langsam)
                /*int currentWidth = fm.horizontalAdvance(currentLabel.second);
                int currentCompareWidth = fm.horizontalAdvance(currentCompareLabel.second);*/

                // don't compare if they have enough space horizontally
                int xSpace = new1.x() - currentCompareLabel.first.x();
                if(xSpace > 0) {
                    if(qAbs(xSpace) >= currentCompareWidth)
                        continue;
                } else {
                    if(qAbs(xSpace) >= currentWidth)
                        continue;
                }

                // calculate y-axis-space
                int ySpace1 = qAbs(new1.y() - currentCompareLabel.first.y());
                int ySpace2 = qAbs(new2.y() - currentCompareLabel.first.y());

                // if they don't have enough space, save collision
                if(ySpace1 < 35) {
                    collision[0] = true;
                }

                if(ySpace2 < 35) {
                    collision[1] = true;
                }
            }

            // if there was no collision at all, save positoin and go to next label
            if(!collision[0]) {
                currentLabel.first = new1;
                break;
            }

            if(!collision[1]) {
                currentLabel.first = new2;
                break;
            }

            // else move by one pixel and check again
            new1.setY(new1.y() + 1);
            new2.setY(new2.y() - 1);
        }

        filteredLabels[i] = currentLabel;
    }

    map->setBusstopLabels(filteredLabels);
    //map->setBusstopLabels(filteredLabels);

    log("drawing paths...");

    ui->progressBar->setValue(0);

    qApp->processEvents();

    int width, height;
    width = map->width() * 300;
    height = map->height() * 300;

    QPixmap *pixmap = new QPixmap(width, height);

    if(ui->rbTerrainHeight->isChecked())
        pixmap->fill(QColor("#6b8060"));
    else
        pixmap->fill(QColor(ui->pbBackgroundColor->styleSheet().remove("background-color: ")));

    QPainter *painter = new QPainter(pixmap);
    painter->setRenderHint(QPainter::Antialiasing);


    QList<QColor> colors;
    colors << QColor("#6b8060");
    colors << QColor("#91b380");
    colors << QColor("#cfe6c3");
    colors << QColor("#dee6c3");
    colors << QColor("#e6dfc3");
    colors << QColor("#ddba94");
    colors << QColor("#ab8152");
    colors << QColor("#7a4732");

    foreach(OmsiMapTile *tile, map->tiles()) {
        // drawTerrain
        for(int i = 0; i < 61; i++) {
            for(int j = 0; j < 61; j++) {
                float pixelValue = tile->terrain(i, j);

                int x = tile->x() * 300 + i*4.918;
                int y = (((map->height() - 1) - tile->y()) * 300) + j*4.918;

                if(pixelValue >= tile->water() || !tile->hasWater() || !ui->cbDrawWater) {
                    int colorIndex;
                    if(pixelValue < 0)
                        colorIndex = 0;
                    else if(pixelValue > 7 * ui->hsTerrainFactor->value())
                        colorIndex = 7;
                    else
                        colorIndex = pixelValue / ui->hsTerrainFactor->value();

                    if(!ui->rbTerrainHeight->isChecked())
                        continue;

                    painter->setBrush(colors[colorIndex]);
                } else {
                    if(ui->cbDrawWater->isChecked())
                        painter->setBrush(QColor(ui->pbWaterColor->styleSheet().remove("background-color: ")));
                }

                painter->setPen(Qt::NoPen);
                painter->drawRect(x, y, 5, 5);
            }
        }
    }

    painter->setBrush(Qt::NoBrush);

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

    if(ui->cbDrawBusstops->isChecked()) {
        log("drawing busstops...");

        foreach(QPoint busstop, map->busstops()) {
            drawBusstop(painter, busstop);
        }

        if(ui->cbDrawBusstopsNames->isChecked()) {
            QList<QPair<QPoint, QString>> labels = map->busstopLabels();
            for(int i = 0; i < map->busstopLabels().count(); i++) {
                QPair<QPoint, QString> label = labels[i];
                drawBusstopLabel(painter, label.first, label.second);
            }
        }
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
    QPen streetPen(QColor(ui->pbStreetColor->styleSheet().remove("background-color: ")), ui->sbPathWidthStreet->value());
    QPen railPen(QColor(ui->pbRailColor->styleSheet().remove("background-color: ")), ui->sbPathWidthRail->value());

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
    QBrush busstopBrush(QColor(ui->pbBusstopColor->styleSheet().remove("background-color: ")));
    painter->setPen(busstopPen);
    painter->setBrush(busstopBrush);

    painter->drawEllipse(point.x() - 10, point.y() - 10, 20, 20);
}

void MainWindow::drawBusstopLabel(QPainter *painter, QPoint point, QString label) {
    QPen busstopLabelOutlinePen(Qt::black);
    QPen busstopLabelPen(QColor(ui->pbBusstopColor->styleSheet().remove("background-color: ")), 2);

    //QFont busstopLabelOutlineFont(ui->fcbBusstopFont->currentFont(), 26, 700);

    painter->setPen(busstopLabelOutlinePen);
    painter->setFont(ui->fcbBusstopFont->currentFont());

    painter->drawText(point.x() + 1, point.y() + 1, label);
    painter->drawText(point.x() + 1, point.y() - 1, label);
    painter->drawText(point.x() - 1, point.y() + 1, label);
    painter->drawText(point.x() - 1, point.y() - 1, label);

    painter->setPen(busstopLabelPen);
    //painter->setFont(busstopLabelFont);
    painter->drawText(point, label);
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



