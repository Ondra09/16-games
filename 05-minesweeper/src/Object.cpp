/**
 * @file Object.cpp
 * @date 4-Aug-2020
 */

#include "Object.h"

void Object::OnCreate()
{
    orxConfig_SetBool("IsObject", orxTRUE);
}

void Object::OnDelete()
{
}

void Object::Update(const orxCLOCK_INFO &_rstInfo)
{
}
