/*******************************************************************************

 name:             PhaserPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          https://oshe.io
 description:      phaser audio plugin.
 lastUpdated:	   March 17 2025 by Anna Andres

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors, juce_dsp,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        linux makefile

 type:             AudioProcessor
 mainClass:        PhaserProcessor

*******************************************************************************/

#pragma once


//==============================================================================
class PhaserProcessor final : public juce::AudioProcessor
{
public:

    //==============================================================================
    // Constructor that lets you define input/output channels as well as parameters and their bounds
    PhaserProcessor()
        : juce::AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo())
                                           .withOutput ("Output", juce::AudioChannelSet::stereo()))
    {
        // adding attack, release, threshold, and ratio parameters as well as their bounds
	// TODO: mess around with ranges and add comments on why these ranges were selected
	addParameter (rate = new juce::AudioParameterFloat ({ "rate", 1 }, "Rate", 0.0f, 100.0f, 50.0f));
	addParameter (depth = new juce::AudioParameterFloat ({ "depth", 1 }, "Depth", 0.0f, 1.0f, 0.5f));
	addParameter (centreFreq = new juce::AudioParameterFloat ({ "centreFreq", 1 }, "Centre Frequency", 0.0f, 100.0f, 50.0f)); 
	addParameter (feedback = new juce::AudioParameterFloat ({ "feedback", 1 }, "Feedback", -1.0f, 1.0f, 0.5f));
	addParameter (mix = new juce::AudioParameterFloat ({ "mix", 1 }, "Mix", 0.0f, 1.0f, 0.5f));
    }

    //==============================================================================
    // This function is used before audio processing. It lets you initialize variables and set up any other resources prior to running the plugin
    void prepareToPlay (double samplerate, int samplesPerBlock) override 
    {
	juce::dsp::ProcessSpec spec { samplerate, static_cast<uint32_t>(samplesPerBlock), static_cast<uint32_t>(getTotalNumOutputChannels()) };
	compressor.prepare(spec);
	compressor.setRate(50.0f);
	compressor.setDepth(0.5f);
	compressor.setCentreFrequency(50.0f);
	compressor.setFeedback(0.5f);
	compressor.setMix(0.5f);
    }
    // This function is usually called after the plugin stops taking in audio. It can deallocate any memory used and clean out buffers
    void releaseResources() override {}

    // This is where all the audio processing happens. One block of audio input is handled at a time.
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
    juce::dsp::AudioBlock<float> block (buffer);
	juce::dsp::ProcessContextReplacing<float> context (block);
	
	// read the values of the parameters in from the GUI
	auto rateValue = rate->get();
	auto depthValue = depth->get();
	auto centreFreqValue = centreFreq->get();
	auto feedbackValue = feedback->get();
	auto mixValue = mix->get();

	phaser.setRate(rateValue);
	phaser.setDepth(depthValue);
	phaser.setCentreFrequency(centreFreqValue);
	phaser.setFeedback(feedbackValue);
	phaser.setMix(mixValue)
	phaser.process(context);
        
        //for (int channel = 0; channel < buffer.getNumChannels(); ++channel) 
      //  {
       //     auto* channelData = buffer.getWritePointer(channel);
       //     for (int sample = 0; sample < buffer.getNumSamples(); ++sample) 
       //     {
		// TODO: process the audio sample-by-sample here
        //        float processedSample = channelData[sample] * gainValue;
                
                // write processed sample back to buffer
        //        channelData[sample] = processedSample;
        //    }
        //}
    }

    //==============================================================================
    // DO NOT CHANGE ANY OF THESE
    // This creates the GUI editor for the plugin
    juce::AudioProcessorEditor* createEditor() override          { return new juce::GenericAudioProcessorEditor (*this); }
    // We have a GUI editor for the plugin so we return true
    bool hasEditor() const override                        { return true;   }

    //==============================================================================
    const juce::String getName() const override                  { return "Phaser PlugIn"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

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
    // TODO: Save the value of your parameter to memory. Make sure you do this for every one of your parameters.
    void getStateInformation (juce::MemoryBlock& destData) override
    {
	juce::MemoryOutputStream (destData, true).writeFloat (*rate);
	juce::MemoryOutputStream (destData, true).writeFloat (*depth);
	juce::MemoryOutputStream (destData, true).writeFloat (*centreFreq);
	juce::MemoryOutputStream (destData, true).writeFloat (*feedback);
	juce::MemoryOutputStream (destData, true).writeFloat (*mix);
    }

    // This function recalls the state of the parameters from the last session ran and restores it into the parameter
    // TODO: Read the value into your parameter from memory. Make sure you do this for every one of your parameters.
    void setStateInformation (const void* data, int sizeInBytes) override
    {    
	rate->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
	depth->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
	centreFreq->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
	feedback->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
	mix->setValueNotifyingHost (juce::MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
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
    juce::dsp::Phaser<float> phaser;
    // TODO: This is where you define your audio parameters from the GUI that your code relies on in the process block. You can also define other variables here.
    // Example:
    juce::AudioParameterFloat* rate;
    juce::AudioParameterFloat* depth;
    juce::AudioParameterFloat* centreFreq;
    juce::AudioParameterFloat* feedback;
    juce::AudioParameterFloat* mix;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaserProcessor)
};
