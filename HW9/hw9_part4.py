import csv
import matplotlib.pyplot as plt
import numpy as np  

t = [] # column 0
data1 = [] # column 1

file_names = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

for file_name in file_names:
    with open(file_name) as f:
        # open the csv file
        reader = csv.reader(f)
        for row in reader:
            # read the rows 1 one by one
            t.append(float(row[0])) # leftmost column
            data1.append(float(row[1])) # second column

    Fs = len(t) / (t[-1] - t[0]) # sample rate
    Ts = 1.0/Fs; # sampling interval
    ts = np.arange(0,t[-1],Ts) # time vector
    y = data1 # the data to make the fft from
    n = len(y) # length of the signal
    k = np.arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(int(n/2))] # one side frequency range
    Y = np.fft.fft(y)/n # fft computing and normalization
    Y = Y[range(int(n/2))]

    fig, (ax1, ax2) = plt.subplots(2, 1)
    ax1.plot(t,y,'b')
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    ax2.loglog(frq,abs(Y),'b') # plotting the fft
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    plt.savefig(file_name[:-4] + '.png') # save the figure as a png file
    # plt.show()

    t = [] # reset t for the next file
    data1 = [] # reset data1 for the next file

