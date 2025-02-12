#ifndef OMSIMAPTILE_H
#define OMSIMAPTILE_H

#include "OmsiSpline.h"
#include "OmsiSceneryobject.h"

#include <QString>


class OmsiMapTile {
public:
    OmsiMapTile(int x, int y, QString fileName);

    int x() const;
    void setX(int newX);

    int y() const;
    void setY(int newY);

    QString fileName() const;
    void setFileName(const QString &newFileName);

    void addSpline(OmsiSpline *);
    QList<OmsiSpline *> splines() const;
    void setSplines(const QList<OmsiSpline *> &newSplines);

    void addObject(OmsiSceneryobject *);
    QList<OmsiSceneryobject *> objects() const;
    void setObjects(const QList<OmsiSceneryobject *> &newObjects);

    float terrain(const int &x, const int &y);
    void setTerrain(const QList<float> &newTerrain);

    float water();
    void setWater(const QList<float> &newWater);

    bool hasWater() const;

private:
    int _x, _y;
    QString _fileName;

    QList<OmsiSpline *> _splines;
    QList<OmsiSceneryobject *> _objects;
    float _terrain[61][61];

    bool _hasWater = false;
    float _water[2][2];
    float _waterSimple;
};

#endif // OMSIMAPTILE_H
