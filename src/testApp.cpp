#include "testApp.h"
#include "ofMeshUtils.h"

using namespace ofxCv;
using namespace cv;

void testApp::setup() {
    ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
    
    //main window initialization
    ofSetFrameRate(60);
    ofEnableSmoothing();
	ofSetVerticalSync(true);

    //Clone element and frame buffer object setup and initialization
	finalClone.setup(640, 720);
	ofFbo::Settings settings;
	settings.width = 640;
	settings.height = 720;
	maskFbo.allocate(settings);
	raceFbo.allocate(settings);
    userFbo.allocate(settings);
    
    //Facetrackers for User and Race initialization and settings
	userTracker.setup();
	raceTracker.setup();
	raceTracker.setAttempts(4);
    userTracker.setIterations(25);
	userTracker.setAttempts(4);
    raceTracker.setIterations(25);
    userTracker.setClamp(4);
    raceTracker.setClamp(4);
    
    //Image object initialization
    userImage.setImageType(OF_IMAGE_COLOR);
    raceImage.setImageType(OF_IMAGE_COLOR);
    
    //set current image to the first one
    currentFace = 0;
    currentRace = 0;
    
    //User image directory intialization 
	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");

    //load the current user face
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    //Race image directory intialization 
    races.allowExt("jpg");
	races.allowExt("png");
	races.listDir("races");

    //load the current race face
	if(races.size()!=0){
		loadRace(races.getPath(currentRace));
	}
    
    //Run facetracker on both of the images
    if(raceImage.getWidth() > 0){
        raceTracker.update(toCv(raceImage));
    }
    if(userImage.getWidth() > 0){
        userTracker.update(toCv(userImage));
    }
    
    //clear frame buffer objects
    maskFbo.begin();
    ofClear(0, 255);
    maskFbo.end();
	raceFbo.begin();
    ofClear(0, 255);
    raceFbo.end();
    userFbo.begin();
    ofClear(0, 255);
    userFbo.end();

    //set frame counter to 0
    frameCounter =0;
    
    //initialize scaling used for forehead portion resizing
    newScaleX =1;
    newScaleY =1;
   
}



void testApp::update() {

    //if the frame counter is at 0 or below, facetracker will run for 25 frames. This is to help get a more accurate reading on the face points.
    if(frameCounter<25){
        userTracker.update(toCv(userImage));
        raceTracker.update(toCv(raceImage));
        frameCounter++;
    }

	if(userTracker.getFound() && raceTracker.getFound()) {
        
        //clear out update the image mesh with image points on the Race image
        raceImageMesh.clear();
        raceImageMesh = raceTracker.getImageMesh();
        
        //create and initalize the user and race 'object' meshes with the currently detected faces
        ofMesh userObjectMesh = userTracker.getObjectMesh();
        ofMesh raceObjectMesh = raceTracker.getObjectMesh();
        
        //use the distance between temples (points 0 and 16) on both user and race faces to get an x-coordinate average
        float theDistanceRaceX = raceTracker.getObjectMesh().getVertex(16).distance(raceTracker.getObjectMesh().getVertex(0));
        float theDistanceUserX = userTracker.getObjectMesh().getVertex(16).distance(userTracker.getObjectMesh().getVertex(0));
        theAverageX = theDistanceRaceX/theDistanceUserX;
        
        //use the distance between chin and the nose (points 32 and 8) on both user and race faces to get an y-coordinate average
        float theDistanceRaceY = raceTracker.getObjectMesh().getVertex(32).distance(raceTracker.getObjectMesh().getVertex(8));
        float theDistanceUserY = userTracker.getObjectMesh().getVertex(32).distance(userTracker.getObjectMesh().getVertex(8));
        theAverageY = theDistanceRaceY/theDistanceUserY;

        //augment the race object mesh to be more in the shape of the user by replacing the coordinates for certain facial features
        for(int i = 0 ; i < raceObjectMesh.getNumVertices(); i++){
            if(i >= 0 && i <=16){
                raceObjectMesh.setVertex(i, userObjectMesh.getVertex(i)); //chin points 0 through 16
            }
            if(i >= 31 && i <=35){
                raceObjectMesh.setVertex(i, userObjectMesh.getVertex(i)); //left eye
            }
            if(i >= 36 && i <=47){
               raceObjectMesh.setVertex(i, userObjectMesh.getVertex(i)); //right eye
            }
        }
        
        //begin the frame buffer object for the face image part of the blend
        raceFbo.begin();
            ofClear(0, 255); //clear the FBO
            ofEnableSmoothing();
        
        
            //translate and rescale the forehead subsection of the race image using the averaging scale calculations from above
            ofPushMatrix();
                ofTranslate(raceImage.getWidth()/2, raceImageMesh.getVertex(17).y);
                newScaleX = 1-theAverageX;
                ofScale( 1+newScaleX,1+(-newScaleX));
                ofTranslate(-raceImage.getWidth()/2, -raceImageMesh.getVertex(21).y);
                raceImage.drawSubsection(0, 0, raceImage.getWidth(), raceImageMesh.getVertex(0).y,0,0); // Draw Forhead Section
            ofPopMatrix();
        
            //setup the screen,translate, and scale the race image
            //bind the race image to the race object mesh which was reshpaed above
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
        
        
        raceObjectMesh.clearTexCoords(); //clear out the image coordinates on the race object mesh
        raceObjectMesh.addTexCoords(userTracker.getImagePoints()); //replace the image coordinates of the race object mesh with the user image coordinates
        
        //create an FBO in the shape of the race object mesh which will act as a mask for the cloning/blending process
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
            
        //begin the FBO for the user image
        userFbo.begin();
            ofClear(0, 255);
            ofEnableSmoothing();
        
        
            //place the race object mesh again, but this time bind the user image to the mesh so that the blending of the two images lines up
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
    
    //set the strength of the blending and doing the blending using the FBOs from above
    blendingStrength = ofMap(mouseX,0,ofGetWidth()/2,0,15,true);
    finalClone.setStrength(blendingStrength);
    finalClone.update(userFbo.getTextureReference(),raceFbo.getTextureReference(),  maskFbo.getTextureReference());

}

void testApp::draw() {
    //setup the screen
    ofEnableSmoothing();
    ofBackground(0);
	ofSetColor(255);
    
    ofPushMatrix();
        ofEnableSmoothing();
    
        float srcScale;
    
        //set scalings for the images
        if(userImage.getWidth()>640){
            ofScale(.7,.7,.7);
            srcScale=.7;
        }
        else{
            srcScale=1;
        }
        userImage.draw(0,0); // draw the preview of the user image
    ofPopMatrix();
    
    //scale and translate the race image preview and draw it to the screen
    ofPushMatrix();
        ofTranslate(0,userImage.getHeight()*srcScale);
        ofScale(1,1);
        raceImage.draw(0,0);
    ofPopMatrix();
    
    //translate and scale the clone blend and draw it to the screen
    ofPushMatrix();
        ofTranslate(640,0);
        ofScale(1.2,1.2,1.2);
        finalClone.draw(0, 0);
    ofPopMatrix();
    
    //draw debug data to the screen
//   drawHighlightString(ofToString( blendingStrength), 300, 10);
//    drawHighlightString(ofToString( newScaleX), 300, 30);
//    drawHighlightString(ofToString( newScaleY), 300, 50);
//    drawHighlightString(ofToString( theAverageX), 300, 130);
    
	
}

void testApp::loadFace(string face){
    // load new user face image and set the framecounter back to 0
	userImage.loadImage(face);
    frameCounter=0;
}

void testApp::loadRace(string race){
    // load new race mask image and set the framecounter back to 0
	raceImage.loadImage(race);
    frameCounter=0;
}

void testApp::keyPressed(int key){
	switch(key){
        case OF_KEY_UP:
            currentFace++; // increment the current user face
            break;
        case OF_KEY_DOWN:
            currentFace--; //decrement the current user face
            break;
        case OF_KEY_RIGHT:
            currentRace++; // increment the current race face
            break;
        case OF_KEY_LEFT:
            currentRace--; //decrement the current race face
            break;
        case 'r':
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
    
    //keep the current face within the bounds of how many images there are in the folders
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

