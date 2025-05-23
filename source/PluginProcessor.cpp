#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PressedProcessor::PressedProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                     apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // caching parameter pointers
    thresholdParam = apvts.getRawParameterValue("threshold");
    ratioParam = apvts.getRawParameterValue("ratio");
    attackParam = apvts.getRawParameterValue("attack");
    releaseParam = apvts.getRawParameterValue("release");
    makeupGainParam = apvts.getRawParameterValue("makeupGain");
}

PressedProcessor::~PressedProcessor()
{
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout PressedProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat> ("threshold", "Threshold", -60.0f, 0.0f, -24.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", 1.0f, 20.0f, 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.1f, 100.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 10.0f, 500.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("makeupGain", "Makeup Gain", 0.0f, 24.0f, 0.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PressedProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PressedProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PressedProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PressedProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PressedProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PressedProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PressedProcessor::getCurrentProgram()
{
    return 0;
}

void PressedProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PressedProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PressedProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PressedProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PressedProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PressedProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

//=============================envelope smoothing===============================
float PressedProcessor::attackSmoothing(float target, float current, float attackMs)
{
    float coeff = std::exp(-1.0f / (getSampleRate() * attackMs * 0.001f));
    return coeff * current + (1.0f - coeff) * target;
}

float PressedProcessor::releaseSmoothing(float target, float current, float releaseMs)
{
    float coeff = std::exp(-1.0f / (getSampleRate() * releaseMs * 0.001f));
    return coeff * current + (1.0f - coeff) * target;
}
//==============================================================================

void PressedProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    const float threshold = *thresholdParam;
    const float ratio = *ratioParam;
    const float attack = *attackParam;
    const float release = *releaseParam;
    const float makeup = *makeupGainParam;

    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();


    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        float env = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            float input = channelData[i];
            float db = juce::Decibels::gainToDecibels(std::abs(input) + 1e-6f);
            float overThreshold = db - threshold;

            float gainReduction = 0.0f;
            if (overThreshold > 0.0f)
            {
                float compressedDB = threshold + (overThreshold / ratio);
                float gainDB = compressedDB - db;
                gainReduction = juce::Decibels::decibelsToGain(gainDB);
            }
            else
            {
                gainReduction = 1.0f;
            }

            // smooth the gain using an attack/release envelope follower
            env = (gainReduction < env)
                ? attackSmoothing(gainReduction, env, attack)
                : releaseSmoothing(gainReduction, env, release);

            float makeupGain = juce::Decibels::decibelsToGain(makeup);
            channelData[i] = input * env * makeupGain;
        }
    }
}

//==============================================================================
bool PressedProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PressedProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PressedProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PressedProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PressedProcessor();
}
