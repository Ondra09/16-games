/**
 * @file 05-minesweeper.h
 * @date 4-Aug-2020
 */

#ifndef __minesweeper_H__
#define __minesweeper_H__

#define __NO_SCROLLED__

#include "Scroll.h"

class TileObject;

#define BOARD_SIZE 10

/** Game Class
 */
class Minesweeper : public Scroll<Minesweeper>
{
public:

private :

    orxSTATUS
    Bootstrap() const;

    void Update(const orxCLOCK_INFO &Info);

    orxSTATUS Init();
    orxSTATUS Run();
    void Exit();
    void BindObjects();

    TileObject* board[BOARD_SIZE][BOARD_SIZE];

private:
};

#endif // __minesweeper_H__
