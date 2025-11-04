#ifndef EI_CLASSIFIER_WRAPPER_H
#define EI_CLASSIFIER_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
typedef enum {
    EI_WRAPPER_OK = 0,
    EI_WRAPPER_ERROR = -1
} ei_wrapper_error_t;

// Classification result structure for C
typedef struct {
    float single_value;
    float double_value;
    float long_value;
    const char* predicted_label;
    float confidence;
} ei_classification_result_t;

/**
 * Run classification on feature array
 * @param features Array of 3 floats: [press_count, avg_interval_ms, total_duration_ms]
 * @param result Output classification result
 * @return 0 on success, negative on error
 */
int ei_run_classification(float features[3], ei_classification_result_t *result);

#ifdef __cplusplus
}
#endif

#endif // EI_CLASSIFIER_WRAPPER_H

