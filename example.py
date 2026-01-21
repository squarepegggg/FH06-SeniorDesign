import tensorflow as tf
import numpy as np

print("TensorFlow:", tf.__version__)

NUM_CLASSES = 6  # <-- set this

def build_1dcnn_classifier(num_classes: int):
    inputs = tf.keras.Input(shape=(3, 25, 1), name="input_3x25x1")

    # Conv1D expects (length, channels). Treat 3x25 as a length-75 sequence.
    x = tf.keras.layers.Reshape((75, 1), name="reshape_to_75x1")(inputs)

    x = tf.keras.layers.Conv1D(32, 5, padding="same", activation="relu")(x)
    x = tf.keras.layers.MaxPool1D(2)(x)

    x = tf.keras.layers.Conv1D(64, 3, padding="same", activation="relu")(x)
    x = tf.keras.layers.MaxPool1D(2)(x)

    x = tf.keras.layers.Conv1D(64, 3, padding="same", activation="relu")(x)
    x = tf.keras.layers.MaxPool1D(2)(x)

    x = tf.keras.layers.Conv1D(64, 3, padding="same", activation="relu")(x)
    x = tf.keras.layers.GlobalAveragePooling1D()(x)

    x = tf.keras.layers.Dense(64, activation="relu")(x)
    x = tf.keras.layers.Dropout(0.2)(x)

    outputs = tf.keras.layers.Dense(num_classes, activation="softmax", name="probs")(x)
    return tf.keras.Model(inputs, outputs)

model = build_1dcnn_classifier(NUM_CLASSES)
model.compile(optimizer="adam",
              loss="sparse_categorical_crossentropy",
              metrics=["accuracy"])

# ---- Train (replace with your real data) ----
X = np.random.randn(200, 3, 25, 1).astype(np.float32)
y = np.random.randint(0, NUM_CLASSES, size=(200,), dtype=np.int32)
model.fit(X, y, epochs=3, batch_size=32)


# ---- Representative dataset for int8 quantization ----
def representative_dataset():
    # IMPORTANT: Use real samples from your training/validation set here.
    for i in range(100):
        # yield a list of input tensors matching model inputs
        sample = X[i:i+1]  # shape (1,3,25,1), float32
        yield [sample]

# ---- Convert to fully-int8 TFLite (TFLite Micro friendly) ----
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset

# Force built-in INT8 kernels only (best for Edge Impulse / TFLite Micro)
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]

# Full integer I/O (often preferred for MCUs)
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

tflite_path = "classifier_3x25x1_1dcnn_int8.tflite"
with open(tflite_path, "wb") as f:
    f.write(tflite_model)

print("Saved:", tflite_path)

# ---- Inspect ops (helpful if EI still complains) ----
tf.lite.experimental.Analyzer.analyze(model_content=tflite_model)
print(model.count_params())