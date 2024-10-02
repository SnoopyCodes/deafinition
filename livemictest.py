import cv2
import numpy as np
import pyaudio
import librosa
import librosa.display
import matplotlib.pyplot as plt
import time

rate = 26000
chunk_size = rate // 4


p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paFloat32,
                channels=1,
                rate=rate,
                input=True,
                input_device_index=1,
                frames_per_buffer=chunk_size)


frames = []

plt.figure(figsize=(10, 4))
do_melspec = librosa.feature.melspectrogram
pwr_to_db = librosa.core.power_to_db

while True:

    start = time.time()

    data = stream.read(chunk_size, exception_on_overflow = False)
    data = np.fromstring(data, dtype=np.float32)

    melspec = do_melspec(y=data, sr=rate, n_mels=128, fmax=4000)
    D = librosa.stft(data)  # STFT(short time fourier transformation) of y
    S_db = librosa.amplitude_to_db(np.abs(D), ref=np.max)
    frames.append(S_db)
    
    if len(frames) == 20:

       
        stack = np.hstack(frames)

        
        librosa.display.specshow(stack, y_axis='mel', fmax=4000, x_axis='time')
        plt.colorbar(format='%+2.0f dB')
        plt.title('Mel spectrogram')
        plt.draw()
        plt.pause(0.0001)
        plt.clf()
        #break
        frames.pop(0)


    t = time.time() - start

    print(1 / t)
    
    