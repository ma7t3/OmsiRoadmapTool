#ifndef OmsiSceneryobject_H
#define OmsiSceneryobject_H

#include "OmsiSpline.h"

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

    QList<OmsiSpline *> pathList();

private:
    float _x, _y, _rot;
    QString _fileName;
};

#endif // OmsiSceneryobject_H
