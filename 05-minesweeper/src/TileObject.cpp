/**
 */

#include "TileObject.h"

void TileObject::OnCreate()
{
    orxConfig_SetBool("IsObject", orxTRUE);
}

void TileObject::OnDelete()
{
}

void TileObject::Update(const orxCLOCK_INFO &_rstInfo)
{
}
