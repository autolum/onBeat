#pragma once

#include "ofMain.h"
#include "ofxFft.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void plot(vector<float>& buffer, float scale, float offset);
	void audioReceived(float* input, int bufferSize, int nChannels);
	void draw();
    void update();
    

	int plotHeight, bufferSize;
    int tBl;

	ofxFft* fft;
	
	ofMutex soundMutex;
	vector<float> drawBins, middleBins, audioBins;
    list<float> tBuffer1, tBuffer2;
    vector<float> tBuffer3, tBuffer4;
    list<bool> tBeat;
    
    bool isBeat;
    float bcol;
    
    void bargraph(vector<float> vl, float sca, int px, int py, int w, int h);
};
