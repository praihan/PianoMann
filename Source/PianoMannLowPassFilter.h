/*
  ==============================================================================

    PianoMannLowPassFilter.h
    Created: 28 Mar 2020 7:24:27am
    Author:  Pranjal Raihan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

template <int kCutoffFrequency>
class PianoMannLowPassFilter : dsp::ProcessorBase {
  using IIRFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>,
                                             dsp::IIR::Coefficients<float>>;
  std::vector<IIRFilter> filters;

public:
  void prepare(const dsp::ProcessSpec &spec) override {
    auto coefficientsArrays =
        dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
            kCutoffFrequency, spec.sampleRate, 17);
    filters.clear();
    filters.resize(coefficientsArrays.size());

    auto filterIterator = filters.begin();
    for (auto &coefficients : coefficientsArrays) {
      filterIterator->state = coefficients;
      filterIterator->prepare(spec);
      ++filterIterator;
    }
  }

  void process(const dsp::ProcessContextReplacing<float> &context) override {
    for (auto &filter : filters) {
      filter.process(context);
    }
  }

  void reset() override {
    for (auto &filter : filters) {
      filter.reset();
    }
  }
};
