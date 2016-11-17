#pragma once

#include "ofMain.h"
#include "gpioclass.h"
#include "ofxMaxim.h"
#include "ofxReverb.h"

#define _USE_LIVE_VIDEO
#define WAVES 5


class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();

    void audioIn(float *input, int bufferSize, int nChannels);
    void audioOut(float *output, int bufferSize, int nChannels);

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    //VIDEO
    int projW, projH, camW, camH;
    ofVideoGrabber grabber;
    ofImage gray;
    int pixLightness, sumLightness, pixAverage;
    int pixG, pixB, pixA, pixT, pixD;
    int minG, maxG, minB, maxB, minA, maxA;
    int minT, maxT, minD, maxD;
    float  bps;
    int countG, countB, countA, countT, countD, trigWave, hiNr;
    int bWaves [WAVES];

    //AUDIO IN
    vector <float> micIn;
    vector <float> buffer;
    vector <float> volHistory;
    int bufferCounter, drawCounter, bufferSize, N;
    int recPos, playPos, recEnabled, playEnabled, numCounted;
    float left, right, smoothedVol, curVol, scaledVol, duration, startTime, curTime, timeCount;
    ofSoundStream streamIn;

    //AUDIO OUT
    vector <float> leftOut;
    vector <float> rightOut;
    double freq;
    double outputs[2];
    unsigned sampleRate;
    double currentSample, delayOut;
    int currentCount,lastCount;
    maxiOsc osc, modulator;
    maxiDelayline delay;
    ofxReverb reverb;
    maxiMix mix;
    float	wetness, verbRoomSize, verbDamp, verbWidth;
    ofSoundStream streamOut;

    //MOTOR CONTROL
    GPIOClass* gpio17;
    GPIOClass* gpio18;
    GPIOClass* gpio22;
    GPIOClass* gpio23;
    int timeM1, timeM2, startTimeM, curTimeM1, curTimeM2, time17, time18, time22, time23;
    int maxM1, maxM2, sum17, sum18, sum22, sum23, center17, center18, center22, center23;
    int M1D, M2D;
    bool tglMotors;

    //GUI ctl
    bool showCtl;
    int iRange;
    ofXml XML;
    string xmlStructure;
    string message;
};
