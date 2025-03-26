#include <JuceHeader.h>
#include "DistortionPlugin.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionProcessor();
}
