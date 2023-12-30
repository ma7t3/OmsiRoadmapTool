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

    QList<QPoint> busstops() const;
    void setBusstops(const QList<QPoint> &newBusstops);
    void addBusstop(const QPoint);

    QList<QPair<QPoint, QString> > busstopLabels() const;
    void setBusstopLabels(const QList<QPair<QPoint, QString>> &newBusstopLabels);
    void addBusstopLabel(const QPair<QPoint, QString>);

private:
    QList<OmsiMapTile *> _tiles;
    QList<QPoint> _busstops;
    QList<QPair<QPoint, QString>> _busstopLabels;
};

#endif // OMSIMAP_H
