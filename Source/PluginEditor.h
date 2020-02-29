/*
  ==============================================================================

  This file was auto-generated!

  It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

//==============================================================================
/**
 */
class PianoMannAudioProcessorEditor : public AudioProcessorEditor {
public:
  explicit PianoMannAudioProcessorEditor(PianoMannAudioProcessor &);
  ~PianoMannAudioProcessorEditor();

  //==============================================================================
  void paint(Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PianoMannAudioProcessor &processor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoMannAudioProcessorEditor)
};
