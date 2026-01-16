#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

extern "C" int ei_classify_csv(const char *csv_file_path);

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

extern "C" int ei_classify_csv(const char *csv_file_path) {
    std::ifstream file(csv_file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << csv_file_path << std::endl;
        return -1; // Cannot open file
    }

    std::string line;
    // Skip header
    if (!std::getline(file, line)) {
        return -2; // File is empty or unreadable
    }

    std::vector<float> x_vals, y_vals, z_vals;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        // timestamp,x,y,z,label
        std::getline(ss, value, ','); // timestamp
        std::getline(ss, value, ','); // x
        x_vals.push_back(std::stof(value));
        std::getline(ss, value, ','); // y
        y_vals.push_back(std::stof(value));
        std::getline(ss, value, ','); // z
        z_vals.push_back(std::stof(value));
    }

    const int window_size = 25;
    const int num_axes = 3;
    if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE != window_size * num_axes) {
        std::cerr << "Error: Mismatch between CSV data shape and model input size." << std::endl;
        return -3; // Model input size mismatch
    }

    size_t num_samples = x_vals.size();
    size_t num_windows = num_samples / window_size;

    for (size_t i = 0; i < num_windows; ++i) {
        static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
        size_t feature_ix = 0;

        // The model expects a flat array of accelerometer data, window by window.
        // The input shape is (3, 25, 1), so we flatten it as [x1..x25, y1..y25, z1..z25]
        size_t start_index = i * window_size;
        for(int j = 0; j < window_size; ++j) features[feature_ix++] = x_vals[start_index + j];
        for(int j = 0; j < window_size; ++j) features[feature_ix++] = y_vals[start_index + j];
        for(int j = 0; j < window_size; ++j) features[feature_ix++] = z_vals[start_index + j];

        signal_t signal;
        if (numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal) != 0) {
            std::cerr << "Window " << i << ": Failed to create signal from buffer." << std::endl;
            continue;
        }

        ei_impulse_result_t result = {};
        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, /*debug*/ false);
        if (err != EI_IMPULSE_OK) {
            std::cerr << "Window " << i << ": Classifier failed with error " << err << std::endl;
            continue;
        }

        // Find and print the top class
        size_t best_i = 0;
        float best_v = 0;
        for (size_t k = 0; k < EI_CLASSIFIER_LABEL_COUNT; k++) {
            if (result.classification[k].value > best_v) {
                best_v = result.classification[k].value;
                best_i = k;
            }
        }
        std::cout << "Window " << i << ": Predicted: " << result.classification[best_i].label
                  << " (" << best_v << ")" << std::endl;
    }

    return 0;
}
