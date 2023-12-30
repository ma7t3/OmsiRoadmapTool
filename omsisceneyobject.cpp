#include "OmsiSceneyobject.h"

OmsiSceneryobject::OmsiSceneryobject(QString fileName, float x, float y, float rot) :
    _x(x),
    _y(y),
    _rot(rot),
    _fileName(fileName) {
}

float OmsiSceneryobject::x() const {
    return _x;
}

void OmsiSceneryobject::setX(float newX) {
    _x = newX;
}

float OmsiSceneryobject::y() const {
    return _y;
}

void OmsiSceneryobject::setY(float newY) {
    _y = newY;
}

float OmsiSceneryobject::rot() const {
    return _rot;
}

void OmsiSceneryobject::setRot(float newRot) {
    _rot = newRot;
}

QString OmsiSceneryobject::fileName() const {
    return _fileName;
}

void OmsiSceneryobject::setFileName(const QString &newFileName) {
    _fileName = newFileName;
}

bool OmsiSceneryobject::exists() {
    QFile f(_fileName);
    return f.exists();
}

QList<OmsiSpline *> OmsiSceneryobject::pathList() {
    QFile f(_fileName);
    if(!f.exists())
        return {};

    if(!f.open(QIODevice::ReadOnly))
        return {};

    QTextStream s(&f);
    s.setEncoding(QStringConverter::Utf8);

    QList<OmsiSpline *> result;

    while(!s.atEnd()) {
        QString line = s.readLine();

        if(line == "[path]") {
            int x, y, rot, rad, len;
            bool ok[5];

            x = s.readLine().toFloat(&ok[0]);
            y = s.readLine().toFloat(&ok[1]);
            s.readLine();
            rot = s.readLine().toFloat(&ok[2]);
            rad = s.readLine().toFloat(&ok[3]);
            len = s.readLine().toFloat(&ok[4]);

            if(!ok[0] || !ok[1] || !ok[2] || !ok[3] || !ok[4])
                continue;

            OmsiSpline *spline = new OmsiSpline("<PATH>", x, y, rot, len, rad);
            result << spline;
        }
    }

    return result;
}






















