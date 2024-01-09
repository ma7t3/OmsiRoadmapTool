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

void OmsiMapTile::addObject(OmsiSceneryobject *object) {
    _objects << object;
}

QList<OmsiSceneryobject *> OmsiMapTile::objects() const {
    return _objects;
}

void OmsiMapTile::setObjects(const QList<OmsiSceneryobject *> &newObjects) {
    _objects = newObjects;
}

float OmsiMapTile::terrain(const int &x, const int &y) {
    if(x > 61 || y > 61)
        return 0;

    return _terrain[x][y];
}

void OmsiMapTile::setTerrain(const QList<float> &newTerrain) {
    float originalArray[61][61];

    for(int i = 0; i < 61; ++i) {
        for(int j = 0; j < 61; ++j) {
            originalArray[i][j] = newTerrain[i * 61 + j];
        }
    }

    // rotate
    for (int i = 0; i < 61; ++i) {
        for (int j = 0; j < 61; ++j) {
            _terrain[j][61 - 1 - i] = originalArray[i][j];
        }
    }
}
