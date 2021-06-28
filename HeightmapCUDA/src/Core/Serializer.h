#ifndef _SERIALIZER_H
#define _SERIALIZER_H

struct Serializer
{
    virtual void Save() = 0;
    virtual void Load() = 0;
};

#endif //_SERIALIZER_H
