// This code is based on the code credited below, but it has been modified
// further by Victor Zappi
 
/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

C++ Real-Time Audio Programming with Bela - Lecture 10: Latency
latency-tester-audio: Measures the latency from audio output to audio input
*/


#include "LDSP.h"
#include <cmath>

// Constants that define the program behaviour
const float kTestsPerSecond = 1.0;				// How many tests to run per second
const float kPulseLength = .02;					// Length of the output pulse
const float kInputLoThreshold = 0.05;			// Input thresholds: look for an edge
const float kInputHiThreshold = 0.1;			// from below the "lo" to above the "hi"

// Global variables
float gLastInput = 0;							// Previous value of the audio input
unsigned int gTestInterval = 0;					// Samples between successive tests
unsigned int gPulseLength = 0;					// How long is the pulse in samples?
unsigned int gTestCounter = 0;					// How many samples since the last test?
bool gInTest = false;							// Are we running a test?
bool gInputAboveThreshold = false;				// Is the input high or low?


bool setup(LDSPcontext *context, void *userData)
{
	// Initialise the number of samples between successive pulses
	// and the length of the pulse
	gTestInterval = context->audioSampleRate / kTestsPerSecond;
	gPulseLength = context->audioSampleRate * kPulseLength;
	
	return true;
}

void render(LDSPcontext *context, void *userData)
{
	// Loop through the number of audio frames
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		float input = 0;
		float output = 0;
		
		// Average the audio inputs
		for(unsigned int ch = 0; ch < context->audioInChannels; ch++) {
			input += fabsf(audioRead(context, n, ch));
		}
		input /= (float)context->audioInChannels;
		
		// Increment the test counter and, when it reaches the interval,
		// start a new test
		if(++gTestCounter >= gTestInterval) {
			// Reset event counter to 0
			gTestCounter = 0;
			
			// CHeck if we got a response to the last pulse and, if not,
			// print a message
			if(gInTest)
              LDSP_log("Pulse timeout\n");
				
			// Init the next Pulse
			gInTest = true;
		}
		
		// If we are counting samples, then look for a rising edge above the threshold
		if(gInTest) {
			if(!gInputAboveThreshold && input >= kInputHiThreshold) {
				// Edge found: stop counting samples	
				gInTest = false;
				gInputAboveThreshold = true;				
				
				// Calculate adjusted latency by subtracting twice the buffer size
				int hardwareSamples = gTestCounter - 2*context->audioFrames;
                int bufferingSamples = gTestCounter - hardwareSamples;
				
				// Print the calculated latency
              LDSP_log("Latency: %.2fms (%d samples) - buff: %.2fms (%d samples) - hw: %.2fms (%d samples)\n",
						  1000.0 * gTestCounter / context->audioSampleRate,
						  gTestCounter,
						  1000.0 * bufferingSamples / context->audioSampleRate,
						  bufferingSamples,
						  1000.0 * hardwareSamples / context->audioSampleRate,
						  hardwareSamples);
			}
			
			// Update the previous input
			gLastInput = input;
		}

		// Generate the pulse if we are still in its window
		if(gTestCounter < gPulseLength) {
			output = 1.0 - (float)gTestCounter / (float)gPulseLength;
		}
		else {
			output = 0.0;
		}
		
		// If the input falls below the lower threshold, set the flag accordingly
		if(input < kInputLoThreshold) {
			gInputAboveThreshold = false;
		}
		
		// Write the output to all the channels
		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++) {
			audioWrite(context, n, ch, output);
		}	
	}
}

void cleanup(LDSPcontext *context, void *userData)
{

}