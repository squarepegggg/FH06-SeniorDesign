#include "ei_classifier_wrapper.h"
#include <string.h>

// C++ includes for Edge Impulse
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

extern "C" {

int ei_run_classification(float features[3], ei_classification_result_t *result)
{
    if (result == NULL) {
        return EI_WRAPPER_ERROR;
    }

    // Initialize result structure
    result->single_value = 0.0f;
    result->double_value = 0.0f;
    result->long_value = 0.0f;
    result->predicted_label = "unknown";
    result->confidence = 0.0f;

    // Create signal from buffer
    signal_t signal;
    int err = numpy::signal_from_buffer(features, 3, &signal);
    if (err != 0) {
        return EI_WRAPPER_ERROR;
    }

    // Run classification
    ei_impulse_result_t ei_result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &ei_result, false);

    if (res != EI_IMPULSE_OK) {
        return EI_WRAPPER_ERROR;
    }

    // Extract results - assuming the labels are in order: single, double, long
    // We need to match by label name since order might vary
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        const char* label = ei_result.classification[ix].label;
        float value = ei_result.classification[ix].value;

        if (strcmp(label, "single") == 0) {
            result->single_value = value;
        } else if (strcmp(label, "double") == 0) {
            result->double_value = value;
        } else if (strcmp(label, "long") == 0) {
            result->long_value = value;
        }

        // Track highest confidence
        if (value > result->confidence) {
            result->confidence = value;
            result->predicted_label = label;
        }
    }

    return EI_WRAPPER_OK;
}

} // extern "C"

