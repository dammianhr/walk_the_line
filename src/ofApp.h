#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxUI.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxBullet.h"
#include "ofxFBX.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    bool debug;
    
    ofxAssimpModelLoader model;
    ofxFBXScene scene;
    
    bool bAnimate;
    float animationPosition;
    
    ofPlanePrimitive plane;
    
    ofMesh mesh;
    ofMesh sceneMesh;
    ofEasyCam cam;
    ofEasyCam cam2;
    ofImage tex;
    ofImage tex_ground;
    ofLight lights[8];
    ofLight ambient;
    ofMaterial piso;
    
    // gui
    void setGui();
    void guiEvent(ofxUIEventArgs &);
    ofxUIScrollableCanvas *gui;
    bool save;
    
    int animationSpeed;
    
    //kinect
    ofxKinect kinect;
	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	
	ofxCvContourFinder contourFinder;
    
    int nearThreshold;
	int farThreshold;
    
    int minBlob;
    int maxBlob;
	
	int tilt;
    
    float x,y,z;
    int angle;
    double rad;
    int radius;
    int angleAmout;
    int timeOut;
    bool isOut;
    ofVec2f pos2D;
    
    float startTime; // store when we start time timer
    float endTime; // when do want to stop the timer
    bool  bTimerReached; // used as a trigger when we hit the timer
    
    //SETUP bullet phisycs
    ofxBulletWorldRigid             world;
    ofxBulletBox *                  ground;
    vector<ofxBulletCustomShape*>   boxs;
    vector <ofxBulletBox*>          box;
    shared_ptr<ofxBulletTriMeshShape>  bulletPlayer;
};
