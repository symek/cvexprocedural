//
// Created by symek on 8/8/20.
//

#pragma once

#include <UT/UT_BoundingBox.h>
#include <UT/UT_String.h>
#include <RAY/RAY_Procedural.h>

namespace HA_SKK {
/// @brief A procedural which does a deferred load of geometry from disk
    class RAY_CVexProcedural : public RAY_Procedural {
    public:
        RAY_CVexProcedural();
        virtual ~RAY_CVexProcedural();
        virtual const char  *className() const;
        virtual int          initialize(const UT_BoundingBox *);
        virtual void         getBoundingBox(UT_BoundingBox &box);
        virtual void         render();
    private:
        UT_BoundingBox       myBox;
        UT_StringHolder      myFile, myVexFile;
        fpreal               myShutter;
        bool                 myVelocityBlur;
        fpreal               myPreBlur, myPostBlur;
    };
}       // End HA_SKK namespace

