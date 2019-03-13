#include "ofApp.h"

void ofApp::setupRecorder() {
    
    recorder.setVideoCodec("mpeg"); // 'ljpeg', 'jpeg2000' etc
//    recorder.setVideoBitrate("800k");
    
    ofAddListener(recorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    
    soundStream.setup(this, 0, 2, 44100, 256, 4);
    
}

void ofApp::recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete." << endl;
}

void ofApp::exit(){
    ofRemoveListener(recorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    recorder.close();
}

void ofApp::setupPiCam() {
#ifdef TARGET_RASPBERRY_PI
    ofxOMXCameraSettings settings;
    
    settings.sensorWidth = camWidth; //default 1280
    settings.sensorHeight = camHeight; //default 720
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
    
    ofLog() << "Setting up PiCam";
    vidGrabber.setup(settings);
    resize = 1;
    
    
    
#endif
}

//--------------------------------------------------------------
void ofApp::setup(){
    
    frameRate = 25;
    
    ofSetFrameRate(frameRate);
    
    bufferOrMpeg = false;
    
    camWidth = 640;  // try to grab at this size.
    camHeight = 480;
    resizeW = camWidth;
    resizeH = camHeight;

#ifndef TARGET_RASPBERRY_PI
    
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(frameRate);
    vidGrabber.initGrabber(camWidth, camHeight);
    
#endif
    
    
    maxSeconds = 99999.0;
    speed = 1.0;
    timeStamp = 0;
    memoryTimestamp = 0;
    
    
    if (recorder.hasVideoError()) {
        ofLogWarning("The video recorder failed to write some frames!");
    }
    
    if (recorder.hasAudioError()) {
        ofLogWarning("The video recorder failed to write some audio samples!");
    }
}


//--------------------------------------------------------------
void ofApp::update(){
    ofBackground(100, 100, 100);
    
    
#ifdef TARGET_RASPBERRY_PI

    
    if (ofGetElapsedTimef() - timeStamp >= (1.0/frameRate) && (isRecording)) {
        
        appendPixels();
        timeStamp = ofGetElapsedTimef();
    }
    
#else
    
    vidGrabber.update();
    if(vidGrabber.isFrameNew() && isRecording) {
        (bufferOrMpeg) ? appendBufferPixels() : appendRecorderFrame();
    }
    
#endif
    
}

void ofApp::appendRecorderFrame() {
    bool success = recorder.addFrame(vidGrabber.getPixels());
    if (!success) {
        ofLogWarning("This frame was not added!");
    }
}

void ofApp::appendBufferPixels() {
    
    ofPixels pixels;
    int ww = vidGrabber.getWidth() ;
    int hh = vidGrabber.getHeight() ;
    int rw = (float)ww * resize;
    int rh = (float)hh * resize;
    
    
#ifdef TARGET_RASPBERRY_PI
    
    pixels = vidGrabber.getPixels();
    
#else
    
    pixels.setFromPixels(vidGrabber.getPixels().getData(), ww, hh, 3);
    
#endif
    
    pixels.resize(rw, rh);
    
    if (frames.size() >= maxSeconds * ofGetFrameRate()) {
        frames.erase(frames.begin(), frames.begin() + 1);
    }
    frames.push_back(pixels);
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
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
    
    int ww = frames[(int)frameIndex].getWidth();
    int hh = frames[(int)frameIndex].getHeight();
    
    
    
    videoTexture.loadData(frames[(int)frameIndex].getData(), ww, hh, GL_RGB);
    
    /*
        ofTexture can report wrong width when mixed with differently sized frames:
        Scale to fit original pixels size
     */
    
    int w = (videoTexture.getWidth() / ww) * ofGetWidth();
    int h = (videoTexture.getHeight() / hh) * ofGetHeight();
    
    videoTexture.draw(0,0, w, h);

    if (isRecording) {
        ofSetColor(255,0,0);
        ofDrawCircle(100, 100, 20);
    }
    
    if ((ofGetElapsedTimef() - memoryTimestamp > 2)&&(isRecording)) {
        string msg;
#ifdef TARGET_RASPBERRY_PI
        msg = ofSystem("free -h");
#endif
    
#ifdef TARGET_OSX
        string cmd = "./data/report-osx.sh";
        cmd = "${PWD}/../../../data/report-osx.sh";
        msg = ofSystem(cmd);
    
#endif
        float time = ofGetElapsedTimef() - measureTimestamp;
        ofLog() << "[TIME] " << time;
        ofLog() << "[MEMORY] \n" << msg;
        memoryTimestamp = ofGetElapsedTimef();
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == 'b') {
        bufferOrMpeg = !bufferOrMpeg;
        
        if (!bufferOrMpeg) ofLog() << "[MODE] Record to Disk";
        if (bufferOrMpeg) ofLog() << "[MODE] Record to RAM";
    }
    
    if (key == 'r') {
        isRecording = !isRecording;
        measureTimestamp = ofGetElapsedTimef();
        
        if (!bufferOrMpeg) {
        
            if(isRecording && !recorder.isInitialized()) {
                ofLog() << "[RECORDER] Recording Video";
                recorder.setup("video-sampler"+ofGetTimestampString()+".mov", vidGrabber.getWidth(), vidGrabber.getHeight(), frameRate);
                recorder.start();
            }
            if (!isRecording && recorder.isInitialized()) {
                ofLog() << "[RECORDER] Saving Video";
                recorder.close();
            }
        }
    }
    if (key == 'c') {
        frames.clear();
    }
    if (key == OF_KEY_RIGHT) {
        if (speed < 0) speed *= -1;
    }
    if (key == OF_KEY_LEFT) {
        if (speed > 0) speed *= -1;
    }
    if (key == OF_KEY_UP) {
        resize += 0.25;
        if (resize > 1) resize = 1;
        ofLog() << "[RESIZE] " << camWidth * resize << " x " << camHeight * resize;
    }
    if (key == OF_KEY_DOWN) {
        resize -= 0.25;
        if (resize < 0.25) resize = 0.25;
        ofLog() << "[RESIZE] " << camWidth * resize << " x " <<camHeight * resize;
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
