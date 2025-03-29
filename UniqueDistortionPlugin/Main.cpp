#include <JuceHeader.h>
#include "UniqueDistortionPlugin.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UniqueDistortionProcessor();
}
