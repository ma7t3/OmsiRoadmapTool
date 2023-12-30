#ifndef OMSISCENERYOBJECT_H
#define OMSISCENERYOBJECT_H

#include "OmsiPath.h"

#include <QtCore>


class OmsiSceneryobject
{
public:
    OmsiSceneryobject(QString fileName, float x, float y, float rot, QStringList strings);

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

    QStringList strings() const;
    void setStrings(const QStringList &newStrings);
    QString stringAt(int);

private:
    float _x, _y, _rot;
    QString _fileName;
    QStringList _strings;
};

#endif // OMSISCENERYOBJECT_H
