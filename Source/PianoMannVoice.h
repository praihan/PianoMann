/*
  ==============================================================================

    PianoMannVoice.h
    Created: 28 Mar 2020 1:52:01am
    Author:  Pranjal Raihan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct PianoMannSound : public SynthesiserSound {
  constexpr static int kMinNote = 21;
  constexpr static int kMaxNote = 108;

  PianoMannSound(int midiNoteNumber) : midiNoteNumber(midiNoteNumber) {
    jassert(midiNoteNumber >= kMinNote && midiNoteNumber <= kMaxNote);
  }

  bool appliesToNote(int midiNoteNumber) override {
    return midiNoteNumber == this->midiNoteNumber;
  }
  bool appliesToChannel(int midiChannel) override {
    ignoreUnused(midiChannel);
    return true;
  }

private:
  int midiNoteNumber;
};

/**
 * A synth voice that plays one specific note only.
 * This is because each note is modeled differently, albeit with similar
 * techniques.
 */
struct PianoMannVoice : public SynthesiserVoice {
  PianoMannVoice(int midiNoteNumber) : midiNoteNumber(midiNoteNumber) {}

  bool canPlaySound(SynthesiserSound *sound) override {
    if (auto *pianoMannSound = dynamic_cast<PianoMannSound *>(sound)) {
      return pianoMannSound->appliesToNote(midiNoteNumber);
    }
    return false;
  }

  void startNote(int midiNoteNumber, float velocity, SynthesiserSound *,
                 int /*currentPitchWheelPosition*/) override {
    currentAngle = 0.0;
    level = velocity * 0.15;
    tailOff = 0.0;

    auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();

    angleDelta = cyclesPerSample * MathConstants<double>::twoPi;
  }

  void stopNote(float /*velocity*/, bool allowTailOff) override {
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
  }

  void pitchWheelMoved(int /*newValue*/) override {}
  void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {}

  void renderNextBlock(AudioBuffer<float> &outputBuffer, int startSample,
                       int numSamples) override {
    if (angleDelta != 0.0) {
      if (tailOff > 0.0) {
        while (--numSamples >= 0) {
          auto currentSample =
              (float)(std::sin(currentAngle) * level * tailOff);

          for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

          currentAngle += angleDelta;
          ++startSample;

          tailOff *= 0.99;

          if (tailOff <= 0.005) {
            clearCurrentNote();

            angleDelta = 0.0;
            break;
          }
        }
      } else {
        while (--numSamples >= 0) {
          auto currentSample = (float)(std::sin(currentAngle) * level);

          for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

          currentAngle += angleDelta;
          ++startSample;
        }
      }
    }
  }

  using SynthesiserVoice::renderNextBlock;

private:
  int midiNoteNumber;
  double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
};
