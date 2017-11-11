#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofSetWindowTitle("Alice == Painter");

  //Syphon stuff
  mainOutputSyphonServer.setName("Screen Output");
  frame.allocate(2770,720, GL_RGBA);
  syphonOn = true;


  //Midi stuff
  midiIn.openPort("nanoKONTROL2 SLIDER/KNOB");
  midiIn.ignoreTypes(false, false, false);
  midiIn.addListener(this);

  textFont.load("Sequel_Demo.ttf", 70);

	//ofHideCursor();

  //post processing initialization
  setupPost();
  setupGUI();

  warp->setAmplitude(0);

  //Text positions
  textCenter.x = 1000;
  textCenter.y = -100;
  textCenter.z = 100;
  textBoxWidth = 4000;
  textBoxHeight = 2000;
  textBoxDepth = 4000;
  showCube = false;
  textErase = false;



  //Kinect initialization
  ofxKinect kinectCounter;
  kinects = kinectCounter.numTotalDevices();
  kinectCounter.close();

  clouds = new PointCloud[kinects];

  for(int i=0; i<kinects; i++)
  {
    clouds[i].init(i);
    clouds[i].loadCalibration(clouds[i].kinect->getSerial() + ".json");
  }

  //Camera initialization
  //initialCameraPosition->x = 0.0;
  //initialCameraPosition->y = 0.0;
  //initialCameraPosition->z = 1800;
  resettingCamera = false;
  cam = ofCamera();
  cam.setFarClip(100000);
  cam.setFov(fov);
  cam.setGlobalPosition(initialCameraPosition->x, initialCameraPosition->y, initialCameraPosition->z);
  cam.lookAt(orbitCenterPoint);
  initialCameraOrientation = cam.getGlobalOrientation();

  up = false;
  down = false;
  left = false;
  right = false;
  fly = false;
  land = false;
  pMouseX = mouseX;
  pMouseY = mouseY;
  mouseControl = false;
  camOrbit = false;
	orbitLatitude = -90;
  orbitLongitude = 0;
  orbitRadius = 5800;
  orbitSpeed.x = 0.01;
  shift = false;
  //targetOrbitSpeed = 0.1;


  synonyms = false;
  listening = false;


  //Debug code
  debug = false;
  calibration = false;
  selectedCloud = 0;

  //SocketIO Stuff
	isConnected = false;
	std::string address = "http://localhost:8080";
	socketIO.setup(address);
	ofAddListener(socketIO.connectionEvent, this, &ofApp::onConnection);
  ofAddListener(socketIO.notifyEvent, this, &ofApp::onNotice);

  //Alice model
  setupModel();

  //Shots
  setupShots("shots_");
  directorMode = false;

}

void ofApp::setupShots(string filePrefix){
  ofxJSON jsonObject;

  for(int i=0; i<9; i++)
  {
    string filename = filePrefix + to_string(i) + ".json";

    string path = "/Users/davidjonas/Documents/Freelance/OpenFrameworks/apps/myApps/Painter/bin/data/" + filename;

    ofVec3f camPos;
    ofVec3f t;

    if (FILE *file = fopen(path.c_str(), "r"))
    {
      fclose(file);
      bool parsingSuccessful = jsonObject.open(path);

      try {
        if (parsingSuccessful)
        {
          ofLogNotice("Shots: ", "Loading shot "+ to_string(i) +"...");
          camPos.x = jsonObject["position"][0].asFloat();
          camPos.y = jsonObject["position"][1].asFloat();
          camPos.z = jsonObject["position"][2].asFloat();

          t.x = jsonObject["target"][0].asFloat();
          t.y = jsonObject["target"][1].asFloat();
          t.z = jsonObject["target"][2].asFloat();
        }
        else {
            ofLogNotice("Shots: ", "Parsing failed...");
        }
      } catch (exception msg)
      {
        ofLogNotice("Shots: ", "Loading failed...");
        ofLogNotice("Shots: %s", msg.what());
      }
    }
    else {
      ofLogNotice("Shots: ", "File not found...");
    }

    shots[i].cameraPosition = camPos;
    shots[i].target = t;
  }
}

void ofApp::saveShots(string filePrefix){
  ofxJSON jsonObject;

  for(int i=0; i<9; i++)
  {
    string filename = filePrefix + to_string(i) + ".json";
    ofstream ss;

    string path = "/Users/davidjonas/Documents/Freelance/OpenFrameworks/apps/myApps/Painter/bin/data/" + filename;

    ss.open(path, ios::out);

    if(ss.is_open()) {
      ofLogNotice("Shots: ", "Saving shots...");
      ss << "{\"position\": " << "[ " << shots[i].cameraPosition.x << ", " <<  shots[i].cameraPosition.y << ", " <<  shots[i].cameraPosition.z << "]" << ",";
      ss << "\"target\": " << "[ " << shots[i].target.x << ", " <<  shots[i].target.y << ", " << shots[i].target.z << "]";
      ss << "}";

      ss.close();
    }
    else
    {
      ofLogNotice("Shots: ", "Error opening file for writing...");
    }
  }
}

void ofApp::applyShot(int index){
  orbitCenterPoint = shots[index].target;
  orbitRadius = ofDist(cam.getGlobalPosition().x, cam.getGlobalPosition().y, cam.getGlobalPosition().z, orbitCenterPoint->x, orbitCenterPoint->y, orbitCenterPoint->z);

  cam.setGlobalPosition(shots[index].cameraPosition.x, shots[index].cameraPosition.y, shots[index].cameraPosition.z);
  cam.lookAt(shots[index].target, cam.getUpDir());
}

void ofApp::setShot(int index){
  shots[index].cameraPosition = cam.getGlobalPosition();
  shots[index].target = cam.getLookAtDir();
  shots[index].target = cam.getGlobalPosition() + (shots[index].target.getNormalized() * ofMap(focus, 988.0, 1000.0, 10, 5000));
}

void ofApp::setupModel(){
  model.init("AliceAnimated.fbx");
  model.setScale(500, 500, 500);
  model.setNoiseLevel(0.08);
	model.setPosition(ofVec3f(0, -1000, -1500));
  model.setColor(ofColor(255,80,80));
	model.setRotation(ofVec3f(0,180,0));
	model.setPointSize(1);
	model.setPointMode(true);
	//light.setPosition(orbitCenterPoint->x, orbitCenterPoint->y, orbitCenterPoint->z); //TODO:This is a test, it should be on setupLight function.
  ofLogNotice("PointModel", "Model setup successful");
}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	if(msg.control == 64 && msg.value == 127)
  {
    dofOn = !dofOn;
  }
  if(msg.control == 41 && msg.value == 127)
  {
    syphonOn = true;
  }
  if(msg.control == 42 && msg.value == 127)
  {
    syphonOn = false;
  }
  if(msg.control == 0)
  {
    focus = ofMap(msg.value, 0, 127,  988, 1000);
  }
  if(msg.control == 1)
  {
    aperture = ofMap(msg.value, 0, 127,  0.1, 20);
  }
  if(msg.control == 2)
  {
    warp->setAmplitude(ofMap(msg.value, 0, 127,  0, 0.2));
  }
  if(msg.control == 45 && msg.value == 127)
  {
    rgbImage = !rgbImage;
  }
  if(msg.control == 43)
  {
    if(msg.value == 127)
    {
      convolutionOn = true;
    }
    else
    {
      convolutionOn = false;
    }
  }
  if(msg.control == 44)
  {
    if(msg.value == 127)
    {
      textErase = true;
    }
    else
    {
      textErase = false;
    }
  }
  if(msg.control == 71 && msg.value == 127)
  {
    toggleCamOrbit();
  }
  if(msg.control == 67 && msg.value == 127)
  {
    contrastOn = !contrastOn;
  }
  if(msg.control == 66 && msg.value == 127)
  {
    warpOn = !warpOn;
  }
  if(msg.control == 46 && msg.value == 127)
  {
    clearSentences();
  }
  if(msg.control == 3)
  {
    contrastValue = ofMap(msg.value, 0, 127,  0.0, 2.0);
  }
  if(msg.control == 4)
  {
    brightnessValue = ofMap(msg.value, 0, 127,  0.0, 10.0);
  }
  if(msg.control == 5)
  {
    multipleValue = ofMap(msg.value, 0, 127,  0.0, 10.0);
  }
  if(msg.control == 20)
  {
    brightBoost = ofMap(msg.value, 0, 127,  0.0, 50);
  }
  if(msg.control == 22)
  {
    cameraResetSpeed = ofMap(msg.value, 0, 127,  0.01, 0.2);
  }
  if(msg.control == 19)
  {
    pointSize = ofMap(msg.value, 0, 127,  0.0, 50);
  }
  if(msg.control == 18)
  {
    sparcity = ofMap(msg.value, 0, 127,  1, 10);
  }
  if(msg.control == 55 && msg.value == 127)
  {
    targetOrbitSpeed = ofVec2f(0,targetOrbitSpeed->y);
  }
  if(msg.control == 54 && msg.value == 127)
  {
    targetOrbitSpeed = ofVec2f(targetOrbitSpeed->x, 0);
  }
  if(msg.control == 39 && msg.value == 127)
  {
    resettingCamera = !resettingCamera;
  }
  if(msg.control == 7)
  {
    orbitSpeed.x = ofMap(msg.value, 0, 127,  0, 20);
    targetOrbitSpeed = orbitSpeed;
  }
  if(msg.control == 6)
  {
    orbitSpeed.y = ofMap(msg.value, 0, 127,  0, 20);
    targetOrbitSpeed = orbitSpeed;
  }
  if(msg.control == 16)
  {
    fov = ofMap(msg.value, 0, 127,  8, 150);
    cam.setFov(fov);
  }
  if(msg.control == 17)
  {
    textEraseSpeed = ofMap(msg.value, 0, 127,  1.0, 10.0);
  }
  if(msg.control == 23)
  {
    orbitRadius = ofMap(msg.value, 0, 127,  12326.0, 20000.0);
  }

  if(msg.control == 21)
  {
    model.setScale(ofMap(msg.value, 0, 127,  0, 1000), ofMap(msg.value, 0, 127,  0, 1000), ofMap(msg.value, 0, 127,  0, 1000));
  }
}

void ofApp::setupGUI()
{
  guiActive = true;
  postGui.setup("Effects", "effectsSettings.xml", 10, 10);
  focusGui.setup("Focus", "focusSettings.xml", 220, 10);
  adjustmentGui.setup("Adjustment", "adjustmentSettings.xml", 440, 10);
  rotationGui.setup("Rotation", "rotationSettings.xml", 660, 10);
  colorGui.setup("Color", "colorSettings.xml", 880, 10);
  eraseGui.setup("Erase", "eraseSettings.xml", 1100, 10);
  aliceGui.setup("Alice", "aliceSettings.xml", 220, 120);

  aliceGui.add(aliceOn.setup("Alice", false));
  aliceGui.add(alicePosition.setup("Alice Position", ofVec3f(0, -1000, -1500), ofVec3f(0, -2000, -2000), ofVec3f(0, 2000, 2000)));
  aliceGui.add(aliceRotation.setup("Alice Rotation", ofVec3f(0, 0, 0), ofVec3f(0, 0, 0), ofVec3f(360, 360, 360)));
  aliceGui.add(aliceScale.setup("Alice scale", 300, 0, 1000));
  aliceGui.add(alicePointSize.setup("Alice pointSize", 1, 1, 5));
  aliceGui.add(aliceFade.setup("Alice fade", 0, 0, 255));


  focusGui.add(dofOn.setup("Depth of field", dof->getEnabled()));
  focusGui.add(focus.setup("Focus", 995, 988, 1000));
  focusGui.add(aperture.setup("Aperture", 0.6, 0.1, 20));
  focusGui.add(fov.setup("FOV", 16, 8, 150));


  adjustmentGui.add(contrastValue.setup("Contrast", 1, 0.0, 2));
  adjustmentGui.add(brightnessValue.setup("Brightness", 2, 0.0, 10));
  adjustmentGui.add(multipleValue.setup("Multiple", 1, 0.0, 10));
  adjustmentGui.add(brightBoost.setup("Bright Boost", 0, 0.0, 50));
  adjustmentGui.add(pointSize.setup("Point Size", 5, 0.0, 50));
  adjustmentGui.add(sparcity.setup("Sparcity", 2, 1, 10));

  rotationGui.add(orbitCenterPoint.setup("Center of rotation", ofVec3f(0,0,-1800), ofVec3f(-2000, -2000, -2000), ofVec3f(3000, 3000, 3000)));
  rotationGui.add(orbitRadius.setup("Radius", 12326, 12326, 50000));
  rotationGui.add(targetOrbitSpeed.setup("Orbit speed", ofVec2f(0.01,0.0), ofVec2f(0.0,0.0), ofVec2f(20.0,20.0)));
  rotationGui.add(initialCameraPosition.setup("Cam Zero Point", ofVec3f(0,0,1800), ofVec3f(-10000,-10000), ofVec3f(10000,10000)));
  rotationGui.add(cameraResetSpeed.setup("Reset speed", 0.01, 0.01, 0.2));
  rotationGui.add(depthFarClip.setup("Far Clip", 255, 0, 10000));
  rotationGui.add(globalAngle.setup("Global rotation angle", 0, 0, 30));
  rotationGui.add(globalAxis.setup("Global rotation axis", ofVec3f(0,0,1), ofVec3f(0,0,0), ofVec3f(1,1,1)));

  colorGui.add(color.setup("color", ofColor(100, 100, 100), ofColor(0, 0), ofColor(255, 255)));
  colorGui.add(rgbImage.setup("RGB Image", true));

  postGui.add(antiAliasOn.setup("Anti-alias", antiAlias->getEnabled()));
  postGui.add(bloomOn.setup("Bloom", bloom->getEnabled()));
  postGui.add(kaleidoscopeOn.setup("Kaleidoscope", kaleidoscope->getEnabled()));
  postGui.add(warpOn.setup("Noise Warp", warp->getEnabled()));
  postGui.add(convolutionOn.setup("Convolution", convolution->getEnabled()));
  postGui.add(bleachOn.setup("Bleach", bleach->getEnabled()));
  postGui.add(pixelateOn.setup("Pixelate", pixelate->getEnabled()));
  postGui.add(edgeOn.setup("Edge", edge->getEnabled()));
  postGui.add(tiltshiftOn.setup("Tilt Shift", tiltshift->getEnabled()));
  postGui.add(godraysOn.setup("God rays", godrays->getEnabled()));
  postGui.add(contrastOn.setup("Contrast", contrast->getEnabled()));

  eraseGui.add(textErase.setup("Text erase", false));
  eraseGui.add(textEraseSpeed.setup("Erase speed", 1.0, 1.0, 10.0));
  eraseGui.add(textOffset.setup("Text Offset", ofVec3f(0,0,0), ofVec3f(-4000,-4000,-4000), ofVec3f(4000,4000,4000)));

  postGui.loadFromFile("effectsSettings.xml");
  focusGui.loadFromFile("focusSettings.xml");
  adjustmentGui.loadFromFile("adjustmentSettings.xml");
  rotationGui.loadFromFile("rotationSettings.xml");
  colorGui.loadFromFile("colorSettings.xml");
  eraseGui.loadFromFile("eraseSettings.xml");
  aliceGui.loadFromFile("aliceSettings.xml");

}

void ofApp::setupPost()
{
  post.init(2770,720);

  dof = post.createPass<DofPass>();
  dof->setEnabled(true);
  dof->setAperture(0.6);
  dof->setFocus(0.995);

  antiAlias = post.createPass<FxaaPass>();
  antiAlias->setEnabled(false);

  bloom = post.createPass<BloomPass>();
  bloom->setEnabled(true);

  kaleidoscope = post.createPass<KaleidoscopePass>();
  kaleidoscope->setEnabled(false);

  warp = post.createPass<NoiseWarpPass>();
  warp->setEnabled(false);

  convolution = post.createPass<ConvolutionPass>();
  convolution->setEnabled(false);

  bleach = post.createPass<BleachBypassPass>();
  bleach->setEnabled(false);

  pixelate = post.createPass<PixelatePass>();
  pixelate->setEnabled(false);

  edge = post.createPass<EdgePass>();
  edge->setEnabled(false);

  tiltshift = post.createPass<VerticalTiltShifPass>();
  tiltshift->setEnabled(false);

  godrays = post.createPass<GodRaysPass>();
  godrays->setEnabled(false);

  contrast = post.createPass<ContrastPass>();
  contrast->setEnabled(false);

}

//SocketIO stuff
void ofApp::onConnection () {
  isConnected = true;
	ofLogNotice("ofxSocketIO", "Connected!");
  bindEvents();
  subscribeEvents();
}

void ofApp::onNotice(string& name)
{
  //ofLogNotice("ofxSocketIO NOTICE", name);
}

void ofApp::bindEvents () {
  std::string sentenceEventName = "sentence";
  socketIO.bindEvent(sentenceEvent, sentenceEventName);
  ofAddListener(sentenceEvent, this, &ofApp::onSentenceEvent);
}

void ofApp::subscribeEvents() {
  std::string msg = "subscribe";
  std::string type = "sentence";
  socketIO.emit(msg, type);
}

void ofApp::onSentenceEvent (ofxSocketIOData& data) {

  map<int, Sentence>::iterator it;

  int id = data.getIntValue("id");
  string txt = data.getStringValue("changed");

  it = sentences.find(id);
  if(it == sentences.end())
  {
    Sentence s;
    s.sentence = txt;
    s.position = ofVec3f(ofRandom(textCenter.z-textBoxDepth, textCenter.z+textBoxDepth), ofRandom(textCenter.y-textBoxHeight/2, textCenter.y+textBoxHeight/2), ofRandom(textCenter.x-textBoxWidth/2, textCenter.x+textBoxWidth/2));
    //s.position = ofVec3f(-800, -((int)sentences.size() * 150) + 400, -1500);
    //s.position = ofVec3f(0, 0, -1500);
    s.active = true;
    s.alpha = 255;
    sentences[id] = s;
  }
  else
  {
    sentences[id].sentence = txt;
  }
}

//--------------------------------------------------------------
void ofApp::update(){
  model.update();
  // if(synonyms)
  // {
  //     //handleTimers();
  // }

  if(resettingCamera)
  {
    resetCamera();
  }

  float scaledOrbitSpeed = (targetOrbitSpeed->x * targetOrbitSpeed->x)/20.0;

  //if(scaledOrbitSpeed - orbitSpeed.x > 0.001 || scaledOrbitSpeed - orbitSpeed.x < -0.001)
  //{
  //  orbitSpeed.x += (scaledOrbitSpeed - orbitSpeed.x) * 0.001;
  //}
  //else
  //{
  orbitSpeed.x = scaledOrbitSpeed;
  //}

  if(targetOrbitSpeed->y - orbitSpeed.y > 0.01 || targetOrbitSpeed->y - orbitSpeed.y < -0.01)
  {
    orbitSpeed.y += (targetOrbitSpeed->y - orbitSpeed.y) * 0.01;
  }
  else
  {
    orbitSpeed.y = targetOrbitSpeed->y;
  }

  if(orbitSpeed.y == 0)
  {
    orbitLongitude += (0 - orbitLongitude) * 0.01;
  }

  handleEraseText();
  GUIUpdate();
  handleCamera();
  for(int i=0; i<kinects; i++)
  {
    clouds[i].update();
  }
}

void ofApp::handleEraseText()
{
  if(textErase)
  {
    for (unsigned i=0; i < sentences.size(); i++)
    {
      if(sentences[i].active)
      {
        if(sentences[i].alpha <= 0)
        {
          sentences[i].active = false;
        }
        else
        {
          sentences[i].alpha -= textEraseSpeed;
          break;
        }
      }
    }
  }
}

void ofApp::GUIUpdate()
{
  dof->setFocus(focus/1000);
  dof->setAperture(aperture);

  contrast->setBrightness(brightnessValue);
  contrast->setContrast(contrastValue);
  contrast->setMultiple(multipleValue);

  dof->setEnabled(dofOn);
  antiAlias->setEnabled(antiAliasOn);
  bloom->setEnabled(bloomOn);
  kaleidoscope->setEnabled(kaleidoscopeOn);
  warp->setEnabled(warpOn);
  convolution->setEnabled(convolutionOn);
  bleach->setEnabled(bleachOn);
  pixelate->setEnabled(pixelateOn);
  edge->setEnabled(edgeOn);
  tiltshift->setEnabled(tiltshiftOn);
  godrays->setEnabled(godraysOn);
  contrast->setEnabled(contrastOn);

  model.setScale(aliceScale, aliceScale, aliceScale);
  model.setPosition(alicePosition->x, alicePosition->y, alicePosition->z);
  model.setRotation(aliceRotation->x, aliceRotation->y, aliceRotation->z);
  model.setColor(ofColor(aliceFade, aliceFade, aliceFade));
  model.setPointSize(alicePointSize);

  for(int i=0; i<kinects; i++)
  {
    clouds[i].brightBoost = brightBoost;
    clouds[i].setPointSize(pointSize);
    clouds[i].setSparcity(sparcity);
    clouds[i].depthFarClip = depthFarClip;
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofBackground(0);
  ofSetColor(255, 255, 255);

  if(syphonOn)  frame.begin();
  post.begin();
  cam.begin();
    //light.enable();

    ofPushMatrix();
      ofRotate(globalAngle, globalAxis->x, globalAxis->y, globalAxis->z);
      drawClouds();
    ofPopMatrix();

    ofNoFill();

    if(aliceOn)
    {
      model.draw();
    }

    if (showCube)
    {
      ofDrawSphere(textCenter.x, textCenter.y, textCenter.z, 50);
      ofDrawBox(textCenter.x, textCenter.y, textCenter.z, textBoxWidth, textBoxHeight, textBoxDepth);
    }

    ofVec3f t = cam.getLookAtDir();
    float f = ofMap(focus, 988.0, 1000.0, 10, 5000 );
    t = cam.getGlobalPosition() + (t.getNormalized() * f);

    if(directorMode)
    {
      ofSetColor(255,0,0);
      ofDrawSphere(t.x, t.y, t.z, 50);
    }

    //light.disable();
    drawSentences();
  cam.end();
  post.end();
  if(syphonOn)  frame.end();

  if(syphonOn)  mainOutputSyphonServer.publishTexture(&frame.getTexture());

  if(calibration)
  {
    drawCalibration();
  }

  if(directorMode)
  {
    ofDrawBitmapString("Target: " + to_string(t.x) + ", " + to_string(t.y) + "," + to_string(t.z) + " --> " + to_string(f), 100, 100);
    ofDrawBitmapString("Cam: " + to_string(cam.getGlobalPosition().x) + ", " + to_string(cam.getGlobalPosition().y) + "," + to_string(cam.getGlobalPosition().z), 100, 150);
  }

  if(debug)
  {
    drawDebug();
  }

  if(guiActive)
  {
    focusGui.draw();
    adjustmentGui.draw();
    rotationGui.draw();
    colorGui.draw();
    postGui.draw();
    eraseGui.draw();
    aliceGui.draw();
  }

  if(resettingCamera)
  {
    ofSetColor(255,0,0);
    ofDrawEllipse(ofGetWindowWidth() - 20, 10, 10, 10);
  }

  if(directorMode)
  {
    ofSetColor(0,255,0);
    ofDrawEllipse(ofGetWindowWidth() - 40, 10, 10, 10);
  }
}

void ofApp::drawClouds()
{
  for(int i=0; i<kinects; i++)
  {
    if(rgbImage)
    {
      clouds[i].draw();
    }
    else
    {
      clouds[i].draw(color);
    }
  }
}

void ofApp::drawSentences()
{
  ofPushMatrix();
  ofRotate(-90,0,1,0);
  ofTranslate(textOffset->x, textOffset->y, textOffset->z);
  for (unsigned i=0; i < sentences.size(); i++)
  {
    if(sentences[i].active)
    {
      ofPushMatrix();
      ofSetColor(255,255,255,sentences[i].alpha);
      ofTranslate(sentences[i].position);
      textFont.drawString(sentences[i].sentence, 0, 0);
      ofPopMatrix();
    }
  }
  ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::handleCamera()
{
  if(pMouseX != 0 || pMouseY !=0)
	{
		mouseDeltaX = mouseX - pMouseX;
		mouseDeltaY = mouseY - pMouseY;
	}
	else
	{
		mouseDeltaX = 0;
		mouseDeltaY = 0;
	}

	pMouseX = mouseX;
	pMouseY = mouseY;

	if(!camOrbit)
	{
    if(mouseControl && shift)
    {
  		cam.rotate(mouseDeltaX/10.0, 0.0, 1.0, 0.0);
  		cam.tilt(mouseDeltaY/10.0);
    }
	}
	else
	{
		orbitLatitude += orbitSpeed.x;
    orbitLongitude += orbitSpeed.y;
		cam.orbit(orbitLatitude, orbitLongitude, orbitRadius, orbitCenterPoint);
    //cam.setGlobalPosition(cam.getGlobalPosition().x, -1000, cam.getGlobalPosition().z);
	}


	if(up) cam.dolly(-50);
	if(down) cam.dolly(50);
	if(left) cam.truck(-50);
	if(right) cam.truck(50);
  if(fly) cam.boom(50);
  if(land) cam.boom(-50);
}

void ofApp::toggleCamOrbit()
{
  if(camOrbit)
  {
    camOrbit = false;
  }
  else
  {
    orbitRadius = ofDist(cam.getGlobalPosition().x, cam.getGlobalPosition().y, cam.getGlobalPosition().z, orbitCenterPoint->x, orbitCenterPoint->y, orbitCenterPoint->z);
    camOrbit = true;
  }
}

void ofApp::resetCamera()
{
  camOrbit = false;
  targetOrbitSpeed = ofVec2f(0.01,0);
  orbitLatitude = -90;
  orbitLongitude = 0;
  ofVec3f lerpPos = cam.getGlobalPosition();
  lerpPos.x += (initialCameraPosition->x - lerpPos.x) * cameraResetSpeed;
	lerpPos.y += (initialCameraPosition->y - lerpPos.y) * cameraResetSpeed;
	lerpPos.z += (initialCameraPosition->z - lerpPos.z) * cameraResetSpeed;
  cam.setGlobalPosition(lerpPos);
  ofQuaternion tweenedCameraQuaternion;
  tweenedCameraQuaternion.slerp(cameraResetSpeed, cam.getGlobalOrientation(), initialCameraOrientation);
  cam.setGlobalOrientation(tweenedCameraQuaternion);
}

void ofApp::clearSentences()
{
  sentences.clear();
}

//--------------------------------------------------------------
void ofApp::drawDebug(){
  ofDrawBitmapString("Number of kinects detected: " + to_string(kinects), 10, 10);


  //displaying depth and RGB from all cameras
  ofVec2f point(30,20);
  for(int i=0; i<kinects; i++)
  {
    clouds[i].drawRGB(point.x, point.y, 0.5);
    clouds[i].drawDepth(point.x + clouds[i].getWidth()/2 + 10, point.y, 0.5);

    point.y += clouds[i].getHeight()/2 + 20;
  }

  //Calibration data
  drawCalibration();
}

void ofApp::drawCalibration()
{
  int y=0;
  for(int i=0; i<kinects; i++)
  {
      if(selectedCloud == i)
      {
        ofSetColor(255,0,0);
      }
      else
      {
        ofSetColor(255);
      }
      ofDrawBitmapString("Calibration data for kinect: " + clouds[i].kinect->getSerial(), ofGetWindowWidth()-450, y+10);
      ofDrawBitmapString("Position: (" + to_string(clouds[i].getPosition().x) + ", " + to_string(clouds[i].getPosition().y) + ", " + to_string(clouds[i].getPosition().z) + ")", ofGetWindowWidth()-450, y+30);
      ofDrawBitmapString("Rotation: (" + to_string(clouds[i].getRotation().x()) + ", " + to_string(clouds[i].getRotation().y()) + ", " + to_string(clouds[i].getRotation().z()) + ", " + to_string(clouds[i].getRotation().w()) + ")", ofGetWindowWidth()-450, y+50);
      ofDrawBitmapString("FlipX: " + string(clouds[i].getFlipX() ? "true" : "false"), ofGetWindowWidth()-450, y+70);
      ofDrawBitmapString("FlipY: " + string(clouds[i].getFlipY() ? "true" : "false"), ofGetWindowWidth()-450, y+90);
      ofDrawBitmapString("FlipZ: " + string(clouds[i].getFlipZ() ? "true" : "false"), ofGetWindowWidth()-450, y+110);

      y += 130;
  }

  ofSetColor(255);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  switch(key)
  {
    case '0':
      resetCamera();
      break;
    case 'p':
      debug = !debug;
      break;

    case 'o':
      toggleCamOrbit();
      break;

    case 'c':
      calibration = !calibration;
      break;

    case 'w':
			up=true;
			break;

		case 's':
			down=true;
			break;

		case 'a':
			left=true;
			break;

		case 'd':
			right=true;
			break;

    case 'r':
      fly = true;
      break;

    case 'f':
      land = true;
      break;

    case 'u':
      fov = fov + 1;
      cam.setFov(fov);
      break;
    case 'j':
      fov = fov -1;
      cam.setFov(fov);
      break;
    case '/':
      synonyms = !synonyms;
      break;
    case '.':
      listening = !listening;
      break;
    case ',':
      clearSentences();
      break;
    case '\t':
      guiActive = !guiActive;
      break;
    case 'z':
      shift = true;
      break;
    case 'm':
      initialCameraPosition = cam.getGlobalPosition();
      break;
    case 'g':
      directorMode = !directorMode;
      break;
  }

  if(!calibration && !directorMode)
  {
    switch(key)
    {
      case '1':
        applyShot(0);
        break;

      case '2':
        applyShot(1);
        break;

      case '3':
        applyShot(2);
        break;

      case '4':
        applyShot(3);
        break;

      case '5':
        applyShot(4);
        break;

      case '6':
        applyShot(5);
        break;

      case '7':
        applyShot(6);
        break;

      case '8':
        applyShot(7);
        break;

      case '9':
        applyShot(8);
        break;
    }
  }

  if(directorMode)
  {
    switch(key)
    {
      case '1':
        setShot(0);
        break;

      case '2':
        setShot(1);
        break;

      case '3':
        setShot(2);
        break;

      case '4':
        setShot(3);
        break;

      case '5':
        setShot(4);
        break;

      case '6':
        setShot(5);
        break;

      case '7':
        setShot(6);
        break;

      case '8':
        setShot(7);
        break;

      case '9':
        setShot(8);
        break;

      case ' ':
        saveShots("shots_");
        break;
    }
  }

  if(calibration)
  {
    switch(key)
    {
      case '1':
        selectedCloud = 0;
        break;

      case '2':
        selectedCloud = 1;
        break;

      case '3':
        selectedCloud = 2;
        break;

      case 'x':
        if(selectedCloud < kinects)
        {
          clouds[selectedCloud].setFlip(!clouds[selectedCloud].getFlipX(), clouds[selectedCloud].getFlipY(), clouds[selectedCloud].getFlipZ());
        }
        break;

      case 'y':
        if(selectedCloud < kinects)
        {
          clouds[selectedCloud].setFlip(clouds[selectedCloud].getFlipX(), !clouds[selectedCloud].getFlipY(), clouds[selectedCloud].getFlipZ());
        }
        break;

      case 'z':
        if(selectedCloud < kinects)
        {
          clouds[selectedCloud].setFlip(clouds[selectedCloud].getFlipX(), clouds[selectedCloud].getFlipY(), !clouds[selectedCloud].getFlipZ());
        }
        break;

      case OF_KEY_UP:
        clouds[selectedCloud].addZCalibration(10);
  			break;

  		case OF_KEY_DOWN:
        clouds[selectedCloud].addZCalibration(-10);
  			break;

  		case OF_KEY_LEFT:
        clouds[selectedCloud].addXCalibration(-10);
  			break;

  		case OF_KEY_RIGHT:
        clouds[selectedCloud].addXCalibration(10);
  			break;

      case '[':
        clouds[selectedCloud].addYCalibration(-10);
  			break;

  		case '\'':
        clouds[selectedCloud].addYCalibration(10);
  			break;

      case ';':
        clouds[selectedCloud].addYRotationCalibration(-0.05);
  			break;

    	case '\\':
        clouds[selectedCloud].addYRotationCalibration(0.05);
  			break;

      case ' ':
        clouds[selectedCloud].saveCalibration(clouds[selectedCloud].kinect->getSerial() + ".json");
        break;

      case 'i':
        for(int i=0; i<kinects; i++)
        {
          clouds[i].kinectAngle++;
    			if(clouds[i].kinectAngle>30) clouds[i].kinectAngle=30;
          clouds[i].updateCameraTiltAngle();
        }
        //cout << clouds[0].kinectAngle;
  			break;

  		case 'k':
        for(int i=0; i<kinects; i++)
        {
          clouds[i].kinectAngle--;
          if(clouds[i].kinectAngle<-30) clouds[i].kinectAngle=-30;
          clouds[i].updateCameraTiltAngle();
        }
        //cout << clouds[0].kinectAngle;
  			break;
    }
  }

  if(camOrbit)
  {
    switch(key)
    {
      case '=':
        //orbitRadius -= 100;
        //targetOrbitSpeed->x += 0.2;
        break;
      case '-':
        //orbitRadius += 100;
        //targetOrbitSpeed->x -= 0.2;
        break;
    }
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
  switch (key) {
		case 'w':
			up=false;
			break;

		case 's':
			down=false;
			break;

		case 'a':
			left=false;
			break;

		case 'd':
			right=false;
			break;

    case 'r':
      fly = false;
      break;

    case 'f':
      land = false;
      break;

    case 'z':
      shift = false;
      break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
  mouseControl = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
  mouseControl = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

void ofApp::exit() {
  socketIO.closeConnection();;
  for(int i=0; i<kinects; i++)
  {
    clouds[i].close();
  }
}
