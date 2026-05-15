import csv
import matplotlib.pyplot as plt
import numpy as np  

t = [] # column 0
data1 = [] # column 1
data_avg = []

file_names = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

for file_name in file_names:
    if file_name == 'sigB.csv':
        with open(file_name) as f:
            # open the csv file
            reader = csv.reader(f)
            for row in reader:
                # read the rows 1 one by one
                t.append(float(row[0])) # leftmost column
                data1.append(float(row[1])) # second column

        A = 0.9
        B = 1 - A # IIR filter coefficients
        data_avg.append(data1[0]) # Initialize the first value of the filtered signal
        for i in range(1, len(data1)):
            new_val = A * data_avg[i-1] + B * data1[i] # IIR filter formula
            data_avg.append(new_val)

        Fs = len(t) / (t[-1] - t[0]) # sample rate
        n = len(data1) # length of the signal
        
        # Calculate frequencies (Shared for both signals)
        k = np.arange(n)
        T = n / Fs
        frq = k / T
        frq = frq[:int(n/2)] # slice to one-sided frequency range
        
        # FFT for Unfiltered
        Y = np.fft.fft(data1) / n
        Y = Y[:int(n/2)]
        
        # FFT for Filtered
        Y_avg = np.fft.fft(data_avg) / n
        Y_avg = Y_avg[:int(n/2)]

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))
        fig.suptitle(f'Infinite Impulse Response (IIR) Filter(A = {A}, B = {B})') # Required title
        
        # Top plot: Time Domain
        ax1.plot(t, data1, 'k', label='Unfiltered') # 'k' is black
        ax1.plot(t, data_avg, 'r', label='Filtered') # 'r' is red
        ax1.set_xlabel('Time (s)')
        ax1.set_ylabel('Amplitude')
        ax1.legend()
        
        # Bottom plot: Frequency Domain (FFT)
        ax2.loglog(frq[1:], abs(Y[1:]), 'k', label='Unfiltered') 
        ax2.loglog(frq[1:], abs(Y_avg[1:]), 'r', label='Filtered')
        ax2.set_xlabel('Freq (Hz)')
        ax2.set_ylabel('|Y(freq)|')
        ax2.legend()
        
        plt.tight_layout() # Prevents labels from overlapping
        plt.savefig(file_name[:-4] + '_IIR.png') # save the figure as a png file
        plt.show()

        t = [] # reset t for the next file
        data1 = [] # reset data1 for the next file
        data_avg = [] # reset data_avg for the next file


