#include "OmsiMapTile.h"

OmsiMapTile::OmsiMapTile(int x, int y, QString fileName) :
    _x(x),
    _y(y),
    _fileName(fileName) {
}

int OmsiMapTile::x() const {
    return _x;
}

void OmsiMapTile::setX(int newX) {
    _x = newX;
}

int OmsiMapTile::y() const {
    return _y;
}

void OmsiMapTile::setY(int newY) {
    _y = newY;
}

QString OmsiMapTile::fileName() const {
    return _fileName;
}

void OmsiMapTile::setFileName(const QString &newFileName) {
    _fileName = newFileName;
}

void OmsiMapTile::addSpline(OmsiSpline *spline) {
    _splines << spline;
}

QList<OmsiSpline *> OmsiMapTile::splines() const {
    return _splines;
}

void OmsiMapTile::setSplines(const QList<OmsiSpline *> &newSplines) {
    _splines = newSplines;
}
