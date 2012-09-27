#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"

class testApp : public ofBaseApp {
public:
   
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
    void loadRace(string race);
	void keyPressed(int key);

    
    float newScaleX;
    float newScaleY;
    
    bool editForehead;

	ofxFaceTracker camTracker;
	ofVideoGrabber cam;
	ofxFaceTracker srcTracker;
	ofImage src;
    ofImage camMemoryTexture;
    ofImage srcMemoryTexture;
    ofImage mask1;
    ofImage mask2;
    ofImage aveMask;
    ofImage aveMask2;
	vector<ofVec2f> srcPoints;
    vector<ofVec2f> camPoints;

	bool cloneReady;
    bool srcCloneReady;
	Clone camClone;
    Clone srcClone;
    Clone cloneClone;
	ofFbo srcFbo, srcMask, camFbo, camMask, orgMask1, orgMask2, finalFbo;

	ofDirectory faces;
	int currentFace;
    
    ofDirectory races;
	int currentRace;
    
    ofEasyCam easyCam;
    
    ofMesh camMesh;
    ofMesh srcMesh;
    
    float depthScale;
    int faceStrength;
    
    ofImage		image;
    
    int captureWidth;
    int captureHeight;
    
    ofVec2f position;
	float scale;
	ofVec3f orientation;
	ofMatrix4x4 rotationMatrix;

    bool srcNeedsSetting;
    bool camNeedsSetting;
    
    bool srcReady;
    bool camReady;
    
    bool drawFaceFrame; 
    bool drawWireFrame;
    
    int frameCounter;
    
    float templeDifference;
    
    float theAverageX;
    float theAverageY;
    

};
