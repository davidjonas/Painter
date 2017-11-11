#include "PointModel.h"

PointModel::PointModel()
{
  initialized = false;
}

PointModel::PointModel(string filename)
{
  init(filename);
}

void PointModel::init(string filename)
{
  initialized = true;
  model.loadModel(filename, 20);
  mesh = model.getMesh(0);
  //mesh.setMode(OF_PRIMITIVE_POINTS);
  pointSize = 3;
  noiseLevel = 1;
  pointColor = ofColor(128,128,128);
  scale.x = 1;
  scale.y = 1;
  scale.z = 1;
  pointMode = true;

  if(model.hasAnimations())
  {
    model.setLoopStateForAllAnimations(OF_LOOP_NORMAL);
    model.playAllAnimations();
    ofLogNotice("PointModel", "Animation loaded");
  }
}

void PointModel::setPointSize(int size){
  pointSize = size;
}

void PointModel::setNoiseLevel(float noise){
  noiseLevel = noise;
}

void PointModel::setColor(ofColor color){
  pointColor = color;
}

void PointModel::setScale(ofVec3f scaleMultipliers){
  scale = scaleMultipliers;
}

void PointModel::setScale(float x, float y, float z){
  scale.x = x;
  scale.y = y;
  scale.z = z;
}

void PointModel::setPosition(ofVec3f point){
  position = point;
}

void PointModel::setPosition(float x, float y, float z)
{
  position.x = x;
  position.y = y;
  position.z = z;
}

void PointModel::setPointMode(bool value)
{
  pointMode = value;
}

ofVec3f PointModel::getPosition(){
  return position;
}

void PointModel::setRotation(ofVec3f euler){
  rotation = euler;
}

void PointModel::setRotation(float x, float y, float z){
  rotation.x = x;
  rotation.y = y;
  rotation.z = z;
}

void PointModel::update()
{
  model.update();
}

void PointModel::draw(){
  ofEnableDepthTest();
    ofPushMatrix();
      glPointSize(pointSize);
      ofSetColor(pointColor);

      ofTranslate(position.x, position.y, position.z);
      ofRotate(rotation.x, 1,0,0);
      ofRotate(rotation.y, 0,1,0);
      ofRotate(rotation.z, 0,0,1);
      ofScale(scale.x, scale.y, scale.z);


      for(int m=0; m<model.getNumMeshes(); m++)
      {
        if(model.hasAnimations())
        {
          mesh = model.getCurrentAnimatedMesh(m);
        }
        else
        {
          mesh = model.getMesh(m);
        }
        for(int i=0; i<mesh.getNumVertices(); i++){
            mesh.setVertex(i, mesh.getVertex(i) + ofVec3f(ofRandomuf() * noiseLevel,ofRandomuf() * noiseLevel,ofRandomuf() * noiseLevel));
        }

        if(pointMode)
        {
          mesh.drawVertices();
        }
        else
        {
          mesh.drawWireframe();
        }
      }

    ofPopMatrix();
  ofDisableDepthTest();
}
