#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofSetWindowTitle("Alice == Painter");

  //Syphon stuff
  mainOutputSyphonServer.setName("Screen Output");
  frame.allocate(2950,800, GL_RGBA);
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

  //Text positions
  textCenter.x = -500;
  textCenter.y = 500;
  textCenter.z = -1500;
  textBoxWidth = 4000;
  textBoxHeight = 2000;
  textBoxDepth = 4000;
  showCube = false;



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
  initialCameraPosition.x = 0.0;
  initialCameraPosition.y = 0.0;
  initialCameraPosition.z = 1800;
  cam = ofCamera();
  cam.setFarClip(100000);
  fov = 30;
  cam.setFov(fov);
  cam.setGlobalPosition(initialCameraPosition.x, initialCameraPosition.y, initialCameraPosition.z);

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
	orbitLatitude = 0;
  orbitLongitude = 0;
  orbitRadius = 5800;
  orbitSpeed.x = 0.1;
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

  //timer
  delayTimer.setPeriod(1);
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
  if(msg.control == 71 && msg.value == 127)
  {
    camOrbit = !camOrbit;
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
    showCube = !showCube;
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
  if(msg.control == 19)
  {
    pointSize = ofMap(msg.value, 0, 127,  0.0, 50);
  }
  if(msg.control == 55 && msg.value == 127)
  {
    targetOrbitSpeed = ofVec2f(0,targetOrbitSpeed->y);
  }
  if(msg.control == 54 && msg.value == 127)
  {
    targetOrbitSpeed = ofVec2f(targetOrbitSpeed->x, 0);
  }
  if(msg.control == 7)
  {
    orbitSpeed.x = ofMap(msg.value, 0, 127,  -10, 10);
    targetOrbitSpeed = orbitSpeed;
  }
  if(msg.control == 6)
  {
    orbitSpeed.y = ofMap(msg.value, 0, 127,  -10, 10);
    targetOrbitSpeed = orbitSpeed;
  }
  if(msg.control == 16)
  {
    fov = ofMap(msg.value, 0, 127,  16, 70);
    cam.setFov(fov);
  }
  if(msg.control == 23)
  {
    orbitRadius = ofMap(msg.value, 0, 127,  10.0, 10000.0);
  }
}

void ofApp::setupGUI()
{
  guiActive = true;
  focusGui.setup("Focus", "focusSettings.xml", 10, 260);
  adjustmentGui.setup("Adjustment", "adjustmentSettings.xml", 100, 260);
  rotationGui.setup("Rotation", "rotationSettings.xml", 200, 260);
  colorGui.setup("Color", "colorSettings.xml", 300, 260);


  focusGui.add(dofOn.setup("Depth of field", dof->getEnabled()));
  focusGui.add(focus.setup("Focus", 995, 988, 1000));
  focusGui.add(aperture.setup("Aperture", 0.6, 0.1, 20));

  adjustmentGui.add(contrastValue.setup("Contrast", 1, 0.0, 2));
  adjustmentGui.add(brightnessValue.setup("Brightness", 2, 0.0, 10));
  adjustmentGui.add(multipleValue.setup("Multiple", 1, 0.0, 10));
  adjustmentGui.add(brightBoost.setup("Bright Boost", 0, 0.0, 50));
  adjustmentGui.add(pointSize.setup("pointSize", 5, 0.0, 50));

  rotationGui.add(orbitCenterPoint.setup("Center of rotation", ofVec3f(0,0,-1800), ofVec3f(-2000, -2000, -2000), ofVec3f(3000, 3000, 3000)));
  rotationGui.add(orbitRadius.setup("Radius", 5800, 10, 10000));
  rotationGui.add(targetOrbitSpeed.setup("Orbit speed", ofVec2f(0.1,0.0), ofVec2f(-10.0,-10.0), ofVec2f(10.0,10.0)));

  colorGui.add(color.setup("color", ofColor(100, 100, 100), ofColor(0, 0), ofColor(255, 255)));
  colorGui.add(rgbImage.setup("RGB Image", true));


  postGui.setup("Effects", "effectsSettings.xml", 10, 10);
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
}

void ofApp::setupPost()
{
  post.init(2950,800);

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
  //std::string serverEventName = "speech";
  //socketIO.bindEvent(speechEvent, serverEventName);
  //ofAddListener(speechEvent, this, &ofApp::onSpeechEvent);

  std::string sentenceEventName = "sentence";
  socketIO.bindEvent(sentenceEvent, sentenceEventName);
  ofAddListener(sentenceEvent, this, &ofApp::onSentenceEvent);
}

void ofApp::subscribeEvents() {
  std::string msg = "subscribe";
  std::string type = "sentence";
  socketIO.emit(msg, type);

  //std::string type2 = "speech";
  //socketIO.emit(msg, type2);
}

//NOTE: Not called anymore, oonly receiving sentence events
void ofApp::onSpeechEvent (ofxSocketIOData& data) {
  if(listening)
  {
    //Sentence s;
    //s.sentence = data.getStringValue("msg");
    //s.position = ofVec3f(ofRandom(-1000, 1000), ofRandom(-2000, 2000), ofRandom(-6000, -1000));
    //s.position = ofVec3f(-800, -((int)sentences.size() * 150) + 400, -1000);
    ////s.delayTimer.setPeriod(ofRandom(3, 7));
    //sentences.push_back(s);
  }
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
    //s.position = ofVec3f(ofRandom(textCenter.x-textBoxWidth/2, textCenter.x+textBoxWidth/2), ofRandom(textCenter.y-textBoxHeight/2, textCenter.y+textBoxHeight/2), ofRandom(textCenter.z-textBoxDepth/2, textCenter.z+textBoxDepth/2));
    s.position = ofVec3f(-800, -((int)sentences.size() * 150) + 400, -1500);
    //s.position = ofVec3f(0, 0, -1500);
    sentences[id] = s;
  }
  else
  {
    sentences[id].sentence = txt;
  }
}

void ofApp::handleTimers()
{
    if(delayTimer.tick() && sentences.size()>0)
    {
      unsigned i = roundf(ofRandom(0, sentences.size()-1));
      string msg = "{\"msg\":\""+ sentences[i].sentence +"\", \"id\":" + to_string(i) + "}";
      string evtName = "process_request";
      socketIO.emit(evtName, msg);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
  if(synonyms)
  {
      //handleTimers();
  }

  if(targetOrbitSpeed->x - orbitSpeed.x > 0.1 || targetOrbitSpeed->x - orbitSpeed.x < -0.1)
  {
    orbitSpeed.x += (targetOrbitSpeed->x - orbitSpeed.x) * 0.01;
  }

  if(targetOrbitSpeed->y - orbitSpeed.y > 0.1 || targetOrbitSpeed->y - orbitSpeed.y < -0.1)
  {
    orbitSpeed.y += (targetOrbitSpeed->y - orbitSpeed.y) * 0.01;
  }

  GUIUpdate();
  handleCamera();
  for(int i=0; i<kinects; i++)
  {
    clouds[i].update();
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

  for(int i=0; i<kinects; i++)
  {
    clouds[i].brightBoost = brightBoost;
    clouds[i].setPointSize(pointSize);
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofBackground(0);
  ofSetColor(255, 255, 255);

  if(syphonOn)  frame.begin();
  post.begin();
  cam.begin();
    drawClouds();
    ofNoFill();
    if (showCube) ofDrawBox(textCenter.x, textCenter.y, textCenter.z, textBoxWidth, textBoxHeight, textBoxDepth);
    drawSentences();
  cam.end();
  post.end();
  if(syphonOn)  frame.end();

  if(syphonOn)  mainOutputSyphonServer.publishTexture(&frame.getTexture());


  if(calibration)
  {
    drawCalibration();
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
  for (unsigned i=0; i < sentences.size(); i++)
  {
    ofPushMatrix();
    ofTranslate(sentences[i].position);
    textFont.drawString(sentences[i].sentence, 0, 0);
    ofPopMatrix();
  }
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

void ofApp::resetCamera()
{
  cam = ofCamera();
  cam.setFarClip(100000);
  fov = 30;
  cam.setFov(fov);
  cam.setGlobalPosition(initialCameraPosition.x, initialCameraPosition.y, initialCameraPosition.z);
  camOrbit = false;
  orbitLatitude = 0;
  orbitLongitude = 0;
  orbitRadius = 1800;
  orbitSpeed.x = 0.0;
  orbitSpeed.y = 0.0;
  targetOrbitSpeed = ofVec2f(0,0);
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

  cam.begin();
  ofPushMatrix();
  ofEnableDepthTest();
	ofDrawSphere(orbitCenterPoint->x, orbitCenterPoint->y, orbitCenterPoint->z, 10);
	ofDisableDepthTest();
	ofPopMatrix();
  cam.end();
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
      camOrbit = !camOrbit;
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
      fov++;
      cam.setFov(fov);
      break;
    case 'j':
      fov--;
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
    case OF_KEY_SHIFT:
      shift = true;
      break;
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

    case OF_KEY_SHIFT:
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
