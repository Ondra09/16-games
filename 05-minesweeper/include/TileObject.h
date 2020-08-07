/**
 * @file Object.h
 * @date 4-Aug-2020
 */

#ifndef __TILE_OBJECT_H__
#define __TILE_OBJECT_H__

#include "05-minesweeper.h"


/** Object Class
 */
class TileObject : public ScrollObject
{
public:
    virtual ~TileObject() {}
    enum class Tiles {
                      EMPTY,
                      ONE,
                      TWO,
                      THREE,
                      FOUR,
                      FIVE,
                      SIX,
                      SEVEN,
                      EIGHT,
                      MINE,
                      NEW,
                      FLAG
    };
    Tiles tile;
    Tiles tileShown = Tiles::NEW;
protected:

    void            OnCreate();
    void            OnDelete();
    void            Update(const orxCLOCK_INFO &Info);


private:
};

#endif // __TILE_OBJECT_H__
