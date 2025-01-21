#include <JuceHeader.h>
#include "PassThru.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PassThruProcessor();
}
