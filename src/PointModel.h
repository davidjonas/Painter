#pragma once

#include "ofxAssimpModelLoader.h"

class PointModel {
  public:
    PointModel();
    PointModel(string filename);
    void init(string filename);
    void setPointSize(int size);
    void setNoiseLevel(float noise);
    void setColor(ofColor color);
    void setScale(ofVec3f scaleMultipliers);
    void setScale(float x, float y, float z);
    void setPosition(ofVec3f point);
    void setPointMode(bool value);
    ofVec3f getPosition();
    void setRotation(ofVec3f euler);
    void draw();

  private:
    ofxAssimpModelLoader model;
    ofMesh mesh;
    int pointSize;
    float noiseLevel;
    ofColor pointColor;
    ofVec3f scale;
    ofVec3f position;
    ofVec3f rotation;
    bool initialized;
    bool pointMode;
};
