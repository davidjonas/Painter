#include "PointCloud.h"

//#define KINECTANGLEDEFAULT -18;
#define KINECTANGLEDEFAULT 0;

PointCloud::PointCloud()
{
  pointSize = 5;
  sparcity = 2;
  kinectAngle = KINECTANGLEDEFAULT;
  mesh.setMode(OF_PRIMITIVE_POINTS);
  calibration.flipX = true;
  calibration.flipY = true;
  calibration.flipZ = true;
  calibration.angle = 0;
}

PointCloud::PointCloud(int index)
{
  kinectIndex = index;
  kinect = new ofxKinect();
  kinect->setRegistration(true);
  kinect->init();
  kinect->open(kinectIndex);
  kinectAngle = KINECTANGLEDEFAULT;

  width = kinect->width;
  height = kinect->height;

  pointSize = 5;
  sparcity = 2;
  mesh.setMode(OF_PRIMITIVE_POINTS);
  calibration.flipX = true;
  calibration.flipY = true;
  calibration.flipZ = true;
  calibration.angle = 0;
}

PointCloud::~PointCloud()
{
  kinect->close();
  delete kinect;
}

void PointCloud::init(int index)
{
  kinectIndex = index;
  kinect = new ofxKinect();
  kinect->setRegistration(true);
  kinect->init();
  kinect->open(kinectIndex);

  width = kinect->width;
  height = kinect->height;
  updateCameraTiltAngle();
}

int PointCloud::getKinectIndex()
{
  return kinectIndex;
}

void PointCloud::updateCameraTiltAngle()
{
  kinect->setCameraTiltAngle(kinectAngle);
}

void PointCloud::setPointSize(int size)
{
  pointSize = size;
}

int PointCloud::getPointSize()
{
  return pointSize;
}

void PointCloud::setSparcity(int value)
{
  sparcity = value;
}

int PointCloud::getSparcity()
{
  return sparcity;
}

void PointCloud::setFlip(bool x, bool y, bool z)
{
  calibration.flipX = x;
  calibration.flipY = y;
  calibration.flipZ = z;
}

bool PointCloud::getFlipX()
{
  return calibration.flipX;
}

bool PointCloud::getFlipY()
{
  return calibration.flipY;
}

bool PointCloud::getFlipZ()
{
  return calibration.flipZ;
}

void PointCloud::setPosition(ofVec3f cal)
{
  calibration.position = cal;
}

void PointCloud::addXCalibration(float value)
{
  ofQuaternion qtAdd(value, 1, 0, 0);
  calibration.position.x += value;
}

void PointCloud::addYCalibration(float value)
{
  calibration.position.y += value;
}

void PointCloud::addZCalibration(float value)
{
  calibration.position.z += value;
}

ofVec3f PointCloud::getPosition()
{
  return calibration.position;
}

void PointCloud::addYRotationCalibration(float value)
{
  calibration.angle += value;
  ofQuaternion qtAdd(calibration.angle, ofVec3f(1,0,0));
  calibration.rotation = qtAdd;
}

ofQuaternion PointCloud::getRotation()
{
  return calibration.rotation;
}

int PointCloud::getWidth()
{
  return width;
}

int PointCloud::getHeight()
{
  return height;
}

void PointCloud::update()
{
  kinect->update();
}

void PointCloud::draw()
{
  mesh.clear();

  for(int y = 0; y < height; y += sparcity) {
  	for(int x = 0; x < width; x += sparcity) {
  		if(kinect->getDistanceAt(x, y) > 0) {
  			ofColor c = kinect->getColorAt(x,y);
				c.setBrightness(c.getBrightness() + 30);
  			mesh.addColor(c);
  			mesh.addVertex(kinect->getWorldCoordinateAt(x, y));
  		}
  	}
  }

  glPointSize(pointSize);
	ofPushMatrix();

  ofVec3f qaxis;
  float qangle;
  calibration.rotation.getRotate(qangle, qaxis);
  ofRotate(qangle, qaxis.x, qaxis.y, qaxis.z);

	ofScale(calibration.flipX ? -1 : 1, calibration.flipY ? -1 : 1, calibration.flipZ ? -1 : 1);
	ofTranslate(0 + calibration.position.x, 0 + calibration.position.y, -1000 + calibration.position.z); // center the points a bit
	ofEnableDepthTest();
	mesh.drawVertices();
	ofDisableDepthTest();
	ofPopMatrix();
}

void PointCloud::draw(ofColor c)
{
  mesh.clear();

  for(int y = 0; y < height; y += sparcity) {
  	for(int x = 0; x < width; x += sparcity) {
  		if(kinect->getDistanceAt(x, y) > 0) {
  			mesh.addColor(c);
        c.setBrightness(c.getBrightness() + 30);
  			mesh.addVertex(kinect->getWorldCoordinateAt(x, y));
  		}
  	}
  }

  glPointSize(pointSize);
	ofPushMatrix();

  ofVec3f qaxis;
  float qangle;
  calibration.rotation.getRotate(qangle, qaxis);
  ofRotate(qangle, qaxis.x, qaxis.y, qaxis.z);

	// the projected points are 'upside down' and 'backwards'
	ofScale(calibration.flipX ? -1 : 1, calibration.flipY ? -1 : 1, calibration.flipZ ? -1 : 1);
	ofTranslate(0 + calibration.position.x, 0 + calibration.position.y, -1000 + calibration.position.z); // center the points a bit
	ofEnableDepthTest();
	mesh.drawVertices();
	ofDisableDepthTest();
	ofPopMatrix();
}

void PointCloud::drawDepth(int x, int y)
{
  kinect->drawDepth(x, y, width, height);
}

void PointCloud::drawRGB(int x, int y)
{
  kinect->draw(x, y, width, height);
}

void PointCloud::drawDepth(int x, int y, float scale)
{
  kinect->drawDepth(x, y, width*scale, height*scale);
}

void PointCloud::drawRGB(int x, int y, float scale)
{
  kinect->draw(x, y, width*scale, height*scale);
}

ofMesh PointCloud::getMesh()
{
  return mesh;
}

ofVec3f * PointCloud::getPointCloud()
{
  ofVec3f * result = new ofVec3f[height*width/sparcity];
  return result;
}

void PointCloud::saveCalibration(string filename)
{
  ofstream ss;
  string path = "/Users/davidjonas/Documents/Freelance/OpenFrameworks/apps/myApps/Painter/bin/data/" + filename;

  ss.open(path, ios::out);

  if(ss.is_open()) {
    ofLogNotice("Calibration: ", "Saving calibration...");
    ss << "{ \"flipX\": " << calibration.flipX << ", ";
    ss << "\"flipY\": " << calibration.flipY << ", ";
    ss << "\"flipZ\": " << calibration.flipZ << ", ";
    ss << "\"position\": " << "[ " << calibration.position.x << ", " <<  calibration.position.y << ", " <<  calibration.position.z << "]" << ",";
    ss << "\"angle\": " << calibration.angle;
    ss << "}";

    ss.close();
  }
  else
  {
    cout << "ERROR: failed to open file.";
  }
}

bool PointCloud::loadCalibration(string filename)
{
  string path = "/Users/davidjonas/Documents/Freelance/OpenFrameworks/apps/myApps/Painter/bin/data/" + filename;

  if (FILE *file = fopen(path.c_str(), "r"))
  {
    fclose(file);
    bool parsingSuccessful = jsonObject.open(path);

    try {
      if (parsingSuccessful)
      {
        ofLogNotice("Calibration: ", "Loading calibration...");
        calibration.flipX = jsonObject["flipX"].asInt() == 1;
        calibration.flipY = jsonObject["flipY"].asInt() == 1;
        calibration.flipZ = jsonObject["flipZ"].asInt() == 1;

        calibration.position.x = jsonObject["position"][0].asFloat();
        calibration.position.y = jsonObject["position"][1].asFloat();
        calibration.position.z = jsonObject["position"][2].asFloat();

        calibration.angle = jsonObject["angle"].asFloat();
      }
    } catch (exception msg)
    {
      ofLogNotice("Calibration: ", "Calibration failed...");
      ofLogNotice("Calibration: %s", msg.what());
      return false;
    }

      return parsingSuccessful;
  }
  else
  {
    return false;
  }
}

void PointCloud::close()
{
  kinect->close();
}
