/*******************************************************************************
**     FileName: Entity.h
**    ClassName: Entity
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/04 15:29
**  Description:
*******************************************************************************/

#ifndef ENTITY_H
#define ENTITY_H

#include <string>

struct Entity
{
    virtual operator std::string() const = 0;
};

#endif // ENTITY_H
