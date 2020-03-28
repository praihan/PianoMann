/*
  ==============================================================================

  This file was auto-generated!

  It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
PianoMannAudioProcessorEditor::PianoMannAudioProcessorEditor(
    PianoMannAudioProcessor &p)
    : AudioProcessorEditor(&p), processor(p),
      midiKeyboardComponent(p.keyboardState,
                            MidiKeyboardComponent::horizontalKeyboard) {
  setOpaque(true);
  setSize(640, 480);
  addAndMakeVisible(midiKeyboardComponent);
}

PianoMannAudioProcessorEditor::~PianoMannAudioProcessorEditor() = default;

void PianoMannAudioProcessorEditor::paint(Graphics &g) {
  // Since this component is opaque, we must fill the entire viewport
  g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void PianoMannAudioProcessorEditor::resized() {
  // TODO: set sizes
  midiKeyboardComponent.setBounds(8, 96, getWidth() - 16, 64);
  // sineButton.setBounds(16, 176, 150, 24);
  // sampledButton.setBounds(16, 200, 150, 24);
  // liveAudioDisplayComp.setBounds(8, 8, getWidth() - 16, 64);
}
