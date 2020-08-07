/**
 * @file 05-minesweeper.cpp
 * @date 4-Aug-2020
 */

#define __SCROLL_IMPL__
#include "05-minesweeper.h"
#undef __SCROLL_IMPL__

#include "Object.h"
#include "TileObject.h"


/** Update function, it has been registered to be called every tick of the core clock
 */
void Minesweeper::Update(const orxCLOCK_INFO &_rstInfo)
{
    // Should quit?
    if(orxInput_IsActive("Quit"))
    {
        // Send close event
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    }

    orxVECTOR vPos;
    if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vPos), orxNULL, &vPos) != orxNULL)
    {
        // Let's see what's currently under the mouse
        auto *object = this->PickObject(vPos, orxString_GetID("Tiles"));
        bool tileChanged = false;
        if(object && (orxInput_HasBeenActivated("Select")))
        {
            auto *tile = dynamic_cast<TileObject*>(object);

            if (tile
                && tile->tileShown == TileObject::Tiles::NEW)
            {
                tile->tileShown = tile->tile;
                tileChanged = true;
            }
        }

        if(object && (orxInput_HasBeenActivated("Flag")))
        {
            auto *tile = dynamic_cast<TileObject*>(object);

            if (tile && tile->tileShown == TileObject::Tiles::NEW)
            {
                tile->tileShown = TileObject::Tiles::FLAG;
                tileChanged = true;
            } else if (tile && tile->tileShown == TileObject::Tiles::FLAG)
            {
                tile->tileShown = TileObject::Tiles::NEW;
                tileChanged = true;
            }
        }

        // we need to change graphics
        if (tileChanged)
        {
            // see tutorial: https://wiki.orx-project.org/en/tutorials/community/sausage/semi-dynamic_objects_and_level_mapping
            auto *tile = dynamic_cast<TileObject*>(object);
            auto *tileObject = tile->GetOrxObject();

            // clear old
            orxGRAPHIC *linkedGraphic = orxGRAPHIC(orxOBJECT_GET_STRUCTURE(tileObject, GRAPHIC));

            if (linkedGraphic)
            {
                orxObject_UnlinkStructure(tileObject, orxSTRUCTURE_ID_GRAPHIC);
                // it does not seems to be leaking
                // orxGraphic_Delete(linkedGraphic);
                // orxObject_Delete(object);
            }

            orxCHAR buffer[16] = {"G-Tile10"};
            if ((int)tile->tileShown < 12)
            {
                orxString_NPrint(buffer, sizeof(buffer) - 1, "G-Tile%d", (int)tile->tileShown);
            }

            // create new
            orxGRAPHIC *graphic;
            graphic = orxGraphic_CreateFromConfig(buffer);


            orxObject_LinkStructure(tileObject, orxSTRUCTURE(graphic));
        }
    }
}


void incrementBoard(TileObject::Tiles (&board)[BOARD_SIZE][BOARD_SIZE], int i, int j)
{
    if (i >= BOARD_SIZE
        || j >= BOARD_SIZE
        || i < 0
        || j < 0)
    {
        return;
    }

    if (board[i][j] != TileObject::Tiles::MINE) {
        board[i][j] = TileObject::Tiles((int)board[i][j] + 1);
    }
}

/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS Minesweeper::Init()
{
    // Display a small hint in console
    orxLOG("\n* This template project creates a simple scene"
    "\n* You can play with the config parameters in ../data/config/05-minesweeper.ini"
    "\n* After changing them, relaunch the executable to see the changes.");

    orxMath_InitRandom((orxS32)orxSystem_GetRealTime()); //a good seeding value
    TileObject::Tiles initBoard[BOARD_SIZE][BOARD_SIZE];

    for (int i = 0; i < BOARD_SIZE; i++)
    {
      for (int j = 0; j < BOARD_SIZE; j++)
      {
          initBoard[i][j] = TileObject::Tiles::EMPTY;
      }
    }

    // create 10 mines
    int mines = 0;

    while (mines < 10) {
        orxS32 mineIdx = orxMath_GetRandomS32(0, BOARD_SIZE * BOARD_SIZE);
        int i = mineIdx / BOARD_SIZE;
        int j = mineIdx % BOARD_SIZE;

        if (initBoard[i][j] != TileObject::Tiles::MINE) {
            initBoard[i][j] = TileObject::Tiles::MINE;
            mines++;
        }
    }

    // propagate neighbours
    for (int i = 0; i < BOARD_SIZE; i++)
    {
      for (int j = 0; j < BOARD_SIZE; j++)
      {
          if (initBoard[i][j] == TileObject::Tiles::MINE)
          {
              incrementBoard(initBoard, i-1, j-1);
              incrementBoard(initBoard, i-1, j);
              incrementBoard(initBoard, i-1, j+1);

              incrementBoard(initBoard, i, j-1);
              incrementBoard(initBoard, i, j+1);

              incrementBoard(initBoard, i+1, j-1);
              incrementBoard(initBoard, i+1, j);
              incrementBoard(initBoard, i+1, j+1);
          }
      }
    }

    // Create the scene
    CreateObject("Scene");

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            auto *tile = dynamic_cast<TileObject*>(CreateObject("O-Tile"));
            // TODO: remove magic numbers 32
            tile->SetPosition({j*32.0f-5*32, i*32.0f - 4*32, 0}, orxTRUE);
            tile->tile = initBoard[i][j];

            board[i][j] = tile;
        }
    }

    // Done!
    return orxSTATUS_SUCCESS;
}

/** Run function, it should not contain any game logic
 */
orxSTATUS Minesweeper::Run()
{
    // Return orxSTATUS_FAILURE to instruct orx to quit
    return orxSTATUS_SUCCESS;
}

/** Exit function, it is called before exiting from orx
 */
void Minesweeper::Exit()
{
    // Let Orx clean all our mess automatically. :)
}

/** BindObjects function, ScrollObject-derived classes are bound to config sections here
 */
void Minesweeper::BindObjects()
{
    // Bind the Object class to the Object config section
    ScrollBindObject<Object>("Object");
    ScrollBindObject<TileObject>("O-Tile");
}

/** Bootstrap function, it is called before config is initialized, allowing for early resource storage definitions
 */
orxSTATUS Minesweeper::Bootstrap() const
{
    // Add a config storage to find the initial config file
    orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

    // Return orxSTATUS_FAILURE to prevent orx from loading the default config file
    return orxSTATUS_SUCCESS;
}

/** Main function
 */
int main(int argc, char **argv)
{
    // Execute our game
    Minesweeper::GetInstance().Execute(argc, argv);

    // Done!
    return EXIT_SUCCESS;
}
