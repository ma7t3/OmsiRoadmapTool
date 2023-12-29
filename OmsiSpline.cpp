#include "OmsiSpline.h"

OmsiSpline::OmsiSpline(QString fileName, float x, float y, float rot, float len, float rad) :
    _x(x),
    _y(y),
    _rot(rot),
    _rad(rad),
    _len(len),
    _fileName(fileName) {
}

float OmsiSpline::x() const {
    return _x;
}

void OmsiSpline::setX(float newX) {
    _x = newX;
}

float OmsiSpline::y() const {
    return _y;
}

void OmsiSpline::setY(float newY) {
    _y = newY;
}

float OmsiSpline::rot() const {
    return _rot;
}

void OmsiSpline::setRot(float newRot) {
    _rot = newRot;
}

float OmsiSpline::rad() const {
    return _rad;
}

void OmsiSpline::setRad(float newRad) {
    _rad = newRad;
}

float OmsiSpline::len() const {
    return _len;
}

void OmsiSpline::setLen(float newLen) {
    _len = newLen;
}

QString OmsiSpline::fileName() const {
    return _fileName;
}

void OmsiSpline::setFileName(const QString &newFileName) {
    _fileName = newFileName;
}
