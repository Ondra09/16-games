/**
 * @file 03-arkanoid.cpp
 * @date 3-Aug-2020
 */

#include "orx.h"

#include <functional>

orxVIEWPORT *viewport = nullptr;
orxOBJECT *paddle = nullptr;
orxOBJECT *ballObject = nullptr;
orxOBJECT *scoreObject = nullptr;
orxOBJECT *tileParent = nullptr;

float paddleSpeed = 10.0f;
orxVECTOR ballSpeed = {0, 0, 0};

int score = 0;

bool checkTilesCollision(orxOBJECT *tileParent,
                         orxVECTOR *position,
                         std::function<void(orxOBJECT*)> fun)
{
    for (orxOBJECT *pstChild = orxOBJECT(orxObject_GetChild(tileParent));
         pstChild != orxNULL;
         pstChild = orxOBJECT(orxObject_GetSibling(pstChild)))
    {
        orxOBOX box;
        orxObject_GetBoundingBox(pstChild, &box);

        if (orxOBox_2DIsInside(&box, position)) {
            fun(pstChild);

            return true;
        }
    }

    return false;
}

void checkAllTiles(orxOBJECT *ballObject)
{
    orxVECTOR position = {};
    orxObject_GetPosition(ballObject, &position);

    orxVECTOR dx = {ballSpeed.fX, 0, 0};
    orxVECTOR dy = {0, ballSpeed.fY, 0};

    orxVector_Add(&position, &position, &dx);

    bool collided = checkTilesCollision(tileParent, &position, [](orxOBJECT *pstChild)
                                                              {
                                                                  orxSTRING childObjectName = (orxSTRING) orxObject_GetName(pstChild);
                                                                  if (orxString_Compare(childObjectName, "TileObject") == 0)
                                                                  {
                                                                      score += 150;
                                                                      orxObject_SetLifeTime(pstChild, 0);
                                                                  }

                                                                  ballSpeed = { -ballSpeed.fX, ballSpeed.fY, 0 };
                                                              });

    orxVector_Add(&position, &position, &dy);

    if (!collided)
    {
        collided = checkTilesCollision(tileParent, &position, [](orxOBJECT *pstChild)
                                               {
                                                   orxSTRING childObjectName = (orxSTRING) orxObject_GetName(pstChild);
                                                   if (orxString_Compare(childObjectName, "TileObject") == 0)
                                                   {
                                                       score += 150;
                                                       orxObject_SetLifeTime(pstChild, 0);
                                                   }

                                                   auto dX = ballSpeed.fX;

                                                   if (orxString_Compare(childObjectName, "PlatformObject") == 0)
                                                   {
                                                       orxFLOAT randomNumber = orxMath_GetRandomFloat(0, 100);
                                                       if (randomNumber < 20)
                                                       {
                                                           dX = -dX;
                                                       }
                                                   }

                                                   ballSpeed = { dX, -ballSpeed.fY, 0 };
                                               });
    }

    // Move object
    orxObject_GetPosition(ballObject, &position);
    orxVector_Add(&position, &position, &ballSpeed);
    orxObject_SetPosition(ballObject, &position);
}

/** Update function, it has been registered to be called every tick of the core clock
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
    orxVECTOR moveSpeed = { -50, 0, 0 };

    // Should quit?
    if(orxInput_IsActive("Quit"))
    {
        // Send close event
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    }

    // update paddle
    orxVECTOR paddlePosition = {};
    orxVECTOR paddleDX = {0, 0, 0};

    orxObject_GetPosition(paddle, &paddlePosition);

    orxFLOAT viewportWidth;
    orxFLOAT viewportHeight;
    orxViewport_GetSize(viewport, &viewportWidth, &viewportHeight);

    if (orxInput_IsActive("GoLeft")
        && paddlePosition.fX > -(viewportWidth*0.5f))
    {
        paddleDX.fX = -paddleSpeed;
    }

    if (orxInput_IsActive("GoRight")
        && paddlePosition.fX < (viewportWidth*0.5f))
    {
        paddleDX.fX = paddleSpeed;
    }

    orxVector_Add(&paddlePosition, &paddlePosition, &paddleDX);

    orxObject_SetPosition(paddle, &paddlePosition);

    if (orxInput_IsActive("Space"))
    {
        ballSpeed = { -3.5f, -3.5f, 0.0f };
    }

    checkAllTiles(ballObject);

    if (scoreObject)
    {
        orxCHAR formattedScore[7+6];
        orxString_Print(formattedScore, "Score: %d", score);

        orxObject_SetTextString(scoreObject, formattedScore);
    }
}

orxCOLOR idxToColor[5] = {{255,255,0,255}, {255,255,255, 255}, {0,255,0,255}, {0,255,255,255}, {255,0,255,255}};

void createScene(orxOBJECT *tileParent)
{
    for (int i = 0; i < 15; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            auto *tileObject = orxObject_CreateFromConfig("TileObject");

            orxObject_SetParent(tileObject, tileParent);
            orxObject_SetOwner(tileObject, tileParent);

            orxVECTOR size;
            orxObject_GetSize(tileObject, &size);

            orxVECTOR position = {i * size.fX + -290, j * size.fY + -200.0f, 0};

            orxObject_SetPosition(tileObject, &position);
            orxObject_SetColor(tileObject, &idxToColor[j]);
        }
    }
}

/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS orxFASTCALL Init()
{
    // Display a small hint in console
    orxLOG("\n* This template project creates a simple scene"
    "\n* You can play with the config parameters in ../data/config/03-arkanoid.ini"
    "\n* After changing them, relaunch the executable to see the changes.");

    // Create the viewport
    viewport = orxViewport_CreateFromConfig("MainViewport");

    // Create the scene
    orxObject_CreateFromConfig("Scene");

    orxMath_InitRandom((orxS32)orxSystem_GetRealTime()); //a good seeding value

    // Register the Update function to the core clock
    orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

    // custom init
    tileParent = orxObject_CreateFromConfig("TileParent");
    paddle = orxObject_CreateFromConfig("PlatformObject");
    ballObject = orxObject_CreateFromConfig("BallObject");
    scoreObject = orxObject_CreateFromConfig("ScoreObject");

    createScene(tileParent);

    orxObject_SetParent(paddle, tileParent);
    orxObject_SetOwner(paddle, tileParent);

    return orxSTATUS_SUCCESS;
}

/** Run function, it should not contain any game logic
 */
orxSTATUS orxFASTCALL Run()
{
    // Return orxSTATUS_FAILURE to instruct orx to quit
    return orxSTATUS_SUCCESS;
}

/** Exit function, it is called before exiting from orx
 */
void orxFASTCALL Exit()
{
    // Let Orx clean all our mess automatically. :)
}

/** Bootstrap function, it is called before config is initialized, allowing for early resource storage definitions
 */
orxSTATUS orxFASTCALL Bootstrap()
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
    // Set the bootstrap function to provide at least one resource storage before loading any config files
    orxConfig_SetBootstrap(Bootstrap);

    // Execute our game
    orx_Execute(argc, argv, Init, Run, Exit);

    // Done!
    return EXIT_SUCCESS;
}
