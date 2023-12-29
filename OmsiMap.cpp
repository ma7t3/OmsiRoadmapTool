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
