#pragma once

#include "ofMain.h"
#include "PointCloud.h"
#include "ofxKinect.h"
#include "ofEvents.h"
#include "ofxSocketIO.h"
#include "ofxSocketIOData.h"
#include "ofxTiming.h"
#include "ofxDOF.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void setupPost();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void handleCamera();
		void resetCamera();
		void clearSentences();

		void drawSentences();
		void drawClouds();
		void drawDebug();
		void drawCalibration();

		PointCloud * clouds;
		int kinects;

		bool debug;
		bool calibration;
		int selectedCloud;

		ofCamera cam;
		ofxDOF depthOfField;
		float fov;
		bool up;
		bool down;
		bool left;
		bool right;
		bool fly;
		bool land;
		int pMouseX;
		int pMouseY;
		int mouseDeltaX;
		int mouseDeltaY;
		bool camOrbit;
		bool mouseControl;
		bool synonyms;
		bool listening;
		float orbitLatitude;
		float orbitLongitude;
		float orbitRadius;
		float orbitSpeed;
		float targetOrbitSpeed;
		ofVec3f orbitCenterPoint;
		ofVec3f initialCameraPosition;

    //SocketIO stuff
  	void onSpeechEvent(ofxSocketIOData& data);
		void onSentenceEvent(ofxSocketIOData& data);
		void handleTimers();


  	ofxSocketIO socketIO;
  	bool isConnected;
    void onConnection();
    void bindEvents();
    ofEvent<ofxSocketIOData&> speechEvent;
    ofEvent<ofxSocketIOData&> sentenceEvent;
  	//SocketIO stuff ends here

		struct Sentence
		{
			string sentence;
			ofVec3f position;
			DelayTimer delayTimer;
		};

		vector<Sentence> sentences;
		ofTrueTypeFont textFont;

};
