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
    
	ofxFaceTracker userTracker;
	ofxFaceTracker raceTracker;

    ofImage userImage;
    ofImage raceImage;

	Clone finalClone;
	ofFbo raceFbo, userFbo, maskFbo;

	ofDirectory faces;
	int currentFace;
    
    ofDirectory races;
	int currentRace;
    
    ofMesh raceImageMesh;
    
    int faceStrength;
    
    int captureWidth;
    int captureHeight;
    
    ofVec2f position;
	float scale;
	ofVec3f orientation;
	ofMatrix4x4 rotationMatrix;
    
    int frameCounter;
    
    float templeDifference;
    
    float theAverageX;
    float theAverageY;

};
