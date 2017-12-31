import os
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

""" load (training) §state file with the following format:
(epoch, {train, valid, test} x {criterion, loss{average, variance, maximum}, error{average, variance, maximum}}, time)+
"""
def get_state_csv(path):
        name = os.path.basename(path).replace(".state", "")
        name = name.replace(name[0 : name.find("_") + 1], "")
        data = mlab.csv2rec(path, delimiter = ' ', names = None)
        return name, data

""" load multiple state files """
def get_state_csvs(paths):
        datas = []
        names = []
        for path in paths:
                name, data = get_state_csv(path)
                datas.append(data)
                names.append(name)
        return names, datas

""" load trial file with the following format:
(model name, trainer name, enhancer name, loss name, test value, test error, #epochs, convergence speed, training time)+
"""
def get_trial_csv(path):
        name = os.path.basename(path).replace(".csv", "")
        name = name.replace(name[0 : name.find("_") + 1], "")
        data = mlab.csv2rec(path, delimiter = ';', names = None)
        return name, data

""" load multiple trial fiels """
def get_trial_csvs(paths):
        datas = []
        names = []
        for path in paths:
                name, data = get_trial_csv(path)
                datas.append(data)
                names.append(name)
        return names, datas

""" plot the training evolution of a model """
def plot_state_one(spath, ppath):
        title, data = get_state_csv(spath)
        with PdfPages(ppath) as pdf:
                # (train, validation, test) loss value and error
                for col in (0, 1):
                        # x axis - epoch/iteration index
                        xname = data.dtype.names[0]
                        xlabel = xname
                        # y axis - train/validation/test datasets
                        yname0 = data.dtype.names[col + 1]
                        yname1 = data.dtype.names[col + 3]
                        yname2 = data.dtype.names[col + 5]
                        ylabel = yname0.replace("train_", "")
                        # plot
                        plt.xlabel(xlabel, fontsize = "smaller")
                        plt.ylabel(ylabel, fontsize = "smaller")
                        plt.title(title, weight = "bold")
                        plt.plot(data[xname], data[yname0], "r-", label = yname0)
                        plt.plot(data[xname], data[yname1], "g-", label = yname1)
                        plt.plot(data[xname], data[yname2], "b-", label = yname2)
                        plt.legend(fontsize = "smaller")
                        plt.grid(True, linestyle='--')
                        pdf.savefig()
                        plt.close()
                # xnorm and gnorm
                for col in (7, 8):
                        # x axis - epoch/iteration index
                        xname = data.dtype.names[0]
                        xlabel = xname
                        # y axis
                        yname = data.dtype.names[col + 1]
                        ylabel = yname
                        # plot
                        plt.xlabel(xlabel, fontsize = "smaller")
                        plt.ylabel(ylabel, fontsize = "smaller")
                        plt.title(title, weight = "bold")
                        plt.plot(data[xname], data[yname], "k-", label = yname)
                        plt.legend(fontsize = "smaller")
                        plt.grid(True, linestyle='--')
                        pdf.savefig()
                        plt.close()

""" plot the training evolution of multiple models on the same plot """
def plot_state_many_wrt(names, datas, pdf, xcol, ycol):
        colnames = datas[0].dtype.names
        title = colnames[ycol + 1]
        # x axis - epoch/iteration index
        xname = colnames[xcol]
        xlabel = xname
        # y axis - train/validation/test datasets
        yname = colnames[ycol + 1]
        ylabel = yname.replace("train_", "").replace("valid_", "").replace("test_", "")
        # plot
        plt.xlabel(xlabel, fontsize = "smaller")
        plt.ylabel(ylabel, fontsize = "smaller")
        plt.title(title, weight = "bold")
        for data, name in zip(datas, names):
                plt.plot(data[xname], data[yname], label = name)
        plt.legend(fontsize = "smaller")
        plt.grid(True, linestyle='--')
        pdf.savefig()
        plt.close()

def plot_state_many(spaths, ppath):
        names, datas = get_state_csvs(spaths)
        with PdfPages(ppath) as pdf:
                for col in (0, 1, 2, 3, 4, 5, 7, 8):
                        # plot wrt epoch/iteration number
                        plot_state_many_wrt(names, datas, pdf, 0, col)
                        # plot wrt time
                        plot_state_many_wrt(names, datas, pdf, 7, col)

""" plot the test results of multiple models on the sample plot """
def plot_trial_many_wrt(title, names, datas, pdf, ycol):
        colnames = datas[0].dtype.names
        # x axis -
        xlabels = names
        # y axis - loss value, loss error, #epochs, convergence speed, time
        yname = colnames[ycol]
        ylabel = yname.replace("train_", "").replace("valid_", "").replace("test_", "").replace("_", " ")
        # plot
        plt.ylabel(ylabel, fontsize = "smaller")
        plt.title(title, weight = "bold")
        bdata = []
        for data in datas:
                bdata.append(data[yname])
        plt.boxplot(bdata, labels = xlabels)
        plt.grid(True, linestyle='--')
        pdf.savefig()
        plt.close()

def plot_trial_many(spaths, ppath, names):
        __, datas = get_trial_csvs(spaths)
        title = os.path.basename(ppath).replace(".pdf", "").replace("result_", "")
        with PdfPages(ppath) as pdf:
                for col in (4, 5, 6, 7, 8):
                        plot_trial_many_wrt(title, names, datas, pdf, col)
