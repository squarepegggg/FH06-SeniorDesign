#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"

extern "C" int ei2_classify_array(const float *input_array, size_t array_size,
                                   const char **out_label, float *out_score)
{
    // Check that the input array size matches the model's expected input size
    if (array_size != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        return -99; // model mismatch - wrong input size
    }

    // Wrap input array in a signal
    signal_t signal;
    if (numpy::signal_from_buffer(input_array,
                                  EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
                                  &signal) != 0) {
        return -98;
    }

    ei_impulse_result_t result = {};
    // Run the classifier
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

