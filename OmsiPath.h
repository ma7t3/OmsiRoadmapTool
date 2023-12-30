#ifndef OMSIPATH_H
#define OMSIPATH_H


class OmsiPath
{
public:
    OmsiPath(float x, float y, float rot, float len, float rad, int type);

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

    int type() const;
    void setType(int newType);

private:
    float _x, _y, _rot, _rad, _len;
    int _type;
};

#endif // OMSIPATH_H
