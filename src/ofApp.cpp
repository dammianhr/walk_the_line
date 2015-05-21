#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(40);
    ofBackground(155);
    ofSetVerticalSync(true);
    ofSetSmoothLighting(true);
    
    ofDisableArbTex(); //we need GL_TEXTURE_2D for our models coords.
    
    debug = true;
    bAnimate = true;
    isOut = false;
    timeOut = 12;
    animationPosition = 0;
    animationSpeed = 1;
    angle = 90;
    radius = 10;
    angleAmout = 10;
    rad = 0.0;
    x,y = 0;
    
    //SETUP depth camera
	kinect.setRegistration(true);
	kinect.init(false,false);
	kinect.open();
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	nearThreshold = 230;
	farThreshold = 70;
    minBlob = 5;
    maxBlob = 20;
	
	tilt = 0;
	kinect.setCameraTiltAngle(tilt);
    
    setGui();
    gui->loadSettings("GUI/guiSettings.xml");
    gui->toggleVisible();
    
    model.loadModel("walker.dae",true);
    model.setPosition(0, 0 , -200);
    model.setLoopStateForAllAnimations(OF_LOOP_NORMAL);
    model.playAllAnimations();
    
    mesh = model.getCurrentAnimatedMesh(0);
    
    scene.load("escena_prueba.fbx");
    scene.setFrame(30);
    
    for (int i = 0; i<8; i++) {
        lights[i].setSpotlight(70.f,0.3f);
        lights[i].setDiffuseColor(ofFloatColor(1.f,1.f,1.f));
        lights[i].setSpecularColor(ofFloatColor(0.6f,0.6f,0.6f));
        lights[i].setAmbientColor(ofFloatColor(0.1f,0.1f,0.1f,0.3f));
        lights[i].setAttenuation();
        lights[i].rotate(-90, ofVec3f(1,0,0));
        lights[i].setPosition(0, 500, (8000/8)*i);
    }
    
    if(!bAnimate) {
        model.setPausedForAllAnimations(true);
    }
    
    tex.loadImage("path.jpg");
    tex_ground.loadImage("mapa_suelo.jpg");
    
    cam.setNearClip(20);
    cam.setFarClip(10500);
    
    plane.set(3000, 8000);
    plane.setMode(OF_PRIMITIVE_TRIANGLES);
    plane.setPosition(0, 4000, 0);
    
    //bullet
    world.setup();
	world.enableGrabbing();
	world.setCamera(&cam);
	world.setGravity( ofVec3f(0, -25., 0) );
    world.enableDebugDraw();
    
    bulletPlayer = shared_ptr<ofxBulletTriMeshShape>(new ofxBulletTriMeshShape());
    bulletPlayer->create(world.world, mesh, model.getPosition(), .3);
    bulletPlayer->add();
    bulletPlayer->enableKinematic();
    bulletPlayer->setActivationState(DISABLE_DEACTIVATION);
    
    ground = new ofxBulletBox;
    ground->create(world.world, scene.getMeshes()[0]->getPositionAtFrame(0), 0., 3000, 5, 8000);
    ground->setProperties(.25, .95);
    ground->add();
    
    for (int i = 0; i<395; i++) {
        boxs.push_back( new ofxBulletCustomShape() );
        boxs[i]->addMesh(scene.getMeshes()[i+61]->getMesh(), ofVec3f(1,1,1), true);
        boxs[i]->create(world.world, scene.getMeshes()[i+61]->getPositionAtFrame(1),5.);
        boxs[i]->add();
        //boxs[i]->setDamping(0.1,0.1);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    world.update();
    ofSetWindowTitle("Walk the Line - fps: "+ofToString(ofGetFrameRate(), 2));
    model.update();
    
    mesh = model.getCurrentAnimatedMesh(0);
    
    kinect.update();
    if (kinect.isFrameNew()) {
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        
        grayImage.mirror(false, true);
        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
        
        grayImage.flagImageChanged();
        
        contourFinder.findContours(grayImage, minBlob, maxBlob, 1, false);
    }
    
    //cam.setPosition(model.getPosition()+ofPoint(0,400,-150));
    //cam.lookAt(model.getPosition()+ofPoint(0,250,0));
    
    model.setRotation(1, angle+270, 0, 1, 0);
    
    rad = ofDegToRad(angle);
    x = -radius*cos(rad);
    y = radius*sin(rad);
    
    if (model.getPosition().z<7900) {
        model.setPosition(model.getPosition().x+x, 0, model.getPosition().z+y);

    }else{
        model.setPausedForAllAnimations(true);
    }
    
    cam.setPosition(model.getPosition()+ofPoint(-x,300,-450));
    //cam.setPosition(model.getPosition().x-(2*x), 300, model.getPosition().z-450);
    cam.lookAt(model.getPosition()+ofPoint(0,250,0));
    
    bulletPlayer->updateMesh(world.world, mesh);
    
    ofVec3f pos = model.getPosition();
    
    btTransform trans;
    trans.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)));
    trans.setRotation(btQuaternion(btVector3(0,1,0), btScalar(rad+(PI*1.5))));
    
    bulletPlayer->getRigidBody()->getMotionState()->setWorldTransform(trans);
    //bulletPlayer->getCollisionShape()->setLocalScaling(btVector3(0.3, 0.3, 0.3));
    bulletPlayer->activate();
    
    pos2D = ofVec2f(ofMap(model.getPosition().x, -1500, 1500, 0, tex.getWidth()),
                    ofMap(model.getPosition().z, 0, 8000, 0, tex.getHeight()));
    
    if (tex.getColor(pos2D.x,pos2D.y) == ofColor(0,0,0)) {
        isOut = true;
    }else{
        isOut = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofEnableDepthTest();
    glShadeModel(GL_SMOOTH);
    
    cam.begin();
    
    if (debug) {
        ofPushStyle();
        ofSetLineWidth(1.f);
        ofSetColor(255, 0, 200);
        //world.drawDebug();
        ofPopStyle();
    }
    
    for (int i = 0; i<8; i++) {
        lights[i].enable();
        if (debug) lights[i].draw();
    }

    ofEnableSeparateSpecularLight();
    
    ofPushMatrix();
    
    ofxAssimpMeshHelper & meshHelper = model.getMeshHelper(0);

    ofMaterial & material = meshHelper.material;
    
    if(meshHelper.hasTexture()){
        meshHelper.getTextureRef().bind();
    }
    
    bulletPlayer->transformGL();
    
    material.begin();
    mesh.drawFaces();
    material.end();
    
    bulletPlayer->restoreTramsformGL();
    
    if(meshHelper.hasTexture()){
        meshHelper.getTextureRef().unbind();
    }
    
    for (int i = 0; i<scene.getMeshes().size(); i++) {
        //scene.getMeshes()[i]->draw();
    }
    
    ofSetColor(21, 94, 15);/*
    ground.transformGL();
    scene.getMeshes()[0]->draw();//piso
    //ground.draw();
    ground.restoreTramsformGL();*/
    tex_ground.bind();
    ground->draw();//piso
    tex_ground.unbind();
    
    ofSetColor(204, 204, 0);
    scene.getMeshes()[1]->draw();//recorrido

    for (int i = 0; i<56; i++) {
        ofSetColor(200);
        scene.getMeshes()[i+4]->draw();//barandas
    }
    
    for (int i = 0; i<200; i++) {
        ofSetColor(0,101,0);
        boxs[i]->transformGL();
        scene.getMeshes()[i+61]->draw();//cajas verdes
        boxs[i]->restoreTramsformGL();
    }
    
    for (int i = 0; i<195; i++) {
        ofSetColor(200,0,0);
        boxs[i+200]->transformGL();
        scene.getMeshes()[i+261]->draw();//cajas rojas
        boxs[i+200]->restoreTramsformGL();
    }
    
    //ofSetColor(100);
    //scene.getMeshes()[456]->draw();//tanque
    
    ofPopMatrix();
    /*
    ofPushMatrix();
    ofRotateX(90);
    tex.bind();
    material.begin();
    plane.drawFaces();
    material.end();
    tex.unbind();
    ofPopMatrix();
    */
    cam.end();
    
    ofDisableDepthTest();
    //light.disable();
    for (int i = 0; i<8; i++) {
        lights[i].disable();
    }
    
    ofDisableLighting();
    ofDisableSeparateSpecularLight();

    if (debug) {
        grayImage.draw(ofGetWindowWidth()-330, 10, 320, 240);
        contourFinder.draw(ofGetWindowWidth()-330, 10, 320, 240);
        
        ofPushStyle();
        ofSetColor(255, 200);
        tex.draw(ofGetWindowWidth()*0.8, ofGetWindowHeight()*0.3, tex.getWidth()*0.3, tex.getHeight()*0.3);
        ofVec2f minPos;
        minPos.x = ofMap(model.getPosition().x, -1500, 1500, ofGetWindowWidth()*0.8, (ofGetWindowWidth()*0.8)+(tex.getWidth()*0.3));
        minPos.y = ofMap(model.getPosition().z, 0, 8000, ofGetWindowHeight()*0.3, (ofGetWindowHeight()*0.3)+(tex.getHeight()*0.3));
        if(isOut) ofSetColor(255, 0, 0);
        else ofSetColor(0, 255, 0);
        ofCircle(minPos, 5);
        ofPopStyle();
        
        ofPushStyle();
        ofSetColor(0, 255, 255);
        ofDrawBitmapString("isOut: " + ofToString(isOut), ofGetWindowWidth()*0.75, ofGetWindowHeight()*0.90);
        ofDrawBitmapString("time Out: " + ofToString(timeOut), ofGetWindowWidth()*0.75, ofGetWindowHeight()*0.92);
        ofDrawBitmapString("Pos: " + ofToString(pos2D), ofGetWindowWidth()*0.75, ofGetWindowHeight()*0.94);
        ofDrawBitmapString("colorPos: " + ofToString(tex.getColor(pos2D.x,pos2D.y)), ofGetWindowWidth()*0.75, ofGetWindowHeight()*0.96);
        ofPopStyle();
        
        ofSetColor(0, 255, 0, 150);
        ofDrawBitmapString("num animations for this model: " + ofToString(model.getAnimationCount()), ofGetWindowWidth()*0.75, 35);
        ofDrawBitmapString("playerPos: " + ofToString(model.getPosition()), ofGetWindowWidth()*0.75, 50);
        ofDrawBitmapString("nodes: " + ofToString(scene.getNulls().size()), ofGetWindowWidth()*0.75, 65);
        ofDrawBitmapString("mesh: "+ ofToString(scene.getMeshes().size()), ofGetWindowWidth()*0.75, 80);
        ofDrawBitmapString("time(s): "+ ofToString(ofGetElapsedTimef(),0), ofGetWindowWidth()*0.75, 95);
        ofDrawBitmapString("damping "+ ofToString(boxs[0]->getDamping()), ofGetWindowWidth()*0.75, 110);
        if (contourFinder.nBlobs>0) {
            ofDrawBitmapString("Blob Xpos: "+ofToString(contourFinder.blobs[0].centroid.x), ofGetWindowWidth()*0.75, 150);
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
		case ' ':
			bAnimate = !bAnimate;
			break;
            
        case 'g':
			gui->toggleVisible();
			break;
            
        case 'd':
			debug =! debug;
			break;
            
        case OF_KEY_LEFT:
			angle = angle+angleAmout;
			break;
            
        case OF_KEY_RIGHT:
			angle=angle-angleAmout;
			break;
            
        default:
            break;
    }
    //mesh = model.getMesh(0);
    
    model.setLoopStateForAllAnimations(OF_LOOP_NORMAL);
    model.playAllAnimations();
    if(!bAnimate) {
        model.setPausedForAllAnimations(true);
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.close();
    if (save) {
        gui->saveSettings("GUI/guiSettings.xml");
    }
    
    delete gui;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (gui->isHit(x, y)) {
        cam.disableMouseInput();
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if (gui->isHit(x, y)) {
        cam.enableMouseInput();
    }
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

//--------------------------------------------------------------
void ofApp::setGui(){
    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float w = 200 - xInit * 2;
    float h = 10;
    
    gui = new ofxUIScrollableCanvas(0, 0, w+xInit*2, ofGetWindowHeight());
	gui->addWidgetDown(new ofxUILabel("Walk the Line", OFX_UI_FONT_MEDIUM));
    gui->addSpacer(w, 2);
    gui->addSlider("Animation Speed", 1, 15, animationSpeed, w, h);
    gui->addSpacer(w*0.75, 1);
    gui->addSlider("Tilt", -30, 30, tilt, w, h);
    gui->addSlider("nearThreshold", 0, 255, nearThreshold, w, h);
    gui->addSlider("farThreshold", 0, 255, farThreshold, w, h);
    gui->addSlider("min Blob", 0, (kinect.width*kinect.height)/2, nearThreshold, w, h);
    gui->addSlider("max Blob", 0, (kinect.width*kinect.height)/2, farThreshold, w, h);
    gui->addSpacer(w*0.75, 1);
    gui->addToggle("Save GUI Setup", false);
    gui->addSpacer(w*0.75, 1);
    gui->addSlider("angle", 0, 180, angle, w, h);
    
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e){
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    
    // gui ----
    if (name == "Animation Speed") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        animationSpeed = slider->getScaledValue();
    }
    else if (name == "Tilt") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        kinect.setCameraTiltAngle(slider->getScaledValue());
    }
    else if (name == "nearThreshold") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        nearThreshold = slider->getScaledValue();
    }
    else if (name == "farThreshold") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        farThreshold = slider->getScaledValue();
    }
    else if (name == "min Blob") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        minBlob = slider->getScaledValue();
    }
    else if (name == "max Blob") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        maxBlob = slider->getScaledValue();
    }
    else if (name == "angle") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        angle = slider->getScaledValue();
    }
    else if (name == "roty") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        //y = slider->getScaledValue();
    }
    else if (name == "rotz") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        //z = slider->getScaledValue();
    }
    else if (name == "Save GUI Setup") {
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        save = toggle->getValue();
    }

}

