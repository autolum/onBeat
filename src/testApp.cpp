#include "testApp.h"

void testApp::setup() {
	ofSetVerticalSync(true);
	
	plotHeight = 128;
	bufferSize = 1024;
    tBl = 200;
	
	//fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
	// To use FFTW, try:
	fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
	
	drawBins.resize(fft->getBinSize());
	middleBins.resize(fft->getBinSize());
	audioBins.resize(fft->getBinSize());
    
    tBuffer1.resize(tBl,0.0);
    tBuffer2.resize(tBl,0.0);
    tBuffer3.resize(tBl,0.0);
    tBuffer4.resize(tBl,0.0);
    tBeat.resize(tBl,false);
	
	// 0 output channels,
	// 1 input channel
	// 44100 samples per second
	// [bins] samples per buffer
	// 4 num buffers (latency)
	
	ofSoundStreamSetup(0, 1, this, 44100, bufferSize, 4);
	
	ofBackground(0, 0, 0);
}

void testApp::draw() {
	ofSetColor(255);
	ofPushMatrix();
	ofTranslate(16, 16);
	
	ofDrawBitmapString("Frequency Domain", 0, 0);
	plot(drawBins, -plotHeight, plotHeight / 2);
	ofPopMatrix();
	string msg = ofToString((int) ofGetFrameRate()) + " fps";
	ofDrawBitmapString(msg, ofGetWidth() - 80, ofGetHeight() - 20);
    ofSetColor(255, 255, 255);
    bargraph(tBuffer1,10.0, 32, 512, 800, 128);
    bargraph(tBuffer2,50.0, 32, 256, 800, 128);
    ofSetColor(255, 255, 0);
    bargraph(tBuffer3,50.0, 32, 256, 800, 128);
    ofSetColor(255, 0, 0);
    bargraph(tBuffer4,50.0, 32, 256, 800, 128);
}

void testApp::update(){
    
    float nrg = 0.0;
    float sdiff = 0.0;
    float diff = 0.0;

    for (int i = 0; i < middleBins.size()/50; i++){
        nrg += middleBins[i];
        diff = drawBins[i]-middleBins[i];
        sdiff += (diff + abs(diff))*.5;
        //sdiff +=
    }
    
    float summ = 0.0;
    int i = 1;
    list<float>::iterator it;
    it   = tBuffer2.begin();
    for (int i = 0; i < 20; i++){
        summ += *it;
        it++;
    }
    summ/=20;
    
    
    //cout << ' ' << nrg;
    tBuffer1.push_front(nrg);
    tBuffer1.pop_back();
    tBuffer2.push_front(sdiff);
    tBuffer2.pop_back();
    tBuffer3.push_front(summ*2);
    tBuffer3.pop_back();
    float sd =sdiff-(summ*2);
    tBuffer4.push_front(sd+abs(sd));
    tBuffer4.pop_back();
    
    soundMutex.lock();
	drawBins = middleBins;
	soundMutex.unlock();
    
    
}

void testApp::plot(vector<float>& buffer, float scale, float offset) {
	ofNoFill();
	int n = buffer.size();
	ofRect(0, 0, n, plotHeight);
	glPushMatrix();
	glTranslatef(0, plotHeight / 2 + offset, 0);
	ofBeginShape();
	for (int i = 0; i < n; i++) {
		ofVertex(i, sqrt(buffer[i]) * scale);
        //ofLine(i,0,i,sqrt(buffer[i]) * scale);
	}
	ofEndShape();
	glPopMatrix();
}

void testApp::bargraph(list<float> vl, float sca, int px, int py, int w, int h) {

    ofNoFill();
    ofRect(px, py, w, h);
    
    int n = vl.size();
    float sum = 0.0;

    int rw = w/(n);
    //int rw = 2;
    //ofSetRectMode(OF_RECTMODE_CENTER);

    ofBeginShape();
    int i = 0;
    for (list<float>::iterator it  = vl.begin(); it != vl.end(); ++it){
        //if(*iit) ofSetColor(255,0,0,100);
        //else ofSetColor(255,100);
        //ofRect(i*rw+px, py + h/2, 1, *it*10*sca);
        ofVertex(i*rw+px, py+h-*it*sca);
        i++;
    }
    ofEndShape();
}

void testApp::audioReceived(float* input, int bufferSize, int nChannels) {	
	float maxValue = 0;
    
    /*
	for(int i = 0; i < bufferSize; i++) {
		if(abs(input[i]) > maxValue) {
			maxValue = abs(input[i]);
		}
	}
	for(int i = 0; i < bufferSize; i++) {
		input[i] /= maxValue;
	}
    */
    
	fft->setSignal(input);
    
	float* curFft = fft->getAmplitude();
	memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
	
    /*
	maxValue = 0;
	for(int i = 0; i < fft->getBinSize(); i++) {
		if(abs(audioBins[i]) > maxValue) {
			maxValue = abs(audioBins[i]);
		}
	}

	for(int i = 0; i < fft->getBinSize(); i++) {
		audioBins[i] /= maxValue;
	}
    */
	
	soundMutex.lock();
	middleBins = audioBins;
	soundMutex.unlock();
}

