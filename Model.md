# Model 

For any other dataset than the one created using the DatasetBuilder you can follow this [tutorial](https://github.com/arthurflor23/handwritten-text-recognition/blob/master/src/tutorial.ipynb) 

### Dataset transformation

Once the image transcription is finished and that you have enough data create a `raw` folder in the `py` folder and a `civil` folder in the `raw` folder. Once all the folders are 
created, put all the folders created by the DatasetBuilder and the `transcription.json` file in the `civil` folder. Then you can run the `transform.py` that will create the HDF5 file 
that will be read for the training. You will find this file in the `data` folder that has been created.

### Model training

To train the model on the dataset you've just created you just need to run the `training.py` file.

If when training your model on the dataset created thanks to our dataset builder you have an error such as `No valid path found` for the loss calculator, try to go to the definition of the 
function `ctc_batch_cost()` used line 261 in the `ctc_loss_lambda_func()` function in the `src/network/model.py`. Once you're in the function `ctc_batch_cost()` add the parameter `ignore_longer_outputs_than_inputs=True`
to the `ctc.ctc_loss()` function that should be on lines 5763-5764.

### Model testing

Once the training is finished you can run tests to see if the training you have done is efficient. To do so you just need to run the `test.py` file. If you want to replace the
model used in the CivilRegistryAnalyser you need to replace the `checkpoint_weights.hdf5` file in the `py` folder by the one that has been created in `output/civil/flor`. 
