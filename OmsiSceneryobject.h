#ifndef OMSISCENERYOBJECT_H
#define OMSISCENERYOBJECT_H

#include "OmsiPath.h"

#include <QtCore>


class OmsiSceneryobject
{
public:
    OmsiSceneryobject(QString fileName, float x, float y, float rot);

    float x() const;
    void setX(float newX);

    float y() const;
    void setY(float newY);

    float rot() const;
    void setRot(float newRot);

    QString fileName() const;
    void setFileName(const QString &newFileName);

    bool exists();

    QList<OmsiPath *> pathList();

private:
    float _x, _y, _rot;
    QString _fileName;
};

#endif // OMSISCENERYOBJECT_H
