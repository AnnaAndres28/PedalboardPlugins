/*******************************************************************************

 name:             DistortionPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          https://oshe.io
 description:      distortion audio plugin.
 lastUpdated:	   March 25 2025 by Anna Andres

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors, juce_dsp,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        linux makefile

 type:             AudioProcessor
 mainClass:        DistortionProcessor

*******************************************************************************/

#pragma once


//==============================================================================
class DistortionProcessor final : public juce::AudioProcessor
{
public:

    //==============================================================================
    // Constructor that lets you define input/output channels as well as parameters and their bounds
    DistortionProcessor()
        : juce::AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo())
                                           .withOutput ("Output", juce::AudioChannelSet::stereo()))
    {
        addParameter (gain = new juce::AudioParameterFloat ({ "gain", 1 }, "Gain", 0.0f, 4.0f, 1.5f));
        addParameter (mode = new juce::AudioParameterFloat({ "mode", 1 }, "Mode", 0.0f, 7.0f, 0.0f));
        addParameter (sc1 = new juce::AudioParameterFloat({ "sc1", 1 }, "SoftClipping1", 1.0f, 10.0f, 1.0f));
        addParameter (sc2 = new juce::AudioParameterFloat({ "sc2", 1 }, "SoftClipping2", 0.0f, 0.33333f, 0.333f));
        addParameter (lowthres4 = new juce::AudioParameterFloat({ "lowthres4", 1 }, "LowThres4", 0.5f, 9.0f, 0.5f));
        addParameter (highthres4 = new juce::AudioParameterFloat({ "highthres4", 1 }, "highthres4", 0.5f, 9.0f, 0.5f));
        addParameter (nBits5 = new juce::AudioParameterFloat({ "nBits5", 1 }, "nBits5", 1.0f, 128.0f, 4.0f));
        addParameter (percentDrop6 = new juce::AudioParameterFloat({ "percentDrop6", 1 }, "PercentDrop6", 0.0f, 10.0f, 4.0f));
        addParameter (threshold7 = new juce::AudioParameterFloat({ "threshold7", 1 }, "Threshold7", 0.5f, 9.0f, 0.5f));
    }

    //==============================================================================
    // This function is used before audio processing. It lets you initialize variables and set up any other resources prior to running the plugin
    void prepareToPlay (double, int) override {}
    // This function is usually called after the plugin stops taking in audio. It can deallocate any memory used and clean out buffers
    void releaseResources() override {}

    // This is where all the audio processing happens. One buffer of audio input is handled at a time.
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        
        auto gainValue = gain->get();
        int modeValue = juce::roundToInt(mode->get());
        auto a1Value = sc1->get();
        auto a2Value = sc2->get();
        auto lthres4 = lowthres4->get();
        auto hthres4 = highthres4->get();
        auto nBits = nBits5->get(); // can be int or float for plugins
        auto ampValues = pow(2, nBits-1);
        auto pDrop = percentDrop6->get();
        auto thres7 = threshold7->get();
        float wavefoldThreshold = 0.05f / thres7;
        float lowThreshold = 0.05f / lthres4;
        float highThreshold = 0.05f / hthres4;
        
        switch(modeValue) {
            case 1: // soft clipping
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        float processedSample = channelData[sample] * gainValue; // applying gain
                        processedSample = 2/(juce::MathConstants<float>::pi)*atan(a1Value*processedSample); // apply soft clipping
                        channelData[sample] = processedSample;
                    }
                }
                break;
            case 2: // another type of soft clipping
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        float processedSample = channelData[sample] * gainValue; // applying gain
                        processedSample = processedSample-a2Value*pow(processedSample,3); // apply soft clipping
                        channelData[sample] = processedSample;
                    }
                }
                break;
            case 3: // harsher type of distortion essentially using a triangle wave LOUD AF
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        float processedSample = channelData[sample] * gainValue; // applying gain
                        processedSample = abs(fmod(2*processedSample+2,4))-1; // apply harsher gain
                        channelData[sample] = processedSample;
                    }
                }
                break;
            case 4: // pause distortion - self made
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        float processedSample = channelData[sample] * gainValue; // applying gain
                        if(processedSample <= (-1 * highThreshold)) {
                            processedSample = processedSample + (highThreshold - lowThreshold);
                        }
                        else if((-1 * highThreshold) < processedSample <= (-1 * lowThreshold)) {
                            processedSample = (-1 * lowThreshold);
                        }
                        else if(lowThreshold < processedSample <= highThreshold) {
                            processedSample = lowThreshold;
                        }
                        else if(processedSample > highThreshold) {
                            processedSample = processedSample - (highThreshold - lowThreshold);
                        }
                            channelData[sample] = processedSample;
                    }
                }
                break;
            case 5: // bit crushing GET RID OF ARTIFACTS
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        float processedSample = ceil(ampValues*gainValue*channelData[sample])*(1/ampValues); // apply bit crushing
                        channelData[sample] = processedSample;
                    }
                }
                break;
            case 6: // sample dropout
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        int randomNum = rand() % 100;
                        if(randomNum < pDrop) {
                            channelData[sample] = 0;
                            continue;
                        }
                        channelData[sample] = channelData[sample] * gainValue;
                    }
                }
                break;
            case 7: // wave folding
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                        float processedSample = channelData[sample] * gainValue; // applying gain
                        float dif = 0;
                        if(processedSample > wavefoldThreshold) {
                            dif = processedSample - wavefoldThreshold;
                            channelData[sample] = wavefoldThreshold - dif;
                        }
                        else if(processedSample < (-1 * wavefoldThreshold)) {
                            dif = wavefoldThreshold - processedSample;
                            channelData[sample] = wavefoldThreshold + dif;
                        }
                        else {
                            channelData[sample] = processedSample;
                        }
                    }
                }
                break;
            default: 
                // do nothing
                break;
        }
    }

    //==============================================================================
    // DO NOT CHANGE ANY OF THESE
    // This creates the GUI editor for the plugin
    juce::AudioProcessorEditor* createEditor() override          { return new juce::GenericAudioProcessorEditor (*this); }
    // We have a GUI editor for the plugin so we return true
    bool hasEditor() const override                        { return true;   }

    //==============================================================================
    const juce::String getName() const override                  { return "Distortion PlugIn"; }
    // This function returns a boolean for whether or not the plugin accepts Midi input. We don't. so this will be false
    bool acceptsMidi() const override                      { return false; }
    // This function returns a boolean for whether or not the plugin has Midi output. We don't. so this will be false
    bool producesMidi() const override                     { return false; }
    // This specifies how much longer there is output when the input stops. This would be helpful for reverb/delay but not so much for distortion/gain
    // A 0 tail length means that the output stops as soon as the input stops
    double getTailLengthSeconds() const override           { return 0; } //TODO: Change tail length if desired

    //==============================================================================
    // DO NOT CHANGE ANY OF THESE
    // This returns the number of presets/configurations for the plugin. We only have a default configuration so we return 1
    int getNumPrograms() override                          { return 1; }
    // This returns the index of the currently selected program. This will always be 0 for this plugin
    int getCurrentProgram() override                       { return 0; }
    // This allows the user to switch to a different program if you have multiple
    void setCurrentProgram (int) override                  {}
    // This gives you the name of the program for a given index
    const juce::String getProgramName (int) override             { return "None"; }
    // This allows you to change the name of a program at the given index
    void changeProgramName (int, const juce::String&) override   {}

    //==============================================================================
    // This function saves the current state of each parameter to memory so that we can load the state of each parameter 
    // in the next session of running the pedal
    void getStateInformation (juce::MemoryBlock& destData) override
    {
	    juce::MemoryOutputStream (destData, true).writeFloat (*gain);
        juce::MemoryOutputStream (destData, true).writeFloat (*mode);
        juce::MemoryOutputStream (destData, true).writeFloat (*sc1);
        juce::MemoryOutputStream (destData, true).writeFloat (*sc2);
        juce::MemoryOutputStream (destData, true).writeFloat (*lowthres4);
        juce::MemoryOutputStream (destData, true).writeFloat (*highthres4);
        juce::MemoryOutputStream (destData, true).writeFloat (*nBits5);
        juce::MemoryOutputStream (destData, true).writeFloat (*percentDrop6);
        juce::MemoryOutputStream (destData, true).writeFloat (*threshold7);
    }

    // This function recalls the state of the parameters from the last session ran and restores it into the parameter
    void setStateInformation (const void* data, int sizeInBytes) override
    {
        gain->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        mode->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        sc1->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        sc2->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        lowthres4->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        highthres4->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        nBits5->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        percentDrop6->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
        threshold7->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
    }

    //==============================================================================
    // This function checks to see if the requested input/output configuration is compatible with the coded plugin
    // DO NOT CHANGE THIS FUNCTION
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto& mainInLayout  = layouts.getChannelSet (true,  0);
        const auto& mainOutLayout = layouts.getChannelSet (false, 0);

        return (mainInLayout == mainOutLayout && (! mainInLayout.isDisabled()));
    }

private:
    //==============================================================================
    juce::AudioParameterFloat* gain;
    juce::AudioParameterFloat* mode;
    juce::AudioParameterFloat* sc1;
    juce::AudioParameterFloat* sc2;
    juce::AudioParameterFloat* lowthres4;
    juce::AudioParameterFloat* highthres4;
    juce::AudioParameterFloat* nBits5;
    juce::AudioParameterFloat* percentDrop6;
    juce::AudioParameterFloat* threshold7;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionProcessor)
};

