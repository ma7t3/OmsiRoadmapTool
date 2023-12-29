#ifndef OMSIMAPTILE_H
#define OMSIMAPTILE_H

#include "OmsiSpline.h"

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

private:
    int _x, _y;
    QString _fileName;

    QList<OmsiSpline *> _splines;
};

#endif // OMSIMAPTILE_H
