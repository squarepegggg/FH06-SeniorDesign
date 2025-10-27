#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

int main() {
    float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 2, 120, 240 };
    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

    ei_impulse_result_t result;
    if (run_classifier(&signal, &result, false) != EI_IMPULSE_OK) return 1;

    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("%s: ", ei_classifier_inferencing_categories[i]);
        ei_printf_float(result.classification[i].value);
        ei_printf("\n");
    }
    return 0;
}