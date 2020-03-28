/*
  ==============================================================================

    PianoMannVoice.h
    Created: 28 Mar 2020 1:52:01am
    Author:  Pranjal Raihan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <vector>

struct PianoMannSound : public SynthesiserSound {
  constexpr static int kMinNote = 21;
  constexpr static int kMaxNote = 108;

  PianoMannSound(int _midiNoteNumber) : midiNoteNumber(_midiNoteNumber) {
    jassert(midiNoteNumber >= kMinNote && midiNoteNumber <= kMaxNote);
  }

  bool appliesToNote(int _midiNoteNumber) override {
    return _midiNoteNumber == this->midiNoteNumber;
  }
  bool appliesToChannel(int midiChannel) override {
    ignoreUnused(midiChannel);
    return true;
  }

private:
  int midiNoteNumber;
};

struct PianoMannVoiceParams {
  int midiNoteNumber;
  dsp::ProcessorBase& postProcessor;
};

/**
 * A synth voice that plays one specific note only.
 * This is because each note is modeled differently, albeit with similar
 * techniques.
 */
struct PianoMannVoice : public SynthesiserVoice {
  PianoMannVoice(PianoMannVoiceParams params) : params(params) {}

  bool canPlaySound(SynthesiserSound *sound) override {
    if (auto *pianoMannSound = dynamic_cast<PianoMannSound *>(sound)) {
      return pianoMannSound->appliesToNote(params.midiNoteNumber);
    }
    return false;
  }

  void setCurrentPlaybackSampleRate(double newRate) override {
    SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
    if (newRate != 0.0) {
      prepareExcitationBuffers();
    }
  }

  void startNote(int midiNoteNumber, float velocity, SynthesiserSound *,
                 int currentPitchWheelPosition) override {
    ignoreUnused(currentPitchWheelPosition);
    jassert(midiNoteNumber == params.midiNoteNumber);
    jassert(isExcitationBufferReady);
    currentVelocity = velocity;
    isNoteHeld = true;
    exciteBuffer();
  }

  void stopNote(float velocity, bool allowTailOff) override {
    ignoreUnused(velocity, allowTailOff);
    clearCurrentNote();
    isNoteHeld = false;
  }

  void renderNextBlock(AudioBuffer<float> &outputBuffer, int startSample,
                       int numSamples) override {
    constexpr auto kDecay = 0.998f;

    if (!isNoteHeld) {
      // TODO: add sustain
      return;
    }

    for (auto sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
      const auto nextBufferPosition = (1 + currentBufferPosition) %
                                      static_cast<int>(delayLineBuffer.size());

      delayLineBuffer[nextBufferPosition] =
          (kDecay * 0.5f *
           (delayLineBuffer[nextBufferPosition] +
            delayLineBuffer[currentBufferPosition]));

      const auto currentSample = delayLineBuffer[currentBufferPosition];
      for (auto channel = outputBuffer.getNumChannels(); --channel >= 0;) {
        auto *output = outputBuffer.getWritePointer(channel, startSample);
        output[sampleIndex] += currentSample;
      }

      currentBufferPosition = nextBufferPosition;
    }

    dsp::AudioBlock<float> block(outputBuffer);
    const dsp::ProcessContextReplacing<float> processingContext(block);
    params.postProcessor.process(processingContext);
  }

  using SynthesiserVoice::renderNextBlock;

  void pitchWheelMoved(int newValue) override { ignoreUnused(newValue); }
  void controllerMoved(int controllerNumber, int newValue) override {
    ignoreUnused(controllerNumber, newValue);
  }

private:
  void prepareExcitationBuffers() {
    const auto sampleRate = getSampleRate();
    jassert(sampleRate != 0.0);

    const auto frequencyInHz =
        MidiMessage::getMidiNoteInHertz(params.midiNoteNumber);
    const auto excitationNumSamples = roundToInt(sampleRate / frequencyInHz);

    delayLineBuffer.resize(excitationNumSamples);
    std::fill(delayLineBuffer.begin(), delayLineBuffer.end(), 0.f);

    excitationBuffer.resize(excitationNumSamples);
    std::generate(excitationBuffer.begin(), excitationBuffer.end(), [] {
      return (Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f;
    });

    currentBufferPosition = 0;
    isExcitationBufferReady = true;
  }

  void exciteBuffer() {
    jassert(delayLineBuffer.size() >= excitationBuffer.size());

    std::transform(excitationBuffer.begin(), excitationBuffer.end(),
                   delayLineBuffer.begin(),
                   [this](float sample) { return currentVelocity * sample; });
  }

  const PianoMannVoiceParams params;
  float currentVelocity = 0.f;

  bool isExcitationBufferReady = false;
  std::vector<float> excitationBuffer, delayLineBuffer;
  int currentBufferPosition = 0;

  bool isNoteHeld = false;
};
