import config
import experiment
import models_mnist as models

# initialize experiment:
# - single-class classification problem using the MNIST dataset
# - the model should predict the digit of a grayscale image
cfg = config.config()
exp = experiment.experiment(
        cfg.task_mnist(),
        cfg.expdir + "/mnist/eval_minibatch/")

# loss functions
exp.add_loss("slogistic")

# iterators
exp.add_iterator("default")

# trainers
stoch_params = "epochs=20,patience=32,epsilon=1e-6,batch={}"
minibatch_name = "minibatch{}"

for size in [32, 64, 128, 256, 512, 1024]:
        exp.add_trainer("stoch_adadelta", stoch_params.format(size), minibatch_name.format(size))

# models
exp.add_model("convnet8", models.convnet8 + models.outlayer)

# train all configurations
trials = 10
exp.run_all(trials = trials)

# compare trainers
for mname, iname, lname in [(x, y, z) for x in exp.models for y in exp.iterators for z in exp.losses]:
        for trial in range(trials):
                exp.plot_many(
                        exp.filter(trial, mname, ".*", iname, lname, ".state"),
                        exp.path(trial, mname, None, iname, lname, ".pdf"))

        exp.summarize(trials, mname, ".*", iname, lname,
                exp.path(None, mname, None, iname, lname, ".log"),
                exp.path(None, mname, None, iname, lname, ".csv"))