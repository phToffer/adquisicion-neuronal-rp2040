import serial
import numpy as np
import matplotlib.pyplot as plt
from collections import deque

PORT = "COM6"
BAUDRATE = 115200

SAMPLE_RATE = 10000
BUFFER_SIZE = 1000
DISPLAY_SAMPLES = 5000

VREF = 3.3

ser = serial.Serial(
    PORT,
    BAUDRATE,
    timeout=1
)

signal = deque(
    np.zeros(DISPLAY_SAMPLES),
    maxlen=DISPLAY_SAMPLES
)

time_axis = np.arange(DISPLAY_SAMPLES) / SAMPLE_RATE

plt.ion()

fig, ax = plt.subplots(figsize=(12, 6))

line, = ax.plot(
    time_axis,
    signal,
    linewidth=1.5
)

ax.set_title("Adquisición de señal - RP2040")
ax.set_xlabel("Tiempo (s)")
ax.set_ylabel("Voltaje (V)")

ax.set_xlim(
    0,
    DISPLAY_SAMPLES / SAMPLE_RATE
)

ax.set_ylim(
    0,
    VREF
)

ax.grid(
    True,
    alpha=0.3
)

info = ax.text(
    0.02,
    0.95,
    "",
    transform=ax.transAxes,
    verticalalignment="top",
    fontsize=10
)

fig.tight_layout()

while True:

    byte = ser.read(1)

    if not byte:
        continue

    if byte[0] != 0xAA:
        continue

    byte = ser.read(1)

    if not byte or byte[0] != 0x55:
        continue

    data = ser.read(BUFFER_SIZE)

    if len(data) != BUFFER_SIZE:
        continue

    adc_data = np.frombuffer(
        data,
        dtype=np.uint8
    )

    voltage = adc_data.astype(np.float32) * VREF / 255.0

    signal.extend(voltage)

    current = voltage[-1]
    minimum = np.min(voltage)
    maximum = np.max(voltage)
    average = np.mean(voltage)

    line.set_ydata(signal)

    info.set_text(
        f"Fs: {SAMPLE_RATE / 1000:.1f} kS/s\n"
        f"ADC actual: {adc_data[-1]}\n"
        f"Voltaje: {current:.3f} V\n"
        f"Min: {minimum:.3f} V\n"
        f"Max: {maximum:.3f} V\n"
        f"Promedio: {average:.3f} V"
    )

    fig.canvas.draw_idle()
    fig.canvas.flush_events()