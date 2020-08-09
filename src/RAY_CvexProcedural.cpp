//
// Created by symek on 8/8/20.
//
#include <iostream>
#include <UT/UT_DSOVersion.h>
#include <GU/GU_Detail.h>
#include <RAY/RAY_ProceduralFactory.h>
#include <CVEX/CVEX_Context.h>
#include "RAY_CvexProcedural.hpp"

using namespace HA_SKK;
static RAY_ProceduralArg        theArgs[] = {
        RAY_ProceduralArg("file",           "string",       ""),
        RAY_ProceduralArg("vexfile",       "string",       ""),
        RAY_ProceduralArg("velocityblur",   "int",          "0"),
        RAY_ProceduralArg("shutter",        "real",         "1"),
        RAY_ProceduralArg()
};
class ProcDef : public RAY_ProceduralFactory::ProcDefinition
{
public:
    ProcDef()
            : RAY_ProceduralFactory::ProcDefinition("cvexprocedural")
    {
    }
    virtual RAY_Procedural      *create() const { return new RAY_CVexProcedural(); }
    virtual RAY_ProceduralArg   *arguments() const { return theArgs; }
};
void
registerProcedural(RAY_ProceduralFactory *factory)
{
    factory->insert(new ProcDef);
}
RAY_CVexProcedural::RAY_CVexProcedural()
        : myShutter(1),
          myPreBlur(0),
          myPostBlur(0)
{
    myBox.initBounds(0, 0, 0);
}
RAY_CVexProcedural::~RAY_CVexProcedural()
{
}
const char *
RAY_CVexProcedural::className() const
{
    return "RAY_CVexProcedural";
}
int
RAY_CVexProcedural::initialize(const UT_BoundingBox *box)
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
    import("vexfile", myVexFile);

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
RAY_CVexProcedural::getBoundingBox(UT_BoundingBox &box)
{
    box = myBox;
}
void
RAY_CVexProcedural::render()
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
    // Run Cvex on that...
    CVEX_Context cvex;
    CVEX_Value *P_in, *P_out;
    cvex.addInput("P", CVEX_TYPE_VECTOR3, true);
    const char * argv[] = {myVexFile.c_str()};

    if (!cvex.load(1, argv)) {
        std::cerr << "Can't load VEX file\n";
    } else {
        std::cout << "Loading CVEX file " << myVexFile << "\n";
    }

    P_in = cvex.findInput("P", CVEX_TYPE_VECTOR3);

    // Set the values for "P"
    auto handle = g0.handle();
    const auto point_range = handle.gdp()->getPointRange();
    UT_Vector3FArray positions;
    if (P_in)
    {
        handle.gdp()->getPos3AsArray(point_range, positions);
        P_in->setTypedData(positions.data(), handle.gdp()->getNumPoints());  // "bind" the buffer to the variable
    }

    P_out = cvex.findOutput("P", CVEX_TYPE_VECTOR3);
    if(P_out) {
        P_out->setTypedData(positions.data(), handle.gdp()->getNumPoints());
    }

    if (!cvex.run(handle.gdp()->getNumPoints(), false)) {
        std::cerr << cvex.getLastError() << "\n";
        std::cerr << cvex.getVexErrors() << "\n";
    }

    handle.gdpNC()->setPos3FromArray(point_range, positions);

    handle.gdp()->computeQuickBounds(myBox);

    // Add geometry to mantra.
    if (myVelocityBlur)
    {
        // If performing velocity blur, then we add velocity blur geometry
        g0.addVelocityBlur(myPreBlur, myPostBlur);
    }
    else
    {
        // Otherwise, we check to see if there's a motion blur geometry file
//        if (myBlurFile.isstring())
//        {
//            auto        g1 = g0.appendSegmentGeometry(myShutter);
//            GU_DetailHandleAutoWriteLock        wlock(g1);
//            if (!wlock.getGdp()->load(myBlurFile, 0).success())
//            {
//                fprintf(stderr, "Unable to load geometry[1]: '%s'\n",
//                        myBlurFile.c_str());
//                g0.removeSegmentGeometry(g1);
//            }
//        }
    }
    RAY_ProceduralChildPtr      obj = createChild();
    obj->addGeometry(g0);
}

