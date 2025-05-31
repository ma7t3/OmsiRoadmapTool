#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>
#include <QPainter>
#include <QDir>

class OmsiMapTile;
class OmsiSpline;
class OmsiPath;

class WorkerThread : public QThread {
    Q_OBJECT
public:
    explicit WorkerThread(QObject *parent = nullptr);

    void setOmsiDir(const QDir &path);
    void setMapName(const QString &mapName);
    void setDrawBusstops(const bool &b);
    void setDrawBusstopNames(const bool &b);
    void setDrawWater(const bool &b);
    void setDrawTerrainHeight(const bool &b);
    void setBusstopLabelFont(const QFont &font);
    void setStreetPen(const QPen &streetPen);
    void setRailPen(const QPen &railPen);
    void setBusstopColor(const QColor &busstopColor);
    void setTerrainBackgroundColor(const QColor &terrainBackgroundColor);
    void setWaterColor(const QColor &waterColor);
    void setTerrainFactor(const int &factor);
    void setTargetImageFilePath(const QString &path);

protected:
    void run() override;
    void drawSpline(QPainter *, OmsiSpline *, OmsiMapTile *, int height);
    void drawPath(QPainter *, OmsiPath *, OmsiMapTile *, int height);
    void drawBusstop(QPainter *, QPoint);
    void drawBusstopLabel(QPainter *, QPoint, QString);

signals:
    void progressMaximumChanged(const int &);
    void progressValueChanged(const int &);
    void log(const QString &);

private:
    QDir _omsiDir;
    QString _mapName, _targetImageFilePath;
    QFont _busstopLabelFont;
    bool _drawBusstops, _drawBusstopNames, _drawWater, _drawTerrainHeight;
    QColor _busstopColor, _terrainBackgroundColor, _waterColor;
    int _terrainFactor;
    QPen _streetPen, _railPen;
};

#endif // WORKERTHREAD_H
