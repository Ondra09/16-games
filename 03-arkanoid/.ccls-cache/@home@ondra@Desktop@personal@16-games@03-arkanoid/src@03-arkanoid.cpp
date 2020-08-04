/**
 * @file 03-arkanoid.cpp
 * @date 3-Aug-2020
 */

#include "orx.h"

orxOBJECT *paddle = nullptr;
orxOBJECT *ballObject = nullptr;
orxOBJECT *scoreObject = nullptr;
orxOBJECT *tileParent = nullptr;

orxVECTOR ballSpeed = {0, 0, 0};

int score = 0;

bool checkTilesCollision(orxOBJECT *tileParent, orxVECTOR *position)
{
    for(orxOBJECT *pstChild = orxOBJECT(orxObject_GetChild(tileParent));
        pstChild != orxNULL;
        pstChild = orxOBJECT(orxObject_GetSibling(pstChild)))
    {
        orxOBOX box;
        orxObject_GetBoundingBox(pstChild, &box);

        if (orxOBox_2DIsInside(&box, position))
        {
            score += 150;
            orxObject_SetLifeTime(pstChild, 0);

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

    if (checkTilesCollision(tileParent, &position)) {
        ballSpeed = { -ballSpeed.fX, ballSpeed.fY, 0 };
    }

    orxVector_Add(&position, &position, &dy);

    if (checkTilesCollision(tileParent, &position)) {
        ballSpeed = { ballSpeed.fX, -ballSpeed.fY, 0 };
    }


    // Move object
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

    if (orxInput_IsActive("GoLeft"))
    {
        orxObject_ApplyImpulse(paddle, &moveSpeed, orxNULL);
    }

    if (orxInput_IsActive("GoRight"))
    {
        orxVECTOR result = {};
        orxObject_ApplyImpulse(paddle, orxVector_Mulf(&result, &moveSpeed, -1), orxNULL);
    }

    if (orxInput_IsActive("Space"))
    {
        ballSpeed = { -2.0f, -2.0f, 0.0f };
        //orxObject_SetSpeed(ballObject, &ballSpeed);
    }


    checkAllTiles(ballObject);
}

orxSTATUS orxFASTCALL PhysicsEventHandler(const orxEVENT *_pstEvent)
{
    if (_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
    {
        orxOBJECT *pstRecipientObject, *pstSenderObject;

        pstSenderObject = orxOBJECT(_pstEvent->hSender);
        pstRecipientObject = orxOBJECT(_pstEvent->hRecipient);

        orxSTRING senderObjectName = (orxSTRING) orxObject_GetName(pstSenderObject);
        orxSTRING recipientObjectName = (orxSTRING) orxObject_GetName(pstRecipientObject);

        if ((orxString_Compare(senderObjectName, "TileObject") == 0 || orxString_Compare(recipientObjectName, "TileObject") == 0)
            && (orxString_Compare(senderObjectName, "BallObject") == 0 || orxString_Compare(recipientObjectName, "BallObject") == 0)
            )
        {
            orxVECTOR firstPos = {};
            orxObject_GetPosition(pstSenderObject, &firstPos);

            orxLOG("collision with tile");


            //orxVECTOR result = ballSpeed;
            // orxVector_Mulf(&ballSpeed, &ballSpeed, -1);
            //ballSpeed = {result.fY, -result.fX, 0};

            //orxObject_SetSpeed(ballObject, &ballSpeed);



        }
        /*
        if ((orxString_Compare(senderObjectName, "BallObject") == 0 ||
             orxString_Compare(recipientObjectName, "BallObject") == 0))
        {
            // orxLOG("collision with tileSSSS");

            }
*/
    }

    return orxSTATUS_SUCCESS;
}

orxCOLOR idxToColor[5] = {{255,255,0,255}, {255,255,255, 255}, {0,255,0,255}, {0,255,255,255}, {255,0,255,255}};

void createScene()
{
    tileParent = orxObject_CreateFromConfig("TileParentBody");
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 5; ++j) {
            auto *tileObject = orxObject_CreateFromConfig("TileObject");

            orxObject_SetParent(tileObject, tileParent);

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
    orxViewport_CreateFromConfig("MainViewport");

    // Create the scene
    orxObject_CreateFromConfig("Scene");

    // Register the Update function to the core clock
    orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

    // custom init
    orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, PhysicsEventHandler);

    paddle = orxObject_CreateFromConfig("PlatformObject");
    ballObject = orxObject_CreateFromConfig("BallObject");

    createScene();

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
