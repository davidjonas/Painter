#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  textFont.load("Sequel_Demo.ttf", 70);

	//ofHideCursor();

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
  orbitRadius = 1800;
  orbitSpeed = 0.1;
  targetOrbitSpeed = 0.1;
  orbitCenterPoint.x = 0;
  orbitCenterPoint.y= 0;
  orbitCenterPoint.z = -1800;


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

  //post processing initialization
  //setupPost();
}

void ofApp::setupPost()
{
  post.init(ofGetWidth(), ofGetHeight());
  post.createPass<FxaaPass>()->setEnabled(false);
  post.createPass<BloomPass>()->setEnabled(false);
  post.createPass<DofPass>()->setEnabled(false);
  post.createPass<KaleidoscopePass>()->setEnabled(false);
  post.createPass<NoiseWarpPass>()->setEnabled(false);
  post.createPass<PixelatePass>()->setEnabled(false);
  post.createPass<EdgePass>()->setEnabled(false);
  post.createPass<VerticalTiltShifPass>()->setEnabled(false);
  post.createPass<GodRaysPass>()->setEnabled(false);
}

//SocketIO stuff
void ofApp::onConnection () {
  isConnected = true;
	ofLogNotice("ofxSocketIO", "Connected!");
  bindEvents();
	std::string msg = "subscribe";
  std::string type = "sentence";
  socketIO.emit(msg, type);

  std::string type2 = "speech";
  socketIO.emit(msg, type2);
}

void ofApp::bindEvents () {
  std::string serverEventName = "speech";
  socketIO.bindEvent(speechEvent, serverEventName);
  ofAddListener(speechEvent, this, &ofApp::onSpeechEvent);

  std::string sentenceEventName = "sentence";
  socketIO.bindEvent(sentenceEvent, sentenceEventName);
  ofAddListener(sentenceEvent, this, &ofApp::onSentenceEvent);
}

void ofApp::onSpeechEvent (ofxSocketIOData& data) {
  if(listening)
  {
    Sentence s;
    s.sentence = data.getStringValue("msg");
    //s.position = ofVec3f(ofRandom(-1000, 1000), ofRandom(-2000, 2000), ofRandom(-6000, -1000));
    s.position = ofVec3f(-800, -((int)sentences.size() * 150) + 400, -1000);
    s.delayTimer.setPeriod(ofRandom(3, 7));
    sentences.push_back(s);
  }
}

void ofApp::onSentenceEvent (ofxSocketIOData& data) {
  int id = data.getIntValue("id");
  string txt = data.getStringValue("changed");

	sentences[id].sentence = txt;
}

void ofApp::handleTimers()
{
  for (unsigned i=0; i < sentences.size(); i++)
  {
    if(sentences[i].delayTimer.tick())
    {
      string msg = "{\"msg\":\""+ sentences[i].sentence +"\", \"id\":" + to_string(i) + "}";
      string evtName = "process_request";
      socketIO.emit(evtName, msg);
    }
  }
}

//--------------------------------------------------------------
void ofApp::update(){
  if(synonyms)
  {
      handleTimers();
  }

  if(targetOrbitSpeed - orbitSpeed > 0.1 || targetOrbitSpeed - orbitSpeed < -0.1)
  {
    orbitSpeed += (targetOrbitSpeed - orbitSpeed) * 0.01;
  }

  handleCamera();
  for(int i=0; i<kinects; i++)
  {
    clouds[i].update();
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofBackground(0);
  ofSetColor(255, 255, 255);

  //post.begin(cam);
  cam.begin();
    drawClouds();
    drawSentences();
  cam.end();
  //post.end();

  if(calibration)
  {
    drawCalibration();
  }

  if(debug)
  {
    drawDebug();
  }
}

void ofApp::drawClouds()
{
  for(int i=0; i<kinects; i++)
  {
    clouds[i].draw();
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
    if(mouseControl)
    {
  		cam.rotate(mouseDeltaX/10.0, 0.0, 1.0, 0.0);
  		cam.tilt(mouseDeltaY/10.0);
    }
	}
	else
	{
		orbitLatitude += orbitSpeed;
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
  orbitRadius = 1800;
  orbitSpeed = 0.1;
  targetOrbitSpeed = 0.1;
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
        targetOrbitSpeed += 0.2;
        break;
      case '-':
        //orbitRadius += 100;
        targetOrbitSpeed -= 0.2;
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
  for(int i=0; i<kinects; i++)
  {
    clouds[i].close();
  }
}
