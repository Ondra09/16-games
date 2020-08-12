/**
 * @file 08-outrun.cpp
 * @date 9-Aug-2020
 */

#include "orx.h"

#define LINES_NO 100
#define FOLIAGE_NO (LINES_NO/10)
orxVIEWPORT *viewport = nullptr;
orxFLOAT camHeight, camWidth;
orxCAMERA *camera = nullptr;

// TODO:  cam Depth from cam
// TODO: read all settings from .ini file
const orxFLOAT camDepth = 1.f;
const float speed = 1.0f;
const float trackLength = 0.15f;
const int trackWidth = 120;
const int borderWidth = 25;

float trackTime = 0.f;
float startSinSeconds = 5.0f;
float startCurveSeconds = 3.0f;
float camerDY = -0.19f;


struct FoliageBillboard
{
    orxTEXTURE *texture = nullptr;
};

#define FOLIAGE_TYPES 3
const char *foliageList[FOLIAGE_TYPES] = {"tree.png",
                                          "tree-2.png",
                                          "bush.png"};
FoliageBillboard foliage[FOLIAGE_TYPES];


//! renders in screen coordinates
void DrawQuad(orxRGBA &color, int x1, int y1, int w1, int x2, int y2, int w2)
{
    orxVECTOR vertices[4] = {{float(x1-w1), float(y1), 0.0f},
                             {float(x2-w2), float(y2), 0.0f},
                             {float(x2+w2), float(y2), 0.0f},
                             {float(x1+w1), float(y1), 0.0f}};
    orxDisplay_DrawPolygon(vertices, 4, color, true);
}

float ProjectionDenominator(float z, orxVECTOR *cameraPosition)
{
    return camDepth/(z-cameraPosition->fZ);
}

// returns false if projected point is behind camera
bool Project(int &X, int &Y, int &WIDTH,
             float x, float y, float z, float width,
             orxVECTOR *cameraPosition)
{
    float denom = ProjectionDenominator(z, cameraPosition);

    X = (1 + (x-cameraPosition->fX) * denom) * camWidth/2;
    Y = (1 + (y-cameraPosition->fY) * denom) * camHeight/2;

    WIDTH = width * denom;

    return denom > 0;
}

float ComputeHeight(float time)
{
    return sin(time) * 0.25f;
}

float ComputeCurve(float time)
{
    return sin(time);
}

/** Update function, it has been registered to be called every tick of the core clock
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
    orxVECTOR cameraPosition = {};
    orxCamera_GetPosition(camera, &cameraPosition);

    // Should quit?
    if(orxInput_IsActive("Quit"))
    {
        // Send close event
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    }

    if (orxInput_IsActive("Left"))
    {
        cameraPosition.fX -= 0.5f *_pstClockInfo->fDT;
    }

    if (orxInput_IsActive("Right"))
    {
        cameraPosition.fX += 0.5f *_pstClockInfo->fDT;
    }

    if (orxInput_IsActive("Up"))
    {
        camerDY -= 0.5f *_pstClockInfo->fDT;
    }

    if (orxInput_IsActive("Down"))
    {
        camerDY += 0.5f *_pstClockInfo->fDT;
    }

    float time = _pstClockInfo->fTime;
    int seconds = (int) _pstClockInfo->fTime;

    trackTime += _pstClockInfo->fDT * speed;

    static int switches = 0; // controls switching of color quads to compenasate for trackLength modulo

    if (trackTime > trackLength) { // float modulo
        trackTime -= trackLength;
        switches++;
    }

    float height = ComputeHeight(time);
    cameraPosition.fY = height + camerDY;

    orxCamera_SetPosition(camera, &cameraPosition);

    for (int i = LINES_NO; i >= 0; i--)
    {
        int X1, Y1, X2, Y2, WIDTH1, WIDTH2;

        bool bothFrontOfCamera = false;

        float trackBegZ = i*trackLength - trackTime;
        float trackEndZ = trackBegZ + trackLength;

        float height = ComputeHeight(trackBegZ+time);
        float height2 = ComputeHeight(trackEndZ+time);

        float curve = ComputeCurve((trackBegZ+time)*0.5);
        float curve2 = ComputeCurve((trackEndZ+time)*0.5);

        bothFrontOfCamera = Project(X1, Y1, WIDTH1, curve, height, trackBegZ, trackWidth, &cameraPosition);
        bothFrontOfCamera &= Project(X2, Y2, WIDTH2, curve2, height2, trackEndZ, trackWidth, &cameraPosition);

        orxRGBA colorRoad;
        orxRGBA colorGrass;
        orxRGBA colorRumble;
        if ((i+switches)%2)
        {
            colorGrass = {16, 200, 16, 255};
            colorRumble = {240, 240, 240, 255};
            colorRoad = {107, 107, 107, 255};
        }
        else
        {
            colorGrass = {0, 154, 0, 255};
            colorRumble = {0, 0, 0, 255};
            colorRoad = {105, 105, 105, 255};
        }

        if (bothFrontOfCamera)
        {
            DrawQuad(colorGrass, 0, Y1, camWidth, 0, Y2, camWidth);
            DrawQuad(colorRumble, X1, Y1, WIDTH1*1.2, X2, Y2, WIDTH2*1.2);
            DrawQuad(colorRoad, X1, Y1, WIDTH1, X2, Y2, WIDTH2);

            if ((i+switches)%2)
            {
                int foliageIndex = ((i+switches)/2+1) % 3;
                orxBITMAP *bitmap = orxTexture_GetBitmap(foliage[foliageIndex].texture);

                orxDISPLAY_MESH mesh = {};
                float texWidth, texHeight;

                orxTexture_GetSize(foliage[foliageIndex].texture, &texWidth, &texHeight);

                float width = (float)texWidth * ProjectionDenominator(trackBegZ, &cameraPosition) * 0.25f;
                float height = (float)texHeight * ProjectionDenominator(trackBegZ, &cameraPosition) * 0.25f;

                float Xoffset = WIDTH1 * 2.0;

                if ((i+switches)%3)
                {
                    Xoffset *= -1;
                }

                orxDISPLAY_VERTEX vv[4] = {{X1-width + Xoffset, Y1-height, 0, 0, {255,255,255,255}},
                                           {X1+width + Xoffset, Y1-height, 1, 0, {255,255,255,255}},
                                           {X1+width + Xoffset, Y1+1, 1, 1, {255,255,255,255}},
                                           {X1-width + Xoffset, Y1+1, 0, 1, {255,255,255,255}}};

                orxU16 indices[4] = {1, 0, 2, 3};
                mesh.astVertexList = vv;
                mesh.au16IndexList = indices;
                mesh.u32VertexNumber = 4;
                mesh.u32IndexNumber = 4;
                mesh.ePrimitive = orxDISPLAY_PRIMITIVE_TRIANGLE_STRIP;


                orxDisplay_DrawMesh(&mesh,
                                    bitmap,
                                    orxDISPLAY_SMOOTHING_DEFAULT,
                                    orxDISPLAY_BLEND_MODE_ALPHA);
            }
        }
    }
}

/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS orxFASTCALL Init()
{
    // Display a small hint in console
    orxLOG("\n* This template project creates a simple scene"
    "\n* You can play with the config parameters in ../data/config/08-outrun.ini"
    "\n* After changing them, relaunch the executable to see the changes.");

    orxMath_InitRandom((orxS32)orxSystem_GetRealTime()); //a good seeding value

    // Create the viewport
    viewport = orxViewport_CreateFromConfig("MainViewport");

    camera = orxViewport_GetCamera(viewport);

    orxViewport_GetSize(viewport, &camWidth, &camHeight);

    // Create the scene
    orxObject_CreateFromConfig("Scene");

    // Register the Update function to the core clock
    orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);


    for (int i = 0; i < FOLIAGE_TYPES; i++)
    {
        foliage[i].texture =  orxTexture_CreateFromFile(foliageList[i], true);
    }

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
