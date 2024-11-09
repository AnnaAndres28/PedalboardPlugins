/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TremoloOSHEAudioProcessor::TremoloOSHEAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , state (*this, nullptr, "STATE", {
        /*
        std::make_unique<juce::AudioParameterFloat> ("rate", "Rate", 0.0f, 20.0f, 10.0f), // rate is in Hz
        std::make_unique<juce::AudioParameterFloat> ("depth", "Depth", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat> ("gain", "Gain", 0.0f, 1.0f, 1.0f)
        */
        
        std::make_unique<AudioParameterFloat> (ParameterID {"rate",  1}, "Rate", NormalisableRange<float> (0.0f, 20.0f), 5.0f), // rate is in Hz
        std::make_unique<AudioParameterFloat> (ParameterID {"depth", 1}, "Depth", NormalisableRange<float> (0.0f, 1.0f), 0.5f),
        std::make_unique<AudioParameterFloat> (ParameterID {"gain",  1}, "Gain", NormalisableRange<float> (0.0f, 1.0f), 1.0f)
    })
{
}

TremoloOSHEAudioProcessor::~TremoloOSHEAudioProcessor()
{
}

//==============================================================================
const juce::String TremoloOSHEAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TremoloOSHEAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TremoloOSHEAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TremoloOSHEAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TremoloOSHEAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TremoloOSHEAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TremoloOSHEAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TremoloOSHEAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TremoloOSHEAudioProcessor::getProgramName (int index)
{
    return {};
}

void TremoloOSHEAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TremoloOSHEAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need.
    position = 0; // Initial value for sample position within LFO signal
}

void TremoloOSHEAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TremoloOSHEAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TremoloOSHEAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    float rate = state.getParameter ("rate")->getValue();
    float depth = state.getParameter ("depth")->getValue();
    float gain = state.getParameter ("gain")->getValue();

    double sampleRate = this->getSampleRate();
    float w = 6.2831853 * rate / sampleRate; // w is in radians per sample
    float LFO = sin(w * position); // Low-frequency oscillator
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {    
            channelData[i] = channelData[i] * (depth * LFO + (1 - depth)); // Tremolo
            channelData[i] *= gain; // Gain
        }
    }
    position += 1;
    if (position >= (6.2831853 / w)) // Check if position is beyond number of samples in one LFO cycle
        position = 0;
}

//==============================================================================
bool TremoloOSHEAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TremoloOSHEAudioProcessor::createEditor()
{
    return new TremoloOSHEAudioProcessorEditor (*this);
}

//==============================================================================
void TremoloOSHEAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TremoloOSHEAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TremoloOSHEAudioProcessor();
}
