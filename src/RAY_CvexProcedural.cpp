//
// Created by symek on 8/8/20.
//

#include <UT/UT_DSOVersion.h>
#include <GU/GU_Detail.h>
#include <RAY/RAY_ProceduralFactory.h>
#include "RAY_CvexProcedural.hpp"

using namespace HDK_Sample;
static RAY_ProceduralArg        theArgs[] = {
        RAY_ProceduralArg("file",           "string",       ""),
        RAY_ProceduralArg("blurfile",       "string",       ""),
        RAY_ProceduralArg("velocityblur",   "int",          "0"),
        RAY_ProceduralArg("shutter",        "real",         "1"),
        RAY_ProceduralArg()
};
class ProcDef : public RAY_ProceduralFactory::ProcDefinition
{
public:
    ProcDef()
            : RAY_ProceduralFactory::ProcDefinition("demofile")
    {
    }
    virtual RAY_Procedural      *create() const { return new RAY_DemoFile(); }
    virtual RAY_ProceduralArg   *arguments() const { return theArgs; }
};
void
registerProcedural(RAY_ProceduralFactory *factory)
{
    factory->insert(new ProcDef);
}
RAY_DemoFile::RAY_DemoFile()
        : myShutter(1),
          myPreBlur(0),
          myPostBlur(0)
{
    myBox.initBounds(0, 0, 0);
}
RAY_DemoFile::~RAY_DemoFile()
{
}
const char *
RAY_DemoFile::className() const
{
    return "RAY_DemoFile";
}
int
RAY_DemoFile::initialize(const UT_BoundingBox *box)
{
    int         ival;
    // The user is required to specify the bounds of the geometry.
    // Alternatively, if the bounds aren't specified, we could forcibly load
    // the geometry at this point and compute the bounds ourselves.
    if (!box)
    {
        fprintf(stderr, "The %s procedural needs a bounding box specified\n",
                className());
        return 0;
    }
    // Stash away the box the user specified
    myBox = *box;

    // Import the shutter time.  This is a scale between 0 and 1
    if (!import("shutter", &myShutter, 1))
        myShutter = 1;
    // A toggle for whether to use velocityblur or not.  Since there's no
    // "bool" type for parameters, import into an int.
    if (import("velocityblur", &ival, 1))
        myVelocityBlur = (ival != 0);
    else
        myVelocityBlur = false;
    // Import the filenames for t0 and t1
    import("file", myFile);
    import("blurfile", myBlurFile);
    // Import the shutter settings for velocity blur. The 'camera:shutter'
    // stores the start and end of the shutter window in fraction of
    // a frame. Divide by the current FPS value to get the correct
    // scaling for velocity blur.
    fpreal      fps = 24.0, shutter[2] = {0};
    import("global:fps", &fps, 1);
    import("camera:shutter", shutter, 2);
    myPreBlur = -(myShutter * shutter[0]) / fps;
    myPostBlur = (myShutter * shutter[1]) / fps;
    return 1;
}
void
RAY_DemoFile::getBoundingBox(UT_BoundingBox &box)
{
    box = myBox;
}
void
RAY_DemoFile::render()
{
    // Allocate geometry.
    // Warning:  When allocating geometry for a procedural, do not simply
    // construct a GU_Detail, but call RAY_Procedural::createGeometry().
    RAY_ProceduralGeo   g0 = createGeometry();
    // Load geometry from disk
    if (!g0->load(myFile, 0).success())
    {
        fprintf(stderr, "Unable to load geometry[0]: '%s'\n",
                myFile.c_str());
        return;
    }
    // Add geometry to mantra.
    if (myVelocityBlur)
    {
        // If performing velocity blur, then we add velocity blur geometry
        g0.addVelocityBlur(myPreBlur, myPostBlur);
    }
    else
    {
        // Otherwise, we check to see if there's a motion blur geometry file
        if (myBlurFile.isstring())
        {
            auto        g1 = g0.appendSegmentGeometry(myShutter);
            GU_DetailHandleAutoWriteLock        wlock(g1);
            if (!wlock.getGdp()->load(myBlurFile, 0).success())
            {
                fprintf(stderr, "Unable to load geometry[1]: '%s'\n",
                        myBlurFile.c_str());
                g0.removeSegmentGeometry(g1);
            }
        }
    }
    RAY_ProceduralChildPtr      obj = createChild();
    obj->addGeometry(g0);
}

