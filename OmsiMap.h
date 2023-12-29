#ifndef OMSIMAP_H
#define OMSIMAP_H

#include <QtCore>
#include <QList>

#include "OmsiMapTile.h"


class OmsiMap {
public:
    OmsiMap();

    void addTile(OmsiMapTile *);
    void addTile(int x, int y, QString fileName);

    QList<OmsiMapTile *> tiles() const;
    void setTiles(const QList<OmsiMapTile *> &newTiles);

    int width() const;
    int height() const;

private:
    QList<OmsiMapTile *> _tiles;
};

#endif // OMSIMAP_H
