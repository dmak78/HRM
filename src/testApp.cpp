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
    
    drawFaceFrame = false;
    drawWireFrame = false;
    
	ofSetVerticalSync(true);
	cloneReady = false;
    srcCloneReady = false;
	//cam.initGrabber(640, 480);
	camClone.setup(640, 720);
    srcClone.setup(640, 720);
	ofFbo::Settings settings;
	settings.width = 640;
	settings.height = 720;
	camMask.allocate(settings);
    srcMask.allocate(settings);
	srcFbo.allocate(settings);
    camFbo.allocate(settings);
    finalFbo.allocate(settings);
    orgMask1.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	
	srcTracker.setAttempts(4);
    camTracker.setIterations(25);
	camTracker.setAttempts(4);
    srcTracker.setIterations(25);
    camTracker.setClamp(4);
    srcTracker.setClamp(4);
   // camTracker.setTolerance(.2);
   // srcTracker.setTolerance(.2);
    
    currentFace = 0;
    currentRace = 0;
    
    srcNeedsSetting=true;
    camNeedsSetting=true;
    srcReady=true;
    camReady=true;
    
    mask1.setImageType(OF_IMAGE_COLOR);
    mask2.setImageType(OF_IMAGE_COLOR);
    
	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    races.allowExt("jpg");
	races.allowExt("png");
	races.listDir("races");
	currentRace = 0;
	if(races.size()!=0){
		loadRace(races.getPath(currentRace));
	}
    
    //  mask1.loadImage("YIM1480A copy.jpg");
    // mask2.loadImage("MEMDF.jpg");
    
    //mask1.resize(mask1.getWidth()/3, mask1.getHeight()/3);
    
    
    if(mask2.getWidth() > 0){
        srcTracker.update(toCv(mask2));
    }
    
    if(mask1.getWidth() > 0){
        camTracker.update(toCv(mask1));
    }
    
  
    
    camMask.begin();
    ofClear(0, 255);
    camMask.end();
    srcMask.begin();
    ofClear(0, 255);
    srcMask.end();
	srcFbo.begin();
    ofClear(0, 255);
    srcFbo.end();
    camFbo.begin();
    ofClear(0, 255);
    camFbo.end();
    finalFbo.begin();
    ofClear(0, 255);
    finalFbo.end();
    orgMask1.begin();
    ofClear(0, 255);
    orgMask1.end();
    
    
    frameCounter =0;
    newScaleX =1;
    newScaleY =1;
    editForehead = false;
}



void testApp::update() {
    
    
    float addScale = ofMap(mouseY,0,ofGetHeight(),-1,1,true);
    
    if(frameCounter<25){
        camTracker.update(toCv(mask1));
        srcTracker.update(toCv(mask2));
        frameCounter++;
    }

    if(!editForehead){
        faceStrength = ofMap(mouseX,0,ofGetWidth()/2,0,15,true);
      
    }
    else{
        newScaleX = ofMap(mouseX,0,ofGetWidth()/2,1,1.5);
         newScaleY = ofMap(mouseY,0,ofGetHeight()/2,1,1.5);
    }
    
    
    // cam.update();
    
	if(camTracker.getFound() && srcTracker.getFound()) {
        
        camMesh.clear();
        camMesh = camTracker.getImageMesh();
       // buildNormals(camMesh);
        
        srcMesh.clear();
        srcMesh = srcTracker.getImageMesh();
      //  buildNormals(srcMesh);
        
        ofMesh camObjectMesh = camTracker.getObjectMesh();
        ofMesh srcObjectMesh = srcTracker.getObjectMesh();
        
        float theDistance1X = srcTracker.getObjectMesh().getVertex(16).distance(srcTracker.getObjectMesh().getVertex(0));
        float theDistance2X = camTracker.getObjectMesh().getVertex(16).distance(camTracker.getObjectMesh().getVertex(0));
        theAverageX = theDistance1X/theDistance2X;
        
        float theDistance1Y = srcTracker.getObjectMesh().getVertex(32).distance(srcTracker.getObjectMesh().getVertex(8));
        float theDistance2Y = camTracker.getObjectMesh().getVertex(32).distance(camTracker.getObjectMesh().getVertex(8));
        theAverageY = theDistance1Y/theDistance2Y;

        
        
        for(int i = 0 ; i < srcObjectMesh.getNumVertices(); i++){
            if(i >= 0 && i <=16){
                srcObjectMesh.setVertex(i, camObjectMesh.getVertex(i));
                
            }
            if(i >= 31 && i <=35){
                srcObjectMesh.setVertex(i, camObjectMesh.getVertex(i));
            }
            if(i >= 36 && i <=47){
               srcObjectMesh.setVertex(i, camObjectMesh.getVertex(i));
            }

            
        }
        
        templeDifference = srcObjectMesh.getVertex(0).distance(srcTracker.getMeanObjectMesh().getVertex(0));

        
        buildNormals(srcObjectMesh);
        


        
    srcFbo.begin();
        ofClear(0, 255);
         ofEnableSmoothing();
        ofPushMatrix();

        ofTranslate(mask2.getWidth()/2, srcMesh.getVertex(17).y);
        
        newScaleX = 1-theAverageX;
       // newScaleY = 1-theAverageY;

       //ofScale( 1,1);
        ofScale( 1+newScaleX,1+(-newScaleX));
        ofTranslate(-mask2.getWidth()/2, -srcMesh.getVertex(21).y);
        mask2.drawSubsection(0, 0, mask2.getWidth(), srcMesh.getVertex(0).y,0,0);//srcMesh.getVertex(6).y, 0, 0);
        ofPopMatrix();
        
        ofPushView();
        ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
        ofTranslate(mask2.getWidth()/2,mask2.getHeight()/2);
        ofVec2f pos = srcTracker.getPosition();
        ofTranslate(pos.x-mask2.getWidth()/2,pos.y-mask2.getHeight()/2);
        applyMatrix(srcTracker.getRotationMatrix());
        ofScale(srcTracker.getScale(),srcTracker.getScale(),srcTracker.getScale());
        mask2.bind();
        srcObjectMesh.draw();
        mask2.unbind();
        ofPopView();
    srcFbo.end();

        
        camObjectMesh.clearTexCoords();
        camObjectMesh.addTexCoords(srcTracker.getImagePoints());
        
        srcObjectMesh.clearTexCoords();
        srcObjectMesh.addTexCoords(camTracker.getImagePoints());

            
    camMask.begin();
        ofClear(0, 255);
         ofEnableSmoothing();
       // ofPushMatrix();
        ofPushView();
        ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
        ofTranslate(mask2.getWidth()/2,mask2.getHeight()/2);
        ofTranslate(pos.x-mask2.getWidth()/2,pos.y-mask2.getHeight()/2);
        applyMatrix(srcTracker.getRotationMatrix());
        ofScale(srcTracker.getScale(),srcTracker.getScale(),srcTracker.getScale());
        
        srcObjectMesh.draw();
       
        ofPopView();
       // ofPopMatrix();
    camMask.end();
        
        
        
        camFbo.begin();
        ofClear(0, 255);
         ofEnableSmoothing();
        mask2.draw(0,0);
        ofPushMatrix();
       // mask2.drawSubsection(0, 0, mask2.getWidth(), srcMesh.getVertex(0).y, 0, 0);
        ofPopMatrix();
        ofPushView();
            ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, false, -1000, 1000);
            ofTranslate(mask2.getWidth()/2,mask2.getHeight()/2);
            ofTranslate(pos.x-mask2.getWidth()/2,pos.y-mask2.getHeight()/2);
            applyMatrix(srcTracker.getRotationMatrix());
            ofScale(srcTracker.getScale(),srcTracker.getScale(),srcTracker.getScale());
            mask1.bind();
            srcObjectMesh.draw();
            mask1.unbind();
        //srcObjectMesh.drawWireframe();
        ofPopView();
        camFbo.end();
        
        srcMask.begin();
        ofClear(255, 255);
        ofPushStyle();
          //  srcMesh.draw();
        ofPopStyle();
        srcMask.end();
        
        finalFbo.begin();
        ofClear(0,255);
        mask1.bind();
        srcMesh.draw();
        mask1.unbind();
        finalFbo.end();
	}  
    
    ofPixels mask1pix;
    camFbo.getTextureReference().readToPixels(mask1pix);
    
    ofPixels mask2pix;
    srcFbo.getTextureReference().readToPixels(mask2pix);
    
    float aveAmount1 = ofMap(mouseY,0,ofGetHeight(),0,1,true);
     float aveAmount2 = ofMap(mouseY,0,ofGetHeight(),1,0,true);
    
    for(int x = 0 ; x < mask1pix.getWidth();x++){
        for(int y = 0 ;y < mask1pix.getHeight();y++){
            ofColor cur1 = mask1pix.getColor(x, y);
            ofColor cur2 = mask2pix.getColor(x, y);
            ofColor average = ofColor((cur1.r*aveAmount1 + cur2.r*aveAmount2)/(aveAmount1+aveAmount2),(cur1.g*aveAmount1+ cur2.g*aveAmount2)/(aveAmount1+aveAmount2),(cur1.b*aveAmount1 + cur2.b*aveAmount2)/(aveAmount1+aveAmount2),(cur1.a*aveAmount1 + cur2.a*aveAmount2)/(aveAmount1+aveAmount2));
            mask1pix.setColor(x, y, average);
            mask2pix.setColor(x, y, average);
        }
    }
    
    aveMask.setFromPixels(mask1pix);
    aveMask2.setFromPixels(mask2pix);
    
//    camClone.setStrength(faceStrength);
//    camClone.update(srcFbo.getTextureReference(),camFbo.getTextureReference(),  camMask.getTextureReference());
    camClone.setStrength(faceStrength);
    camClone.update(camFbo.getTextureReference(),srcFbo.getTextureReference(),  camMask.getTextureReference());
    
    srcClone.setStrength(faceStrength);
    srcClone.update(finalFbo.getTextureReference(), mask2.getTextureReference(), srcMask.getTextureReference());
    
//    orgMask1.begin();
//    ofClear(0,255);
//    srcClone.draw(0,0);
//    orgMask1.end();


}

void testApp::draw() {
    ofEnableSmoothing();
    ofBackground(0);
	ofSetColor(255);
    
    ofPushMatrix();
    ofEnableSmoothing();
    float srcScale;
    if(mask1.getWidth()>640){
        ofScale(.7,.7,.7);
        srcScale=.7;
    }
    else{
        srcScale=1;
    }
    mask1.draw(0,0);
    //camTracker.getImageMesh().drawWireframe();
    ofPopMatrix();
    ofPushMatrix();
    ofTranslate(0,mask1.getHeight()*srcScale);
    ofScale(1,1);
    mask2.draw(0,0);

    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(640,0);
    ofScale(1.2,1.2,1.2);
    camClone.draw(0, 0);
    ofPopMatrix();
//        srcClone.draw(640, 0);

    

    
    
//    mask1.draw(0,0);
//    ofPushView();
//    ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, true, -1000, 1000);
//    ofTranslate(mask1.getWidth()/2,mask1.getHeight()/2);
//    ofVec2f pos = camTracker.getPosition();
//    ofTranslate(pos.x-mask1.getWidth()/2,pos.y-mask1.getHeight()/2);
//    applyMatrix(camTracker.getRotationMatrix());
//    ofScale(camTracker.getScale(),camTracker.getScale(),camTracker.getScale());
//    mask1.bind();
//    camObjectMesh.draw();
//    mask1.unbind();
//    ofPopView();
    
  // aveMask.draw(0,0);
   //aveMask2.draw(0,0);
    

    


    ofPushMatrix();
    if(drawWireFrame){
     //   srcMesh.drawWireframe();
        srcTracker.draw();
      //  ofTranslate(0, 0);
       // camMesh.drawWireframe();
    }
    ofPopMatrix();
    
    ofPushMatrix();
    if(drawFaceFrame){
        //camMesh.drawWireframe();
        ofPushView();
        ofSetupScreenOrtho(1280, 480, OF_ORIENTATION_DEFAULT, true, -1000, 1000);
        ofTranslate(mask2.getWidth()/2,mask2.getHeight()/2);
        ofVec3f pos = srcTracker.getPosition();
        ofTranslate(pos.x-mask2.getWidth()/2,pos.y-mask2.getHeight()/2);
        applyMatrix(srcTracker.getRotationMatrix());
        ofScale(srcTracker.getScale(),srcTracker.getScale(),srcTracker.getScale());
       // mask1.bind();
        camTracker.getObjectMesh().drawWireframe();
       // mask1.unbind();
        ofPopView();
     //   srcTracker.draw();
     //   ofTranslate(0, 0);
     //   camTracker.draw();
    }
    ofPopMatrix();
    

    
  //  mask1.draw(0,480,mask1.getWidth()/3,mask1.getHeight()/3);
  //  mask2.draw(800/3,480,mask2.getWidth()/3,mask2.getHeight()/3);
   drawHighlightString(ofToString( srcTracker.getScale()), 300, 10);
    drawHighlightString(ofToString( newScaleX), 300, 30);
    drawHighlightString(ofToString( newScaleY), 300, 50);
    drawHighlightString(ofToString( templeDifference), 300, 70);
    drawHighlightString(ofToString( theAverageX), 300, 130);
    
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 300, 10);
	}
	if(src.getWidth() == 0) {
        //	drawHighlightString("drag an image here", 300, 30);
	} else if(!srcTracker.getFound()) {
		drawHighlightString("image face not found", 300, 30);
	}
    
    //ofDrawBitmapString(ofToString((int) camMesh.getNumTexCoords()) , 350, 20);
//    drawHighlightString("slide mouse across screen the engage blending", 275, 30);
//    drawHighlightString("press up and down arrows to change left face", 275, 50);
//    drawHighlightString("press left and right arrows to change right face", 275, 70);
//    drawHighlightString("press 'd' key to toggle facetracker wireframe ", 275, 90);
//    drawHighlightString("press 'f' key to toggle facetracker outline ", 275, 110);
}

void testApp::loadFace(string face){
	mask1.loadImage(face);
    frameCounter=0;
    if(mask1.getWidth() > 0){
      //  camTracker.update(toCv(mask1));
    }

}

void testApp::loadRace(string race){
	mask2.loadImage(race);
    frameCounter=0;
    if(mask2.getWidth() > 0){
       // mask2.resize(mask2.getWidth()/3,mask2.getHeight()/3);
     //   srcTracker.update(toCv(mask2));
    }

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
//            srcTracker.reset();
//            camTracker.reset();
            break;
        case 'f':
            drawFaceFrame = !drawFaceFrame;
            break;
        case 'd':
            drawWireFrame = !drawWireFrame;
            break;
        case 'q':
//            faces.listDir("faces");
//            currentFace = 0;
//            if(faces.size()!=0){
//                loadFace(faces.getPath(currentFace));
//            }
            break;
        case 'w':
//            faces.listDir("races");
//            currentFace = 0;
//            if(faces.size()!=0){
//                loadFace(faces.getPath(currentFace));
//            }
            break;
        case 'z':
            ofToggleFullscreen();
            break;
        case 'e':
            editForehead=!editForehead;
            break;
	}
    
	//currentFace = ofClamp(currentFace,0,faces.size());
    
    if(currentFace ==faces.size()){
        currentFace=1;
    }
    if(currentFace <0){
        currentFace=faces.size()-1;
    }
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    //currentRace = ofClamp(currentRace,0,races.size());
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

