#include "ofApp.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

//--------------------------------------------------------------
void ofApp::setup(){

    //Set generic framerate
    ofSetFrameRate(60);

    //we load our settings file
    //if it doesn't exist we can still make one
    //by hitting the 's' key
    if( XML.load("settings.xml") ){
        cout<<"settings.xml loaded!"<<endl;
    }else{
        // make a correct XML document, if exists
        // create root
        XML.addChild("RANGES");
        // move to root
        XML.setTo("RANGES");
    }
    //the double forward slash //MAXD means: search from the root of the xml for a tag could MAXD
    //otherwise the search would be relative to where you are in the xml structure. // = absolute
    if(XML.exists("MAXD")) {
        maxD	= XML.getValue<int>("MAXD");
    } else {
        maxD = 80;
    }
    if(XML.exists("MINT")) {
        minT	= XML.getValue<int>("MINT");
    } else {
        minT = 80;
    }
    if(XML.exists("MAXT")) {
        maxT	= XML.getValue<int>("MAXT");
    } else {
        maxT = 120;
    }
    if(XML.exists("MINA")) {
        minA	= XML.getValue<int>("MINA");
    } else {
        minA = 120;
    }
    if(XML.exists("MAXA")) {
        maxA	= XML.getValue<int>("MAXA");
    } else {
        maxA = 190;
    }
    if(XML.exists("MINB")) {
        minB	= XML.getValue<int>("MINB");
    } else {
        minB = 190;
    }
    if(XML.exists("MAXB")) {
        maxB	= XML.getValue<int>("MAXB");
    } else {
        maxB = 230;
    }
    if(XML.exists("MING")) {
        minG	= XML.getValue<int>("MING");
    } else {
        minG = 230;
    }
    //Set basic frame sizes
    projW = 640;
    projH = 480;
    //VIDEO
    //Define camera resolution
    camW = 320;
    camH = 240;
    //Point to camera device, initialize image frame recorded, and framerate
    grabber.setDeviceID(0);
    grabber.initGrabber(camW, camH);
    grabber.setDesiredFrameRate(60);
    //Allocate a grayscale image
    gray.allocate(camW, camH, OF_IMAGE_GRAYSCALE);
    //Implement starting values for pixel values, set it at least to 1 by default
    pixD = 1;
    pixT = 1;
    pixA = 1;
    pixB = 1;
    pixG = 1;
    //Ranges of grayscales for categories
    minD = 1;
    maxG = 255;
    //Count pixels dedicated to a category
    countD = 0;
    countT = 0;
    countA = 0;
    countB = 0;
    countG = 0;
    // base freq of the lowest sine wave in cycles per second (hertz)
    freq = 0;
    //AUDIO
    bufferSize = 512;       //Define buffersize of sound
    sampleRate = 48000;     //Define sample rate of sound
    //AUDIO IN
    duration = 4;       //Duration of recorded sound in seconds
    N = duration * sampleRate;      //Size of the PCM buffer
    streamIn.printDeviceList();     //Print a list of all audio devices
    streamIn.setDeviceID(1);        //Point to audio record device
    streamIn.setup(this, 0, 1, sampleRate, bufferSize, 2);      //Setup of audio input stream
    buffer.resize(N, 0.0);   //Assign PCM buffer of sound sample and fill it with zeros
    recPos = 0;
    playPos = 0;
    recEnabled = 0;
    playEnabled = 0;
    //Set up a volume monitor to create a trigger threshold for motors and audio sampling
    micIn.assign(bufferSize, 0.0);
    volHistory.assign(400, 0.0);
    bufferCounter = 0;
    drawCounter = 0;
    smoothedVol = 0.0;
    scaledVol = 0.0;
    //AUDIO OUT
    //Point to sound card and setup audio output stream
    streamOut.setDeviceID(1);
    streamOut.setup(this, 2, 0, sampleRate, bufferSize, 2);
    leftOut.assign(bufferSize, 0.0);
    rightOut.assign(bufferSize, 0.0);
    //Set reverb values
    wetness = 0.4f;
    verbRoomSize = 0.7f;
    verbDamp = 0.2f;
    verbWidth	= 0.99f;

    //MOTOR CONTROL
    //Initiate connected pins
    gpio17 = new GPIOClass("17"); //create new GPIO objects
    gpio18 = new GPIOClass("18");
    gpio22 = new GPIOClass("22");
    gpio23 = new GPIOClass("23");
    //Create needed system files
    gpio17->export_gpio();
    gpio18->export_gpio();
    gpio22->export_gpio();
    gpio23->export_gpio();
    //Set the direction of GPIO pins to out
    gpio17->setdir_gpio("out"); //GPIO17 set to output
    gpio18->setdir_gpio("out");
    gpio22->setdir_gpio("out");
    gpio23->setdir_gpio("out");
    //Set timing and max value
    maxM1 = 400;
    maxM2 = 500;
    timeM1 = 0;
    timeM2 = 0;
    time17 = maxM1 / 2;
    time18 = maxM1 / 2;
    time22 = maxM2 / 2;
    time23 = maxM2 / 2;
    M1D = 0;
    M2D = 0;
    hiNr = 0;
    tglMotors = false;
    trigWave = 0;
    showCtl = false;
    iRange = 0;
}

//--------------------------------------------------------------
void ofApp::update(){

//VIDEO
    //Update camera to get the next frame
    grabber.update();

    if ( grabber.isFrameNew() ) {	//Check for new frame

        sumLightness = 0;
        pixAverage = 0;
        pixD = minD;
        pixT = minT;
        pixA = minA;
        pixB = minB;
        pixG = minG;

        countD = 1;
        countT = 1;
        countA = 1;
        countB = 1;
        countG = 1;
        hiNr = 0;
        trigWave = 0;

        //Get pixels of the grayscale image for lightness reading
        ofPixels &pixels = grabber.getPixels();
        //get pixels from webcam image
        gray.setFromPixels(pixels);
        //No color needed for the monochrome tv of the project
        gray.setImageType(OF_IMAGE_GRAYSCALE);
        //Calculate values for lightness in particular grayscale ranges
        for(int x = 0; x < camW; x++){
            for(int y = 0; y < camH; y++){
                pixLightness = pixels.getColor(x, y).getLightness();
                if(pixLightness > minG && pixLightness < maxG){
                    pixG = pixG + pixLightness;
                    countG++;
                }
                if(pixLightness >= minB && pixLightness < maxB){
                    pixB = pixB + pixLightness;
                    countB++;
                }
                if(pixLightness >= minA && pixLightness < maxA){
                    pixA = pixA + pixLightness;
                    countA++;
                }
                if(pixLightness >= minT && pixLightness < maxT){
                    pixT = pixT + pixLightness;
                    countT++;
                }
                if(pixLightness >= minD && pixLightness < maxD){
                    pixD = pixD + pixLightness;
                    countD++;
                }
                sumLightness = sumLightness + pixLightness;
            }
        }
        //Calculate mean of lightness in total
        pixAverage = sumLightness / (camW * camH);
        pixG = pixG / countG;
        pixB = pixB / countB;
        pixA = pixA / countA;
        pixT = pixT / countT;
        pixD = pixD / countD;
        //Add counted pixels of each section into array
        bWaves[0] = countG;
        bWaves[1] = countB;
        bWaves[2] = countA;
        bWaves[3] = countT;
        bWaves[4] = countD;
        //Compare values of array to get highest number
        for(int i = 0; i < 5; i++){
            if(bWaves[i] > hiNr){
                trigWave = i;
                hiNr = bWaves[i];
            }
        }
        //Generate beats per second by mapping dedicated pixel value
        //into repetition range of real brain waves
        if(trigWave == 0){
            bps = ofMap(pixG, minG, maxG, 40, 100);
        }
        if(trigWave == 1){
            bps = ofMap(pixB, minB, maxB, 12, 40);
        }
        if(trigWave == 2){
            bps = ofMap(pixA, minA, maxA, 8, 12);
        }
        if(trigWave == 3){
            bps = ofMap(pixT, minT, maxT, 4, 8);
        }
        if(trigWave == 4){
            bps = ofMap(pixD, minD, maxD, 0.1, 4);
        }
        //Use overall mean of pixels summary to generate sound frequency
        freq = ofMap(pixAverage, 0, 255, 0, 2700);
    }

//VOLUME METER

    //lets scale the vol up to a 0-1 range
    scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
    //lets record the volume into an array
    volHistory.push_back( scaledVol );
    //if we are bigger the the size we want to record - lets drop the oldest value
    if( volHistory.size() >= 100 ){
        volHistory.erase(volHistory.begin(), volHistory.begin()+1);
    }
    //cout<<scaledVol<<endl;

    if(scaledVol == 1.0){
        if(recEnabled == 1 && timeCount < (duration * 1000)){
            recEnabled = 1;
        }
        else if(recEnabled == 1 && timeCount >= (duration * 1000)){
            recEnabled = 0;
        }
        else{
            recEnabled = 1;
            startTime = ofGetElapsedTimeMillis();
        }
        playEnabled = 0;
    }
    //activate motors and audio sampling as soon scaled volume reaches threshold
    if(recEnabled == 1)
    {
        curTime = ofGetElapsedTimeMillis();
        timeCount = curTime - startTime;
        cout<<timeCount<<endl;
        if(timeCount >= (duration * 1000)){
            cout << "Sound sample recorded" << endl;
            recEnabled = 0;
            playEnabled = 1;
            tglMotors = true;
            timeCount = 0;
            cout<<"The Motors Get Activated!!!"<<endl;
            //cout<<tglMotors<<endl;
        }
    }

    //MOTOR CONTROL
        if(tglMotors == true){
            //Get a direction of the motor randomly, while 1 is one direction, -1 the opposite direction, and 0 is no direction
            M1D = ofRandom(4) - 2;
            M2D = ofRandom(4) - 2;
            cout<<M1D<<" : "<<M2D<<endl;
            //Get a starting time
            startTimeM = ofGetElapsedTimeMillis();
            //Switch off motor trigger, as all values are now set
            tglMotors = false;
        }
         //Switch on/off of motor 1 according to value
         if(M1D == 1){
             //Generate a random time within the possible time range
            time17 = ofRandom(maxM1 / 2);
            if((sum17 + time17) <= maxM1){
                //Get initial time
                curTimeM1 = ofGetElapsedTimeMillis();
                //set energy flow to pin 17 of GPIO
                gpio17->setval_gpio("1");
                //Switch off the motor as soon random time is over
                if(curTimeM1 - startTimeM >= time17){
                    gpio17->setval_gpio("0");
                    //Add time to summary
                    sum17 = sum17 + time17;
                    //Balance it with summary of opposite direction
                    sum18 = sum18 - time17;
                    //Set random time to 0
                    time17 = 0;
                    //Set direction to 0
                    M1D = 0;
                }
            }
            if((sum17 + time17) > maxM1){
                //If the summary and random time is bigger than max
                //only use the difference
                time17 = maxM1 % time17;
                //Get initial time
                curTimeM1 = ofGetElapsedTimeMillis();
                //set energy flow to pin 17 of GPIO
                gpio17->setval_gpio("1");
                //Switch off the motor when random time is over
                if(curTimeM1 - startTimeM >= time17){
                    gpio17->setval_gpio("0");
                    //Add random value to summary
                    sum17 = sum17 + time17;
                    //Balance it with summary of opposite direction
                    sum18 = sum18 - time17;
                    //Set random time to 0
                    time17 = 0;
                    //Set initial time to 0
                    M1D = 0;
                }
            }
        }
        else if(M1D == -1){
              //Generate a random time within the possible time range
             time18 = ofRandom(maxM1 / 2);
             if((sum18 + time18) <= maxM1){
                 //Get initial time
                 curTimeM1 = ofGetElapsedTimeMillis();
                 //set energy flow to pin 17 of GPIO
                 gpio18->setval_gpio("1");
                 //Switch off the motor as soon random time is over
                 if(curTimeM1 - startTimeM >= time18){
                     gpio18->setval_gpio("0");
                     //Add time to summary
                     sum18 = sum18 + time18;
                     //Balance it with summary of opposite direction
                     sum17 = sum17 - time18;
                     //Set random time to 0
                     time18 = 0;
                     //Set direction to 0
                     M1D = 0;
                 }
             }
             if((sum18 + time18) > maxM1){
                 //If the summary and random time is bigger than max
                 //only use the difference
                 time18 = maxM1 % time18;
                 //Get initial time
                 curTimeM1 = ofGetElapsedTimeMillis();
                 //set energy flow to pin 17 of GPIO
                 gpio18->setval_gpio("1");
                 //Switch off the motor when random time is over
                 if(curTimeM1 - startTimeM >= time18){
                     gpio18->setval_gpio("0");
                     //Add random value to summary
                     sum18 = sum18 + time18;
                     //Balance it with summary of opposite direction
                     sum17 = sum17 - time18;
                     //Set random time to 0
                     time18 = 0;
                     //Set initial time to 0
                     M1D = 0;
                 }
            }
          }
          else if(M1D == 0){
            gpio17->setval_gpio("0");
            gpio18->setval_gpio("0");
        }
         else{
             gpio17->setval_gpio("0");
             gpio18->setval_gpio("0");
             cout<<"Something went wrong, motors set to 0"<<endl;
         }
         //Switch on/off of motor 1 according to value
         if(M2D == 1){
             //Generate a random time within the possible time range
            time22 = ofRandom(maxM2 / 2);
            if((sum22 + time22) <= maxM2){
                //Get initial time
                curTimeM2 = ofGetElapsedTimeMillis();
                //set energy flow to pin 22 of GPIO
                gpio22->setval_gpio("1");
                //Switch off the motor as soon random time is over
                if(curTimeM2 - startTimeM >= time22){
                    gpio22->setval_gpio("0");
                    //Add time to summary
                    sum22 = sum22 + time22;
                    //Balance it with summary of opposite direction
                    sum23 = sum23 - time22;
                    //Set random time to 0
                    time22 = 0;
                    //Set direction to 0
                    M2D = 0;
                }
            }
            if((sum22 + time22) > maxM2){
                //If the summary and random time is bigger than max
                //only use the difference
                time22 = maxM2 % time22;
                //Get initial time
                curTimeM2 = ofGetElapsedTimeMillis();
                //set energy flow to pin 22 of GPIO
                gpio22->setval_gpio("1");
                //Switch off the motor when random time is over
                if(curTimeM2 - startTimeM >= time22){
                    gpio22->setval_gpio("0");
                    //Add random value to summary
                    sum22 = sum22 + time22;
                    //Balance it with summary of opposite direction
                    sum23 = sum23 - time22;
                    //Set random time to 0
                    time22 = 0;
                    //Set initial time to 0
                    M2D = 0;
                }
            }
        }
        else if(M2D == -1){
              //Generate a random time within the possible time range
             time23 = ofRandom(maxM2 / 2);
             if((sum23 + time23) <= maxM2){
                 //Get initial time
                 curTimeM2 = ofGetElapsedTimeMillis();
                 //set energy flow to pin 23 of GPIO
                 gpio23->setval_gpio("1");
                 //Switch off the motor as soon random time is over
                 if(curTimeM2 - startTimeM >= time23){
                     gpio23->setval_gpio("0");
                     //Add time to summary
                     sum23 = sum23 + time23;
                     //Balance it with summary of opposite direction
                     sum22 = sum22 - time23;
                     //Set random time to 0
                     time23 = 0;
                     //Set direction to 0
                     M2D = 0;
                 }
             }
             if((sum23 + time23) > maxM2){
                 //If the summary and random time is bigger than max
                 //only use the difference
                 time23 = maxM2 % time23;
                 //Get initial time
                 curTimeM2 = ofGetElapsedTimeMillis();
                 //set energy flow to pin 17 of GPIO
                 gpio23->setval_gpio("1");
                 //Switch off the motor when random time is over
                 if(curTimeM2 - startTimeM >= time23){
                     gpio23->setval_gpio("0");
                     //Add random value to summary
                     sum23 = sum23 + time23;
                     //Balance it with summary of opposite direction
                     sum22 = sum17 - time23;
                     //Set random time to 0
                     time23 = 0;
                     //Set initial time to 0
                     M2D = 0;
                 }
            }
          }
          else if(M2D == 0){
            gpio22->setval_gpio("0");
            gpio23->setval_gpio("0");
        }
         else{
             gpio22->setval_gpio("0");
             gpio23->setval_gpio("0");
             cout<<"Something went wrong, motors set to 0"<<endl;
         }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //Draw camera frame
    grabber.draw(16, 0, projW, projH);

    // draw some labels
    if(showCtl == true){
        ofSetColor(80, 80, 80);
        ofDrawBitmapString("Influence range: " + ofToString(iRange), 100, 80);
        ofDrawBitmapString("Range 1   " + ofToString(minT), 100, 100);
        ofDrawBitmapString("Range 2   " + ofToString(minA), 100, 120);
        ofDrawBitmapString("Range 3   " + ofToString(minB), 100, 140);
        ofDrawBitmapString("Range 4   " + ofToString(minG), 100, 160);
    }
}

//AUDIO IN
//--------------------------------------------------------------
void ofApp::audioIn(float *input, int bufferSize, int nChannels){
    curVol = 0.0;
    // samples are "interleaved"
    numCounted = 0;
    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        if(recEnabled && timeCount <= (duration * 1000)){
            micIn[i] = input[i*2];
            curVol += micIn[i] * micIn[i];
            numCounted+=1;
            buffer[ recPos ] = input[i*2];
            recPos++;
            recPos %= N;
        }
        else{
            micIn[i] = input[i*2];
            curVol += micIn[i] * micIn[i];
            numCounted+=1;
        }
    }
    //this is how we get the mean of rms :)
    curVol /= (float)numCounted;
    // this is how we get the root of rms :)
    curVol = sqrt( curVol );
    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;
    bufferCounter++;
}

//--------------------------------------------------------------
void ofApp::audioOut(float *output, int bufferSize, int nChannels){

    /* update internal reverb parameter values */
    //	reverb.setmode(GetParameter(KMode));
    reverb.setroomsize(verbRoomSize);
    reverb.setdamp(verbDamp);
    reverb.setwidth(verbWidth);
    reverb.setwet(wetness);
    reverb.setdry(1.0f - wetness);

    for(int i = 0; i < bufferSize; i++){
        //Create sound
        switch (trigWave) {
            case 0:
                currentSample = (osc.triangle(freq) * (modulator.square(bps) + 1)) / 2;
                break;
            case 1:
                currentSample = (osc.sinewave(freq) * (modulator.square(bps) + 1)) / 2;
                break;
            case 2:
                currentSample = (osc.sinewave(freq) * (modulator.square(bps) + 1)) / 2;
                break;
            case 3:
                currentSample = (osc.saw(freq) * (modulator.square(bps) + 1)) / 2;
                break;
            case 4:
                currentSample = (osc.pulse(freq, 0.2) * (modulator.square(bps) + 1)) / 2;
                break;
        }
        if(playEnabled){
            currentSample = (currentSample + buffer[playPos]) * 0.5;
            playPos++;
            //When the end of buffer is reached, playPos sets
            //to 0, so we hear looped sound
            playPos %= N;
        }
        //Add some delay
        currentSample = (currentSample + (delay.dl(currentSample, freq, 0.8) * 0.9)) / 2;
        //Assign sample to left and right channels
        left = currentSample * 0.5;
        right = left;
        //Add some reverb
        reverb.processreplace(&left, &right, &output[i * nChannels], &output[i * nChannels + 1], 1, 1);

        //Adjust processed sample to output
        leftOut[i] = output[i * nChannels];
        rightOut[i] = output[i * nChannels + 1];
    }
}

//--------------------------------------------------------------
void ofApp::exit(){

    //MOTOR CONTROL
    //First, switch of all motors
    gpio17->setval_gpio("0");
    gpio18->setval_gpio("0");
    gpio22->setval_gpio("0");
    gpio23->setval_gpio("0");
    //Set starting time to current time
    startTime = ofGetElapsedTimeMillis();
    //Now check the position through time summary
    if(sum17 < maxM1 / 2){
        center17 = maxM1 / 2 - sum17;
        curTime = ofGetElapsedTimeMillis();
        if(curTime - startTime < center17){
            gpio17->setval_gpio("1");
        }
        else{
            gpio17->setval_gpio("0");
        }
    }
    else if(sum18 < maxM1 / 2){
        center18 = maxM1 / 2 - sum18;
        curTime = ofGetElapsedTimeMillis();
        if(curTime - startTime < center18){
            gpio18->setval_gpio("1");
        }
        else{
            gpio18->setval_gpio("0");
        }
    }
    if(sum22 < maxM2 / 2){
        center22 = maxM2 / 2 - sum22;
        curTime = ofGetElapsedTimeMillis();
        if(curTime - startTime < center22){
            gpio22->setval_gpio("1");
        }
        else{
            gpio22->setval_gpio("0");
        }
    }
    else if(sum23 < maxM2 / 2){
        center23 = maxM2 / 2 - sum23;
        curTime = ofGetElapsedTimeMillis();
        if(curTime - startTime < center23){
            gpio23->setval_gpio("1");
        }
        else{
            gpio23->setval_gpio("0");
        }
    }
    //Free the system from addressed GPIOs
    gpio17->unexport_gpio();
    gpio18->unexport_gpio();
    gpio22->unexport_gpio();
    gpio23->unexport_gpio();

    cout<<"IterNoise closed"<<endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'f'){
        recEnabled = 1;
    }
    if(key == 'r'){
        playEnabled = 1;
    }
    if(key == 'p'){
        ofToggleFullscreen();
    }
    if(key == 'm'){
        showCtl=!showCtl;
        iRange = 0;
        cout<<showCtl<<endl;
    }
    if(key == '1'){
        iRange = 1;
    }
    if(key == '2'){
        iRange = 2;
    }
    if(key == '3'){
        iRange = 3;
    }
    if(key == '4'){
        iRange = 4;
    }
    if(key == OF_KEY_UP){
        if(iRange == 1){
            maxD = maxD + 1;
            minT = minT + 1;
            if(minT >= maxT){
                maxD = maxT - 1;
                minT = maxT - 1;
            }
        }
        else if(iRange == 2){
            maxT = maxT + 1;
            minA = minA + 1;
            if(minA >= maxA){
                maxT = maxA - 1;
                minA = maxA -1;
            }
        }
        else if(iRange == 3){
            maxA = maxA + 1;
            minB = minB + 1;
            if(minB >= maxB){
                maxA = maxB - 1;
                minB = maxB - 1;
            }
        }
        else if(iRange == 4){
            maxB = maxB + 1;
            minG = minG + 1;
            if(minG >= maxG){
                minG = maxG - 1;
                maxB = maxG - 1;
            }
        }
    }
    if(key == OF_KEY_DOWN){
        if(iRange == 1){
            maxD = maxD - 1;
            minT = minT - 1;
            if(maxD <= minD){
                maxD = minD + 1;
                minT = minD + 1;
            }
        }
        else if(iRange == 2){
            maxT = maxT - 1;
            minA = minA - 1;
            if(maxT <= minT){
                maxT = minT + 1;
                minA = minT + 1;
            }
        }
        else if(iRange == 3){
            maxA = maxA - 1;
            minB = minB - 1;
            if(maxA <= minA){
                maxA = minA + 1;
                minB = minA + 1;
            }
        }
        else if(iRange == 4){
            maxB = maxB - 1;
            minG = minG - 1;
            if(maxB <= minB){
                maxB = minB + 1;
                minG = minB + 1;
            }
        }
    }
    //no data gets saved unless you hit the s key
    if(key == 's')
    {
        // did we already make a color? Because if so, we don't
        // even need to go down into that node, we can just use the //
        // syntax to set it because there's only 1 in the document
        if(XML.exists("RANGES"))
        {
            XML.setValue("MAXD", ofToString(maxD));
            XML.setValue("MINT", ofToString(minT));
            XML.setValue("MAXT", ofToString(maxT));
            XML.setValue("MINA", ofToString(minA));
            XML.setValue("MAXA", ofToString(maxA));
            XML.setValue("MINB", ofToString(minB));
            XML.setValue("MAXB", ofToString(maxB));
            XML.setValue("MING", ofToString(minG));
        }
        else
        {
            // otherwise, we haven't made one, so let's go ahead
            // and do that! First make a <RANGES>
            XML.addChild("RANGES");
            XML.setTo("RANGES");
            // now add a MAX and MIN of wave ranges
            XML.addValue("MAXD", ofToString(maxD));
            XML.addValue("MINT", ofToString(minT));
            XML.addValue("MAXT", ofToString(maxT));
            XML.addValue("MINA", ofToString(minA));
            XML.addValue("MAXA", ofToString(maxA));
            XML.addValue("MINB", ofToString(minB));
            XML.addValue("MAXB", ofToString(maxB));
            XML.addValue("MING", ofToString(minG));

        }
        XML.save("settings.xml");
        cout<< "settings saved to xml!"<<endl;
    }
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
