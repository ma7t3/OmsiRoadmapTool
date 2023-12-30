#ifndef OMSISPLINE_H
#define OMSISPLINE_H

#include <QtCore>


class OmsiSpline
{
public:
    OmsiSpline(QString fileName, float x, float y, float rot, float len, float rad);

    float x() const;
    void setX(float newX);

    float y() const;
    void setY(float newY);

    float rot() const;
    void setRot(float newRot);

    float rad() const;
    void setRad(float newRad);

    float len() const;
    void setLen(float newLen);

    QString fileName() const;
    void setFileName(const QString &newFileName);

    bool exists();

    QList<QPair<int, float>> pathList();

    int pathType();

private:
    float _x, _y, _rot, _rad, _len;
    QString _fileName;
};

#endif // OMSISPLINE_H
