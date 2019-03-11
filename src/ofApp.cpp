#include "ofApp.h"

void ofApp::setupPiCam() {
    frameRate = 25;
#ifdef TARGET_RASPBERRY_PI
    ofxOMXCameraSettings settings;
    
    settings.sensorWidth = camWidth; //default 1280
    settings.sensorHeight = camHeight; //default 720
    //    settings.drawCropRectangle.set(0,0,0,0);
    //    settings.sensorCropRectangle.set(0,0,736,576);
    settings.enableTexture = true; //default true
    settings.enableExtraVideoFilter = false;
    
    settings.enablePixels = true;
    settings.sensorMode = 7;
    settings.ISO = 0;
    settings.brightness = 50;
    settings.contrast = 0;
    settings.sharpness = 0;
    settings.saturation = 0;
    settings.autoISO = false;
    settings.autoShutter = false;
    
    //pass in the settings and it will start the camera
    
    ofLog() << "Setting up PiCam";
    vidGrabber.setup(settings);
    resize = 1;
    
#endif
}

//--------------------------------------------------------------
void ofApp::setup(){
    camWidth = 640;  // try to grab at this size.
    camHeight = 480;
    resizeW = camWidth;
    resizeH = camHeight;

#ifndef TARGET_RASPBERRY_PI
    
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(60);
    vidGrabber.initGrabber(camWidth, camHeight);
    
#endif
    

    videoInverted.allocate(camWidth, camHeight, OF_PIXELS_RGB);
    videoTexture.allocate(videoInverted);
    
    ofSetFrameRate(frameRate);
    
    maxSeconds = 99999.0;
    speed = 1.0;
    timeStamp = 0;
    memoryTimestamp = 0;
}


//--------------------------------------------------------------
void ofApp::update(){
    ofBackground(100, 100, 100);
    
    
#ifdef TARGET_RASPBERRY_PI

    
    if (ofGetElapsedTimef() - timeStamp >= (1.0/frameRate)) {
        
        if (isRecording) appendPixels();
        timeStamp = ofGetElapsedTimef();
    }
    
#else
    
    vidGrabber.update();
    if(vidGrabber.isFrameNew() && isRecording) appendPixels();
    
#endif
    
}

void ofApp::appendPixels() {
    
    ofPixels pixels;
    int ww = vidGrabber.getWidth() ;
    int hh = vidGrabber.getHeight() ;
    
    
#ifdef TARGET_RASPBERRY_PI
//    ofImage img;
//    img.setFromPixels(vidGrabber.getPixels(), vidGrabber.getWidth(), vidGrabber.getHeight(), OF_IMAGE_GRAYSCALE, true);
//    pixels = img.getPixels();
//    pixels.setFromPixels(vidGrabber.getRawPixels(), ww, hh, OF_IMAGE_GRAYSCALE);
    int rw = (float)ww * resize;
    int rh = (float)hh * resize;
    pixels = vidGrabber.getPixels();
    
//    ofLog() << "Pixels: " << frames.size() << " Width: " << rw << " Height: " << rh;
    pixels.resize(rw, rh);
    
#else
    
    pixels.setFromPixels(vidGrabber.getPixels().getData(), ww, hh, 3);
    
#endif
    if (frames.size() >= maxSeconds * ofGetFrameRate()) {
        frames.erase(frames.begin(), frames.begin() + 1);
    }
    frames.push_back(pixels);
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
//    ofLog() << "Frames: " << frames.size() << " Index: " << (int)frameIndex;
#ifdef TARGET_RASPBERRY_PI
    if (vidGrabber.isReady()) vidGrabber.draw(0,0,ofGetWidth(), ofGetHeight());
#else
    
    vidGrabber.draw(0,0,ofGetWidth(), ofGetHeight());
    
#endif
    
    frameIndex += speed;
    
    ofSetColor(255);
    
    if ((int)frameIndex < 0) frameIndex = frames.size() - 1;
    if ((int)frameIndex >= frames.size()) frameIndex = 0;
    
    if (frames.size() == 0) return;
    
    videoTexture.loadData(frames[(int)frameIndex]);
    videoTexture.draw(0,0,ofGetWidth(), ofGetHeight());
    
//    for (int i = 0; i < frames.size(); i++) {
//       int x =  ofRandom(0, ofGetWidth());
//       int y = ofRandom(0, ofGetHeight());
//            videoTexture.loadData(frames[i]);
//            videoTexture.draw(x,y,120, 120);
//
//    }
    
    if (isRecording) {
        ofSetColor(255,0,0);
        ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, 20);
    }
    
    
#ifdef TARGET_RASPBERRY_PI
    if (ofGetElapsedTimef() - memoryTimestamp > 2) {
        ofLog() << ofSystem("free -h");
        ofLog() << "Time elapsed recording: " << ofGetElapsedTimef() - measureTimestamp;
        memoryTimestamp = ofGetElapsedTimef();
    }
#endif
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == 'r') {
        isRecording = !isRecording;
        measureTimestamp = ofGetElapsedTimef();
    }
    if (key == OF_KEY_RIGHT) {
        if (speed < 0) speed *= -1;
    }
    if (key == OF_KEY_LEFT) {
        if (speed > 0) speed *= -1;
    }
    if (key == OF_KEY_UP) {
        if (resize != 1) resize += 0.25;
        ofLog() << "Resizing to: " << resize;
    }
    if (key == OF_KEY_DOWN) {
        if (resize != 0.25) resize -= 0.25;
        ofLog() << "Resizing to: " << resize;
    }
    if(isdigit(key)) {
        float direction = (speed < 0) ? -2 : 2;
        speed = ofMap(key, 49, 57, 0, direction);
    }
    if (key == ' ') {
        setupPiCam();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
//    speed = ofMap(x, 0, ofGetWidth(), -4, 4);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
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
