import tensorflow as tf


def limit_gpu_memory(gpu_index=0, memory_limit=1024):
    gpus = tf.config.experimental.list_physical_devices('GPU')
    if gpus:
        # Restrict TensorFlow to only allocate memory_limit (MB) of memory on the first GPU
        try:
            tf.config.experimental.set_virtual_device_configuration(
                gpus[gpu_index],
                [tf.config.experimental.VirtualDeviceConfiguration(memory_limit=memory_limit)])
            logical_gpus = tf.config.experimental.list_logical_devices('GPU')
        except RuntimeError as e:
            # Virtual devices must be set before GPUs have been initialized
            print(e)