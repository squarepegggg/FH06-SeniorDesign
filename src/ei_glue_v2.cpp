/**
 * ei_glue_v2.cpp - C-callable wrapper for the ei-v2 Edge Impulse model
 * 
 * This file uses the ei-v2 model which expects 75 input features (3 x 25 x 1).
 * For testing, we pass dummy data to verify the model is correctly integrated.
 */

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"

extern "C" int ei_v2_classify_test(const char **out_label, float *out_score)
{
    // The ei-v2 model expects EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE features
    // (should be 75 for 3 x 25 x 1)
    static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
    
    // Fill with some dummy test data
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
        features[i] = (float)(i % 10) / 10.0f;  // Values 0.0 to 0.9 repeating
    }

    // Wrap features in a signal
    signal_t signal;
    if (numpy::signal_from_buffer(features,
                                  EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
                                  &signal) != 0) {
        return -98;
    }

    ei_impulse_result_t result = {};
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, /*debug*/ false);
    if (err != EI_IMPULSE_OK) {
        return static_cast<int>(err);
    }

    // Find the top class
    size_t best_i = 0;
    float  best_v = result.classification[0].value;
    for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > best_v) {
            best_v = result.classification[i].value;
            best_i = i;
        }
    }

    if (out_label) *out_label = result.classification[best_i].label;
    if (out_score) *out_score = best_v;
    return 0;
}
