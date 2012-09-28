#include "testApp.h"
#include "ofMeshUtils.h"

extern "C" {
#include "macGlutfix.h"
}

using namespace ofxCv;
using namespace cv;



void testApp::setup() {
    ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
    
    ofSetFrameRate(60);
    ofEnableSmoothing();
	ofSetVerticalSync(true);

	finalClone.setup(640, 720);
	ofFbo::Settings settings;
	settings.width = 640;
	settings.height = 720;
	maskFbo.allocate(settings);
	raceFbo.allocate(settings);
    userFbo.allocate(settings);

	userTracker.setup();
	raceTracker.setup();
	
	raceTracker.setAttempts(4);
    userTracker.setIterations(25);
	userTracker.setAttempts(4);
    raceTracker.setIterations(25);
    userTracker.setClamp(4);
    raceTracker.setClamp(4);
    
    currentFace = 0;
    currentRace = 0;
    
    userImage.setImageType(OF_IMAGE_COLOR);
    raceImage.setImageType(OF_IMAGE_COLOR);
    
	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");

	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    races.allowExt("jpg");
	races.allowExt("png");
	races.listDir("races");

	if(races.size()!=0){
		loadRace(races.getPath(currentRace));
	}
    
    if(raceImage.getWidth() > 0){
        raceTracker.update(toCv(raceImage));
    }
    
    if(userImage.getWidth() > 0){
        userTracker.update(toCv(userImage));
    }
    
    maskFbo.begin();
    ofClear(0, 255);
    maskFbo.end();
	raceFbo.begin();
    ofClear(0, 255);
    raceFbo.end();
    userFbo.begin();
    ofClear(0, 255);
    userFbo.end();

    frameCounter =0;
    newScaleX =1;
    newScaleY =1;
   
}



void testApp::update() {

    if(frameCounter<25){
        userTracker.update(toCv(userImage));
        raceTracker.update(toCv(raceImage));
        frameCounter++;
    }

	if(userTracker.getFound() && raceTracker.getFound()) {
        
        raceImageMesh.clear();
        raceImageMesh = raceTracker.getImageMesh();
        
        ofMesh userObjectMesh = userTracker.getObjectMesh();
        ofMesh raceObjectMesh = raceTracker.getObjectMesh();
        
        float theDistance1X = raceTracker.getObjectMesh().getVertex(16).distance(raceTracker.getObjectMesh().getVertex(0));
        float theDistance2X = userTracker.getObjectMesh().getVertex(16).distance(userTracker.getObjectMesh().getVertex(0));
        theAverageX = theDistance1X/theDistance2X;
        
        float theDistance1Y = raceTracker.getObjectMesh().getVertex(32).distance(raceTracker.getObjectMesh().getVertex(8));
        float theDistance2Y = userTracker.getObjectMesh().getVertex(32).distance(userTracker.getObjectMesh().getVertex(8));
        theAverageY = theDistance1Y/theDistance2Y;

        for(int i = 0 ; i < raceObjectMesh.getNumVertices(); i++){
            if(i >= 0 && i <=16){
                raceObjectMesh.setVertex(i, userObjectMesh.getVertex(i));
            }
            if(i >= 31 && i <=35){
                raceObjectMesh.setVertex(i, userObjectMesh.getVertex(i));
            }
            if(i >= 36 && i <=47){
               raceObjectMesh.setVertex(i, userObjectMesh.getVertex(i));
            }
        }
        
        templeDifference = raceObjectMesh.getVertex(0).distance(raceTracker.getMeanObjectMesh().getVertex(0));
        
        raceFbo.begin();
            ofClear(0, 255);
            ofEnableSmoothing();
        
            ofPushMatrix();
                ofTranslate(raceImage.getWidth()/2, raceImageMesh.getVertex(17).y);
                newScaleX = 1-theAverageX;
                ofScale( 1+newScaleX,1+(-newScaleX));
                ofTranslate(-raceImage.getWidth()/2, -raceImageMesh.getVertex(21).y);
                raceImage.drawSubsection(0, 0, raceImage.getWidth(), raceImageMesh.getVertex(0).y,0,0); // Draw Forhead Section
            ofPopMatrix();
        
            ofPushView();
                ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
                ofTranslate(raceImage.getWidth()/2,raceImage.getHeight()/2);
                ofVec2f pos = raceTracker.getPosition();
                ofTranslate(pos.x-raceImage.getWidth()/2,pos.y-raceImage.getHeight()/2);
                applyMatrix(raceTracker.getRotationMatrix());
                ofScale(raceTracker.getScale(),raceTracker.getScale(),raceTracker.getScale());
                raceImage.bind();
                raceObjectMesh.draw();
                raceImage.unbind();
            ofPopView();
        raceFbo.end();
        
            
        raceObjectMesh.clearTexCoords();
        raceObjectMesh.addTexCoords(userTracker.getImagePoints());
        

        maskFbo.begin();
            ofClear(0, 255);
            ofEnableSmoothing();
        
            ofPushView();
                ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
                ofTranslate(raceImage.getWidth()/2,raceImage.getHeight()/2);
                ofTranslate(pos.x-raceImage.getWidth()/2,pos.y-raceImage.getHeight()/2);
                applyMatrix(raceTracker.getRotationMatrix());
                ofScale(raceTracker.getScale(),raceTracker.getScale(),raceTracker.getScale());
                raceObjectMesh.draw();
            ofPopView();
        maskFbo.end();
            
        userFbo.begin();
            ofClear(0, 255);
            ofEnableSmoothing();
            raceImage.draw(0,0);
        
            ofPushView();
                ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
                ofTranslate(raceImage.getWidth()/2,raceImage.getHeight()/2);
                ofTranslate(pos.x-raceImage.getWidth()/2,pos.y-raceImage.getHeight()/2);
                applyMatrix(raceTracker.getRotationMatrix());
                ofScale(raceTracker.getScale(),raceTracker.getScale(),raceTracker.getScale());
                userImage.bind();
                raceObjectMesh.draw();
                userImage.unbind();
            ofPopView();
       userFbo.end();
        
	}  
    
    faceStrength = ofMap(mouseX,0,ofGetWidth()/2,0,15,true);
    finalClone.setStrength(faceStrength);
    finalClone.update(userFbo.getTextureReference(),raceFbo.getTextureReference(),  maskFbo.getTextureReference());

}

void testApp::draw() {
    ofEnableSmoothing();
    ofBackground(0);
	ofSetColor(255);
    
    ofPushMatrix();
        ofEnableSmoothing();
        float srcScale;
        if(userImage.getWidth()>640){
            ofScale(.7,.7,.7);
            srcScale=.7;
        }
        else{
            srcScale=1;
        }
        userImage.draw(0,0);
    ofPopMatrix();
    
    ofPushMatrix();
        ofTranslate(0,userImage.getHeight()*srcScale);
        ofScale(1,1);
        raceImage.draw(0,0);
    ofPopMatrix();
    
    ofPushMatrix();
        ofTranslate(640,0);
        ofScale(1.2,1.2,1.2);
        finalClone.draw(0, 0);
    ofPopMatrix();
    
   
   drawHighlightString(ofToString( raceTracker.getScale()), 300, 10);
    drawHighlightString(ofToString( newScaleX), 300, 30);
    drawHighlightString(ofToString( newScaleY), 300, 50);
    drawHighlightString(ofToString( templeDifference), 300, 70);
    drawHighlightString(ofToString( theAverageX), 300, 130);
    
	
}

void testApp::loadFace(string face){
	userImage.loadImage(face);
    frameCounter=0;
}

void testApp::loadRace(string race){
	raceImage.loadImage(race);
    frameCounter=0;
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0]);
}

void testApp::keyPressed(int key){
	switch(key){
        case OF_KEY_UP:
            currentFace++;
            break;
        case OF_KEY_DOWN:
            currentFace--;
            break;
        case OF_KEY_RIGHT:
            currentRace++;
            break;
        case OF_KEY_LEFT:
            currentRace--;
            break;
        case 'r':
            raceTracker.reset();
            userTracker.reset();
            break;
        case 'f':
            break;
        case 'd':
            break;
        case 'q':
            break;
        case 'w':
            break;
        case 'z':
            ofToggleFullscreen();
            break;
        case 'e':
            break;
	}
    
    if(currentFace ==faces.size()){
        currentFace=1;
    }
    if(currentFace <0){
        currentFace=faces.size()-1;
    }
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    if(currentRace ==races.size()){
        currentRace=0;
    }
    if(currentRace <0){
        currentRace=races.size()-1;
    }
	if(races.size()!=0){
		loadRace(races.getPath(currentRace));
	}
}

