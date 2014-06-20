#include "testApp.h"

void testApp::setup() {
	ofSetVerticalSync(true);
	bcol=0;
	plotHeight = 128;
	bufferSize = 1024;
    tBl = 60;
	
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
	
    ofSoundStreamSetup(0, 1, 44100, bufferSize, 4);
    
	
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
//    bargraph(tBuffer1,10.0, 32, 512, 800, 128);
//    bargraph(tBuffer2,50.0, 32, 256, 800, 128);
    ofSetColor(255, 255, 0);
    bargraph(tBuffer3,50.0, 32, 256, 800, 128);
    ofSetColor(255, 0, 0);
    bargraph(tBuffer4,50.0, 32, 256, 800, 128);
    
    if(isBeat) bcol=255;
    ofSetColor(bcol);
    ofFill();
    ofRect(10, 400, 500, 100);
    bcol = 0;
}

void testApp::update(){
    
    float nrg = 0.0;
    float sdiff = 0.0;
    float diff = 0.0;

    for (int i = 0; i < middleBins.size(); i++){
        nrg += middleBins[i];
        diff = drawBins[i]-middleBins[i];
        sdiff += (diff + abs(diff))*.5;
        //sdiff +=
    }
    
    tBuffer1.push_front(nrg);
    tBuffer1.pop_back();
    tBuffer2.push_front(sdiff);
    tBuffer2.pop_back();
    
    //Mittelwert berechnen
    float summ = 0.0;
    list<float>::iterator it;
    for (it = tBuffer2.begin(); it != tBuffer2.end(); ++it){
        summ += *it;
    }
    
    //Mittelwert auf Null setzten und Summe der Quadrate
    summ/=tBl;
    it   = tBuffer2.begin();
    float sq = 0.0;
    for (int i = 0; i < tBl; i++){
        tBuffer3[i] = *it-summ;
        it++;
        sq += tBuffer3[i]*tBuffer3[i];
    }
    
    //Varianz auf eins
    for (int i = 0; i < tBl; i++){
        tBuffer3[i] /=sqrt(sq);
    }
    
    //Peak pick
    int w = 3;
    int m = 4;
    float delta = 0.4;
    isBeat = true;
    summ = 0.0;
    for (int i = -w; i < w*m; i++){
        if (tBuffer3[w]<tBuffer3[w+i]) isBeat = false;
        summ += tBuffer3[w+i];
    }
    summ /= (m*w+w+1);
    if(tBuffer3[w] < summ+delta) isBeat = false;
    
    //cout << tBuffer3[w]-summ <<"\n";
    
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

void testApp::bargraph(vector<float> vl, float sca, int px, int py, int w, int h) {

    ofNoFill();
    ofRect(px, py, w, h);
    
    int n = vl.size();
    float sum = 0.0;

    int rw = w/(n);
    //int rw = 2;
    //ofSetRectMode(OF_RECTMODE_CENTER);

    ofBeginShape();
    int i = 0;
    for (vector<float>::iterator it  = vl.begin(); it != vl.end(); ++it){
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
    */
	for(int i = 0; i < fft->getBinSize(); i++) {
		audioBins[i] /= sqrt(audioBins[i]);
	}
    
	
	soundMutex.lock();
	middleBins = audioBins;
	soundMutex.unlock();
}

