/*
  ==============================================================================

    PianoMannLowPassFilter.h
    Created: 28 Mar 2020 7:24:27am
    Author:  Pranjal Raihan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template <int kCutoffFrequency>
class PianoMannLowPassFilter : dsp::ProcessorBase {
  using IIRFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>,
                                             dsp::IIR::Coefficients<float>>;
  IIRFilter filter;

public:
  void prepare(const dsp::ProcessSpec &spec) override {
    filter.state = dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate,
                                                              kCutoffFrequency);
    filter.prepare(spec);
  }

  void process(const dsp::ProcessContextReplacing<float> &context) override {
    filter.process(context);
  }

  void reset() override { filter.reset(); }
};
