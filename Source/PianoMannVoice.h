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
  constexpr static int kMaxNote = 84;

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
    tailOff = 0.f;
    exciteBuffer();
  }

  void stopNote(float velocity, bool allowTailOff) override {
    ignoreUnused(velocity);
    if (allowTailOff) {
      if (tailOff == 0.f) {
        tailOff = 1.0f;
      }
    } else {
      clearCurrentNote();
    }
    isNoteHeld = false;
  }

  static constexpr float getWeightedAverageFilterForNote(int midiNoteNumber) {
    return 0.7f;
  }

  struct DecaySpec {
    float sustain;
    float release;
  };
  static constexpr DecaySpec getDecayForNote(int midiNoteNumber) {
    return DecaySpec{0.998f, 0.992f};
  }

  void renderNextBlock(AudioBuffer<float> &outputBuffer, int startSample,
                       int numSamples) override {
    const static auto weightedAverageFilterFactor =
        getWeightedAverageFilterForNote(params.midiNoteNumber);
    const static auto decaySpec = getDecayForNote(params.midiNoteNumber);

    constexpr auto kDecayPowerLevelThreshold = 0.005f;

    if (!isNoteHeld && tailOff == 0.f) {
      // Not playing note nor releasing it slowly
      return;
    }

    if (tailOff > 0.f) {
      // tailOff > 0.f implies we are releasing this note slowly.
      tailOff *= decaySpec.release;
      if (tailOff < kDecayPowerLevelThreshold) {
        tailOff = 0.f;
        clearCurrentNote();
        return;
      }
    }

    for (auto sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
      const auto nextBufferPosition = (1 + currentBufferPosition) %
                                      static_cast<int>(delayLineBuffer.size());

      const auto weightedNextDelaySample =
          weightedAverageFilterFactor * delayLineBuffer[nextBufferPosition];
      const auto weightCurrentDelaySample =
          (1 - weightedAverageFilterFactor) *
          delayLineBuffer[currentBufferPosition];

      auto decay = decaySpec.sustain;
      if (tailOff > 0.f) {
        decay *= tailOff;
      }
      delayLineBuffer[nextBufferPosition] =
          decay * (weightedNextDelaySample + weightCurrentDelaySample);

      const auto currentSample = delayLineBuffer[currentBufferPosition];
      for (auto channel = outputBuffer.getNumChannels(); --channel >= 0;) {
        auto *output = outputBuffer.getWritePointer(channel, startSample);
        output[sampleIndex] += currentSample;
      }

      currentBufferPosition = nextBufferPosition;
    }
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
  float tailOff = 0.f;
};
