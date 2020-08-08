//
// Created by symek on 8/8/20.
//

#pragma once

#include <UT/UT_BoundingBox.h>
#include <UT/UT_String.h>
#include <RAY/RAY_Procedural.h>

namespace HDK_Sample {
/// @brief A procedural which does a deferred load of geometry from disk
    class RAY_DemoFile : public RAY_Procedural {
    public:
        RAY_DemoFile();
        virtual ~RAY_DemoFile();
        virtual const char  *className() const;
        virtual int          initialize(const UT_BoundingBox *);
        virtual void         getBoundingBox(UT_BoundingBox &box);
        virtual void         render();
    private:
        UT_BoundingBox       myBox;
        UT_StringHolder      myFile, myBlurFile;
        fpreal               myShutter;
        bool                 myVelocityBlur;
        fpreal               myPreBlur, myPostBlur;
    };
}       // End HDK_Sample namespace

