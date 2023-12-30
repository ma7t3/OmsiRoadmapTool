#include "OmsiPath.h"

OmsiPath::OmsiPath(float x, float y, float rot, float len, float rad, int type) :
    _x(x),
    _y(y),
    _rot(rot),
    _rad(rad),
    _len(len),
    _type(type) {
}

float OmsiPath::x() const {
    return _x;
}

void OmsiPath::setX(float newX) {
    _x = newX;
}

float OmsiPath::y() const {
    return _y;
}

void OmsiPath::setY(float newY) {
    _y = newY;
}

float OmsiPath::rot() const {
    return _rot;
}

void OmsiPath::setRot(float newRot) {
    _rot = newRot;
}

float OmsiPath::rad() const {
    return _rad;
}

void OmsiPath::setRad(float newRad) {
    _rad = newRad;
}

float OmsiPath::len() const {
    return _len;
}

void OmsiPath::setLen(float newLen) {
    _len = newLen;
}

int OmsiPath::type() const {
    return _type;
}

void OmsiPath::setType(int newType) {
    _type = newType;
}
