/**
 * ei_glue_v2.cpp - C-callable wrapper for the ei-v2 Edge Impulse model
 * 
 * This file uses the ei-v2 model which expects 75 input features.
 * Input shape: 25 rows × 3 columns, flattened row-major to 75 values.
 * For testing, we use real demo data from the training set.
 */

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"

// Demo data: 25 rows × 3 columns (flattened row-major order)
// Each row has 3 values, total 75 values
static const float demo_data[75] = {
    // Row 1-5
    0.149014f, -0.04148f, 0.194307f,
    0.456909f, -0.07025f, -0.07024f,
    0.473764f, 0.23023f, -0.14084f,
    0.162768f, -0.13903f, -0.13972f,
    0.072589f, -0.57398f, -0.51748f,
    // Row 6-10
    -0.16869f, -0.30385f, 0.094274f,
    -0.27241f, -0.42369f, 0.439695f,
    -0.06773f, 0.020258f, -0.42742f,
    -0.16331f, 0.033277f, -0.3453f,
    0.112709f, -0.18019f, -0.08751f,
    // Row 11-15
    -0.18051f, 0.555683f, -0.00405f,
    -0.31731f, 0.246763f, -0.36625f,
    0.062659f, -0.5879f, -0.39846f,
    0.059058f, 0.22154f, 0.05141f,
    -0.03469f, -0.09033f, -0.44356f,
    // Row 16-20
    -0.21595f, -0.13819f, 0.317137f,
    0.103085f, -0.52891f, 0.097225f,
    -0.11552f, -0.20308f, 0.183503f,
    0.3093f, 0.279384f, -0.25177f,
    -0.09276f, 0.099379f, 0.292664f,
    // Row 21-25
    -0.14375f, -0.0557f, -0.3319f,
    -0.35886f, 0.243758f, 0.406872f,
    -0.0216f, 0.30106f, 0.108491f,
    -0.19354f, 0.108419f, 0.461411f,
    -0.01075f, 0.469393f, -0.78592f
};

extern "C" int ei_v2_classify_test(const char **out_label, float *out_score)
{
    // Copy demo data to mutable buffer (required by signal_from_buffer)
    static float features[75];
    for (int i = 0; i < 75; i++) {
        features[i] = demo_data[i];
    }

    // Wrap features in a signal
    signal_t signal;
    if (numpy::signal_from_buffer(features, 75, &signal) != 0) {
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
