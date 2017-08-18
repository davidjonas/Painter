#pragma once

#include "Calibration.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxJSON.h"

class PointCloud {
  private:
    int kinectIndex;
    int width;
    int height;
    int pointSize;
    int sparcity;
    Calibration calibration;
    ofMesh mesh;


  public:
    ofxKinect * kinect;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage depthImage;
    int kinectAngle;
    float brightBoost;

    ofxJSON jsonObject;

    PointCloud();
    PointCloud(int index);
    ~PointCloud();

    void init(int index);

    int getKinectIndex();
    void updateCameraTiltAngle();

    void setPointSize(int size);
    int getPointSize();

    void setSparcity(int value);
    int getSparcity();

    void setFlip(bool x, bool y, bool z);
    bool getFlipX();
    bool getFlipY();
    bool getFlipZ();

    void addXCalibration(float value);
    void addYCalibration(float value);
    void addZCalibration(float value);

    ofVec3f getPosition();
    void setPosition(ofVec3f cal);

    void addYRotationCalibration(float value);

    ofQuaternion getRotation();

    int getWidth();
    int getHeight();

    void update();
    void draw();
    void draw(ofColor c);
    void drawDepth(int x, int y);
    void drawRGB(int x, int y);
    void drawDepth(int x, int y, float scale);
    void drawRGB(int x, int y, float scale);
    ofMesh getMesh();
    ofVec3f * getPointCloud();

    void saveCalibration(string filename);

    bool loadCalibration(string filename);

    void close();
};
