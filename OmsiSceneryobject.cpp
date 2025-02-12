#include "OmsiSceneryobject.h"

OmsiSceneryobject::OmsiSceneryobject(QString fileName, float x, float y, float rot, QStringList strings) :
    _x(x),
    _y(y),
    _rot(rot),
    _fileName(fileName),
    _strings(strings) {
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

QList<OmsiPath *> OmsiSceneryobject::pathList() {
    QFile f(_fileName);
    if(!f.exists())
        return {};

    if(!f.open(QIODevice::ReadOnly))
        return {};

    QTextStream s(&f);
    s.setEncoding(QStringConverter::Utf8);

    QList<OmsiPath *> result;

    while(!s.atEnd()) {
        QString line = s.readLine();

        if(line == "[path]" || line == "[path_2]") {
            float x, y, rot, rad, len;
            int type;
            bool ok[6];

            x = s.readLine().toFloat(&ok[0]);
            y = s.readLine().toFloat(&ok[1]);
            s.readLine();
            rot = s.readLine().toFloat(&ok[2]);
            rad = s.readLine().toFloat(&ok[3]);
            len = s.readLine().toFloat(&ok[4]);

            s.readLine();
            s.readLine();
            type = s.readLine().toInt(&ok[5]);

            if(!ok[0] || !ok[1] || !ok[2] || !ok[3] || !ok[4] || !ok[5])
                continue;

            OmsiPath *spline = new OmsiPath(x, y, rot, len, rad, type);
            result << spline;
        }
    }

    return result;
}

QStringList OmsiSceneryobject::strings() const {
    return _strings;
}

void OmsiSceneryobject::setStrings(const QStringList &newStrings) {
    _strings = newStrings;
}

QString OmsiSceneryobject::stringAt(int index) {
    if(_strings.count() <= index)
        return "";

    return _strings[index];
}






















