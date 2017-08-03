#pragma once

#include "ofMain.h"

class Calibration {
  public:
    Calibration();

    bool flipX;
    bool flipY;
    bool flipZ;
    float angle;
    ofVec3f position;
    ofQuaternion rotation;
};
