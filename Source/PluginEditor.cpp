/*
  ==============================================================================

	This file was auto-generated!

	It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PianoMannAudioProcessorEditor::PianoMannAudioProcessorEditor(PianoMannAudioProcessor& p)
	: AudioProcessorEditor(&p), processor(p)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(400, 300);
}

PianoMannAudioProcessorEditor::~PianoMannAudioProcessorEditor()
{
}

//==============================================================================
void PianoMannAudioProcessorEditor::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid color)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::white);
	g.setFont(15.0f);
	g.drawFittedText("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void PianoMannAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// sub-components in your editor..
}
