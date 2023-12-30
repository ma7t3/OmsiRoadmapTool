#include "OmsiMap.h"
#include "OmsiMapTile.h"

OmsiMap::OmsiMap() {

}

void OmsiMap::addTile(OmsiMapTile *tile) {
    _tiles << tile;
}

void OmsiMap::addTile(int x, int y, QString fileName) {
    _tiles << new OmsiMapTile(x, y, fileName);
}

QList<OmsiMapTile *> OmsiMap::tiles() const {
    return _tiles;
}

void OmsiMap::setTiles(const QList<OmsiMapTile *> &newTiles) {
    _tiles = newTiles;
}

int OmsiMap::width() const {
    int value = 0;
    foreach(OmsiMapTile *tile, _tiles) {
        if(tile->x() > value)
            value = tile->x();
    }
    return value + 1;
}

int OmsiMap::height() const {
    int value = 0;
    foreach(OmsiMapTile *tile, _tiles) {
        if(tile->y() > value)
            value = tile->y();
    }
    return value + 1;
}

QList<QPoint> OmsiMap::busstops() const {
    return _busstops;
}

void OmsiMap::setBusstops(const QList<QPoint> &newBusstops) {
    _busstops = newBusstops;
}

void OmsiMap::addBusstop(const QPoint busstop) {
    _busstops << busstop;
}

QList<QPair<QPoint, QString> > OmsiMap::busstopLabels() const {
    return _busstopLabels;
}

void OmsiMap::setBusstopLabels(const QList<QPair<QPoint, QString> > &newBusstopLabels) {
    _busstopLabels = newBusstopLabels;
}

void OmsiMap::addBusstopLabel(const QPair<QPoint, QString> busstopLabel) {
    _busstopLabels << busstopLabel;
}





