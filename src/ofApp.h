#pragma once

#include "ofMain.h"
#include "PointCloud.h"
#include "ofxKinect.h"
#include "ofEvents.h"
#include "ofxSocketIO.h"
#include "ofxSocketIOData.h"
#include "ofxPostProcessing.h"
#include "ofxTiming.h"
#include "ofxGui.h"
#include "ofxSyphon.h"
#include "ofxMidi.h"

class ofApp : public ofBaseApp, public ofxMidiListener{

	public:
		void setup();
		void setupPost();
		void setupGUI();
		void GUIUpdate();
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
		//float orbitRadius;
		ofVec2f orbitSpeed;
		//ofVec2f targetOrbitSpeed;
		//ofVec3f orbitCenterPoint;
		ofVec3f initialCameraPosition;
		ofxPostProcessing post;

		//post effects
		DofPass::Ptr dof;
		FxaaPass::Ptr antiAlias;
	  BloomPass::Ptr bloom;
	  KaleidoscopePass::Ptr kaleidoscope;
	  NoiseWarpPass::Ptr warp;
	  ConvolutionPass::Ptr convolution;
	  BleachBypassPass::Ptr bleach;
	  PixelatePass::Ptr pixelate;
	  EdgePass::Ptr edge;
	  VerticalTiltShifPass::Ptr tiltshift;
	  GodRaysPass::Ptr godrays;
		ContrastPass::Ptr contrast;

		//Main GUI
		ofxPanel gui;
		ofxFloatSlider focus;
		ofxFloatSlider aperture;
		ofxFloatSlider contrastValue;
		ofxFloatSlider brightnessValue;
		ofxFloatSlider multipleValue;
		ofxVec3Slider orbitCenterPoint;
		ofxFloatSlider orbitRadius;
		ofxVec2Slider targetOrbitSpeed;
		ofxColorSlider color;
		ofxToggle rgbImage;
		bool guiActive;

		//Effects GUI
		ofxPanel postGui;
		ofxToggle dofOn;
		ofxToggle antiAliasOn;
		ofxToggle bloomOn;
	  ofxToggle kaleidoscopeOn;
	  ofxToggle warpOn;
	  ofxToggle convolutionOn;
	  ofxToggle bleachOn;
	  ofxToggle pixelateOn;
	  ofxToggle edgeOn;
	  ofxToggle tiltshiftOn;
	  ofxToggle godraysOn;
		ofxToggle contrastOn;
		bool postGuiActive;

    //SocketIO stuff
  	void onSpeechEvent(ofxSocketIOData& data);
		void onSentenceEvent(ofxSocketIOData& data);
		void handleTimers();


  	ofxSocketIO socketIO;
  	bool isConnected;
    void onConnection();
		void onNotice(string& name);
    void bindEvents();
		void subscribeEvents();
    ofEvent<ofxSocketIOData&> speechEvent;
    ofEvent<ofxSocketIOData&> sentenceEvent;
  	//SocketIO stuff ends here

		//Sentences
		struct Sentence
		{
			string sentence;
			ofVec3f position;
			//DelayTimer delayTimer;
		};

		//vector<Sentence> sentences;
		map<int, Sentence> sentences;
		ofTrueTypeFont textFont;
		DelayTimer delayTimer;

		//Syphon stuff
		ofxSyphonServer mainOutputSyphonServer;
		ofFbo frame;
		bool syphonOn;


		//Midi stuff
		ofxMidiIn midiIn;
		void newMidiMessage(ofxMidiMessage& eventArgs);
};
