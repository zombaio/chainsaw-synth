/*
 * JuceProcessor.cpp
 *
 *  Created on: Nov 2, 2010
 *      Author: suva
 */

/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "JuceProcessor.h"

//==============================================================================
/** A demo synth sound that's just a basic sine wave.. */
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound()
    {
    }

    bool appliesToNote (const int midiNoteNumber)           { return true; }
    bool appliesToChannel (const int midiChannel)           { return true; }
};

//==============================================================================
/** A simple demo synth voice that just plays a sine wave.. */
class SineWaveVoice  : public SynthesiserVoice
{
public:
    SineWaveVoice()
        : angleDelta (0.0),
          tailOff (0.0)
    {
    }

    bool canPlaySound (SynthesiserSound* sound)
    {
        return dynamic_cast <SineWaveSound*> (sound) != 0;
    }

    void startNote (const int midiNoteNumber, const float velocity,
                    SynthesiserSound* sound, const int currentPitchWheelPosition)
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void stopNote (const bool allowTailOff)
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                                // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..

            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (const int newValue)
    {
        // can't be bothered implementing this for the demo!
    }

    void controllerMoved (const int controllerNumber, const int newValue)
    {
        // not interested in controllers in this case.
    }

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (sin (currentAngle) * level * tailOff);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        *outputBuffer.getSampleData (i, startSample) += currentSample;

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (sin (currentAngle) * level);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        *outputBuffer.getSampleData (i, startSample) += currentSample;

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle, angleDelta, level, tailOff;
};


//==============================================================================
ChainsawAudioProcessor::ChainsawAudioProcessor()
	: b(32000)
{
	p.vp.stereoSpread = 0;
	p.vp.detune = 0;
	p.vp.octaveSpread = 0;
	p.vp.numosc = 1;
	for(int i = 0; i < NUM_OSC_GROUPS; i++){
			p.vp.g[i].type = 0;
			p.vp.g[i].vol = 0.5;
			p.vp.g[i].octave = 0;

	}
	p.vp.volAttack = 0;
	p.vp.volDecay = 0;
	p.vp.volSustain = 1;
	p.vp.volRelease = 0;
	p.vp.filterResonance = 0;
	p.vp.filterCutoff = 1;

}

ChainsawAudioProcessor::~ChainsawAudioProcessor()
{
}

//==============================================================================
int ChainsawAudioProcessor::getNumParameters()
{
    return 0;
}

float ChainsawAudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
	return 0;
}

void ChainsawAudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
}

const String ChainsawAudioProcessor::getParameterName (int index)
{
    return String::empty;
}

const String ChainsawAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

//==============================================================================
void ChainsawAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    p.sFreq = sampleRate;
    keyboardState.reset();
}

void ChainsawAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
}

void ChainsawAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{


	MidiBuffer::Iterator mbi(midiMessages);
	MidiMessage message (0, 0.0);
	int time;

    b.size = buffer.getNumSamples();
    vm.process(&b, &p);

    while(mbi.getNextEvent(message, time)) {
    	if(message.isNoteOnOrOff()){
    		Note note;
			if(message.isNoteOn()){
				note.type = Note::NOTE_ON;
			} else {
				note.type = Note::NOTE_OFF;
			}
			note.note = message.getNoteNumber();
			note.velocity = message.getVelocity();
			vm.note(note);
    	}
    }

    for(int i = 0; i < b.size; i++) {
    	*buffer.getSampleData (0, i) += b.dataL[i];
    	*buffer.getSampleData (1, i) += b.dataR[i];
    }
}

//==============================================================================
AudioProcessorEditor* ChainsawAudioProcessor::createEditor()
{
    return 0;
}

//==============================================================================
void ChainsawAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute (T("uiWidth"), lastUIWidth);
    xml.setAttribute (T("uiHeight"), lastUIHeight);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void ChainsawAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != 0)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
        {
            // ok, now pull out our parameters..
            lastUIWidth  = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);

        }
    }
}

const String ChainsawAudioProcessor::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ChainsawAudioProcessor::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ChainsawAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ChainsawAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ChainsawAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ChainsawAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter(const String& commandLine)
{
    return new ChainsawAudioProcessor();
}
