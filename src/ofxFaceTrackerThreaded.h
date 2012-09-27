#pragma once

#include "ofxFaceTracker.h"

class ofxFaceTrackerThreaded : public ofThread {
public:
	void setup() {
		startThread(false, false);
	}
    
    void reset() {
		if(lock()) {
			tracker->reset();
			unlock();
		}
	}
	void update(cv::Mat mat) {
		if(lock()) {
			ofxCv::copy(mat, buffer);
			unlock();
			newFrame = true;
			ofSleepMillis(30); // give the tracker a moment
		}
	}
	void draw() {
		if(lock()) {
			tracker->draw();
			unlock();
		}
	}
    void setIterations(int iterations) {
		if(lock()) {
			tracker->setIterations(iterations);
			unlock();
		}
	}
    void setAttempts(int attempts) {
		if(lock()) {
			tracker->setAttempts(attempts);
			unlock();
		}
	}
	bool getFound() {
		bool found = false;
		if(lock()) {
			found = tracker->getFound();
			unlock();
		}
		return found;
	}
	ofMesh getImageMesh() {
		ofMesh imageMesh;
		if(lock()) {
			imageMesh = tracker->getImageMesh();
			unlock();
		}
		return imageMesh;
	}
    ofMesh getObjectMesh() {
		ofMesh objectMesh;
		if(lock()) {
			objectMesh = tracker->getObjectMesh();
			unlock();
		}
		return objectMesh;
	}
    ofPolyline getImageFeature(ofxFaceTracker::Feature feature) {
		ofPolyline imageFeature;
		if(lock()) {
			imageFeature = tracker->getImageFeature(feature);
			unlock();
		}
		return imageFeature;
	}
    
    vector<ofVec2f> getImagePoints() {
		vector<ofVec2f> imagePoints;
		if(lock()) {
			imagePoints = tracker->getImagePoints();
			unlock();
		}
		return imagePoints;
	}
protected:
	void threadedFunction() {
		newFrame = false;
		tracker = new ofxFaceTracker();
		tracker->setup();
		while(isThreadRunning()) {
			if(newFrame) {
				if(lock()) {
					newFrame = false;
					tracker->update(buffer);
					unlock();
				}
			}
			ofSleepMillis(1);
		}
	}
	
	ofxFaceTracker* tracker;
	cv::Mat buffer;
	bool newFrame;
};