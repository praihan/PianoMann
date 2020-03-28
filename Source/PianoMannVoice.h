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

struct PianoMannVoiceParams
{
  int midiNoteNumber;
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
    prepareExcitationBuffers();
  }

  void startNote(int midiNoteNumber, float velocity, SynthesiserSound *,
                 int /*currentPitchWheelPosition*/) override {
    jassert(midiNoteNumber == params.midiNoteNumber);
    jassert(areBuffersReady);
    currentVelocity = velocity;
    exciteBuffer();
  }

  void stopNote(float /*velocity*/, bool allowTailOff) override {
    /*
    if (allowTailOff) {
      // start a tail-off by setting this flag. The render callback will pick up
      // on this and do a fade out, calling clearCurrentNote() when it's
      // finished.

      if (tailOff == 0.0) // we only need to begin a tail-off if it's not
                          // already doing so - the
        tailOff = 1.0;    // stopNote method could be called more than once.
    } else {
      // we're being told to stop playing immediately, so reset everything..
      clearCurrentNote();
      angleDelta = 0.0;
    }
    */
    ignoreUnused(allowTailOff);
    clearCurrentNote();
  }

  void pitchWheelMoved(int /*newValue*/) override {}
  void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {}

  void renderNextBlock(AudioBuffer<float> &outputBuffer, int startSample,
                       int numSamples) override {
    constexpr auto kDecay = 0.998f;

    for (auto sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
      const auto nextBufferPosition = (1 + currentBufferPosition) %
                                      static_cast<int>(delayLineBuffer.size());

      delayLineBuffer[nextBufferPosition] =
          (kDecay * 0.5f *
           (delayLineBuffer[nextBufferPosition] +
            delayLineBuffer[currentBufferPosition]));

      const auto currentSample = delayLineBuffer[currentBufferPosition];
      for (auto outputChannel = outputBuffer.getNumChannels();
           --outputChannel >= 0;) {
        auto *output = outputBuffer.getWritePointer(outputChannel, startSample);
        output[sampleIndex] += currentSample;
      }
      currentBufferPosition = nextBufferPosition;
    }
  }

  using SynthesiserVoice::renderNextBlock;

private:
  void prepareExcitationBuffers() {
    const auto sampleRate = getSampleRate();
    if (sampleRate == 0.0) {
      areBuffersReady = false;
      return;
    }
    const auto frequencyInHz = MidiMessage::getMidiNoteInHertz(params.midiNoteNumber);
    const auto excitationNumSamples = roundToInt(sampleRate / frequencyInHz);

    delayLineBuffer.resize(excitationNumSamples);
    std::fill(delayLineBuffer.begin(), delayLineBuffer.end(), 0.f);

    excitationBuffer.resize(excitationNumSamples);
    std::generate(excitationBuffer.begin(), excitationBuffer.end(), [] {
      return (Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f;
    });

    currentBufferPosition = 0;
    areBuffersReady = true;
  }

  void exciteBuffer() {
    jassert(delayLineBuffer.size() >= excitationBuffer.size());

    std::transform(excitationBuffer.begin(), excitationBuffer.end(),
                   delayLineBuffer.begin(),
                   [this](float sample) { return currentVelocity * sample; });
  }

  const PianoMannVoiceParams params;
  float currentVelocity = 0.f;

  bool areBuffersReady = false;
  std::vector<float> excitationBuffer, delayLineBuffer;
  int currentBufferPosition = 0;
};
