/**
 * @file 02-doodle-jump.cpp
 * @date 24-Jul-2020
 */

#include "orx.h"


int score = 0;

orxOBJECT *hero = nullptr;
orxOBJECT *scoreObject = nullptr;
orxCAMERA *camera = nullptr;

static const orxVECTOR jumpSpeed = { 0, -600, 0 };
static const orxVECTOR autoJumpSpeedPlatform = { 0, -600, 0 };

void orxFASTCALL Jump(orxOBJECT *hero, const orxVECTOR *jumpSpeed) {
    orxObject_ApplyImpulse(hero, jumpSpeed, orxNULL);
}

/** Update function, it has been registered to be called every tick of the core clock
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
    // Should quit?
    if(orxInput_IsActive("Quit"))
    {
        // Send close event
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    }

    orxVECTOR moveSpeed = { -20, 0, 0 };

    orxVECTOR flipLeft = { 1, 1, 1 };
    orxVECTOR flipRight = { -1, 1, 1 };

    if (orxInput_IsActive("GoLeft"))
    {
        orxObject_SetScale(hero, &flipLeft);
        orxObject_ApplyImpulse(hero, &moveSpeed, orxNULL);
    }

    if (orxInput_IsActive("GoRight"))
    {
        orxObject_SetScale(hero, &flipRight);

        orxVECTOR result = {};
        orxObject_ApplyImpulse(hero, orxVector_Mulf(&result, &moveSpeed, -1), orxNULL);
    }

    if (orxInput_IsActive("Jump")
        && orxInput_HasNewStatus("Jump")) // ignores key-down, reacts only when key is pressed
    {
        orxLOG("doodle jump");
        Jump(hero, &jumpSpeed);
    }

    if (scoreObject)
    {
        orxCHAR formattedScore[7+6];
        orxString_Print(formattedScore, "Score: %d", score);

        orxObject_SetTextString(scoreObject, formattedScore);
    }
}

/**
   Physics handler
*/
orxSTATUS orxFASTCALL PhysicsEventHandler(const orxEVENT *_pstEvent)
{
    if (_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
    {
        orxOBJECT *pstRecipientObject, *pstSenderObject;

        pstSenderObject = orxOBJECT(_pstEvent->hSender);
        pstRecipientObject = orxOBJECT(_pstEvent->hRecipient);

        orxSTRING senderObjectName = (orxSTRING) orxObject_GetName(pstSenderObject);
        orxSTRING recipientObjectName = (orxSTRING) orxObject_GetName(pstRecipientObject);

        if ((orxString_Compare(senderObjectName, "DoodleObject") == 0 || orxString_Compare(recipientObjectName, "DoodleObject") == 0)
            && (orxString_Compare(senderObjectName, "PlatformObject") == 0 || orxString_Compare(recipientObjectName, "PlatformObject") == 0)
            )
        {
            orxLOG("doodle auto jump");

            Jump(hero, &autoJumpSpeedPlatform);
            score += 150;
        }
    }

    return orxSTATUS_SUCCESS;
}

void orxFASTCALL InitPlatforms() {


    for (int i = 0; i < 9; i++)
    {
        auto platform = orxObject_CreateFromConfig("PlatformObject");

        orxFLOAT randomNumber = orxMath_GetRandomFloat(-300, 300);
        orxVECTOR position = {randomNumber,
                              i*-50.0f + 250.0f,
                              0};

        orxObject_SetPosition(platform, &position);
    }
}


/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS orxFASTCALL Init()
{
    // Display a small hint in console
    orxLOG("\n* This template project creates a simple scene"
    "\n* You can play with the config parameters in ../data/config/02-doodle-jump.ini"
    "\n* After changing them, relaunch the executable to see the changes.");

    // Init globals
    orxMath_InitRandom((orxS32)orxSystem_GetRealTime()); //a good seeding value

    // Create the viewport
    orxVIEWPORT *viewport = orxViewport_CreateFromConfig("MainViewport");

    // Create the scene
    orxObject_CreateFromConfig("Scene");
    // orxObject_CreateFromConfig("DoodleObject");

    // Register the Update function to the core clock
    orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

    // Get handlers

    InitPlatforms();

    hero = orxObject_CreateFromConfig("DoodleObject");

    camera = orxViewport_GetCamera(viewport);
    orxCamera_SetParent(camera, hero);

    scoreObject = orxObject_CreateFromConfig("ScoreObject");

    orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, PhysicsEventHandler);

    // Done!
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
