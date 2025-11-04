#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"

extern "C" int ei_classify_three(float presses, float avg_ms, float window_ms,
                                 const char **out_label, float *out_score)
{
    // Build feature vector in the SAME order you trained on
    // (button_presses, avg_response_ms, window_ms)
    static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

    if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE < 3) {
        return -99; // model mismatch
    }
    features[0] = presses;
    features[1] = avg_ms;
    features[2] = window_ms;

    // Wrap features in a signal
    signal_t signal;
    if (numpy::signal_from_buffer(features,
                                  EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
                                  &signal) != 0) {
        return -98;
    }

    ei_impulse_result_t result = {};
    // NOTE: the correct API name is run_classifier(...)
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
