/*
  ==============================================================================

  This file was auto-generated!

  It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PianoMannVoice.h"
#include "PluginEditor.h"

//==============================================================================
PianoMannAudioProcessor::PianoMannAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
  initializeSynth();
}

PianoMannAudioProcessor::~PianoMannAudioProcessor() = default;

void PianoMannAudioProcessor::initializeSynth() {
  synth.clearVoices();
  synth.clearSounds();
  for (auto midiNote = PianoMannSound::kMinNote;
       midiNote <= PianoMannSound::kMaxNote; ++midiNote) {
    synth.addVoice(new PianoMannVoice({midiNote}));
    synth.addSound(new PianoMannSound(midiNote));
  }
}

//==============================================================================
// ReSharper disable once CppConstValueFunctionReturnType
const String PianoMannAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool PianoMannAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PianoMannAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PianoMannAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PianoMannAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int PianoMannAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int PianoMannAudioProcessor::getCurrentProgram() { return 0; }

void PianoMannAudioProcessor::setCurrentProgram(int index) {
  ignoreUnused(index);
}

// ReSharper disable once CppConstValueFunctionReturnType
const String PianoMannAudioProcessor::getProgramName(int index) {
  ignoreUnused(index);
  return {};
}

void PianoMannAudioProcessor::changeProgramName(int index,
                                                const String &newName) {
  ignoreUnused(index, newName);
}

//==============================================================================
void PianoMannAudioProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  ignoreUnused(samplesPerBlock);
  synth.setCurrentPlaybackSampleRate(sampleRate);
  keyboardState.reset();
}

void PianoMannAudioProcessor::releaseResources() { keyboardState.reset(); }

#ifndef JucePlugin_PreferredChannelConfigurations
bool PianoMannAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void PianoMannAudioProcessor::processBlock(AudioBuffer<float> &buffer,
                                           MidiBuffer &midiMessages) {
  ScopedNoDenormals noDenormals;
  ignoreUnused(noDenormals);
  const auto totalNumInputChannels = getTotalNumInputChannels();
  const auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, buffer.getNumSamples());
  }

  const auto numSamples = buffer.getNumSamples();
  keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
  synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
}

//==============================================================================
bool PianoMannAudioProcessor::hasEditor() const {
  return true;
}

AudioProcessorEditor *PianoMannAudioProcessor::createEditor() {
  return new PianoMannAudioProcessorEditor(*this);
}

//==============================================================================
void PianoMannAudioProcessor::getStateInformation(MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  ignoreUnused(destData);
}

void PianoMannAudioProcessor::setStateInformation(const void *data,
                                                  int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new PianoMannAudioProcessor();
}
