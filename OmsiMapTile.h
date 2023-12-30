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

private:
    int _x, _y;
    QString _fileName;

    QList<OmsiSpline *> _splines;
    QList<OmsiSceneryobject *> _objects;
};

#endif // OMSIMAPTILE_H
