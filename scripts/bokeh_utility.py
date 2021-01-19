from bokeh.plotting import figure, output_notebook, output_file, show
from bokeh.models import ColumnDataSource, Range1d
from bokeh.models.tools import HoverTool
from bokeh.io import output_notebook, show
output_notebook()
import numpy as np


def plot_raw(x, y, title, yrange = Range1d(0, (2**16)-1), legend = 'ADC'):
    hTool = HoverTool(
        tooltips=[
            ("x", "@x"),
            ("y", "@y"),
            ("index", '@index')
        ],
        mode='vline'
    )

    fig = figure(plot_width=900, plot_height=500, title = title)
    if yrange != None:
        fig.y_range = yrange
    fig.add_tools(hTool)
    data_source = ColumnDataSource(data=dict(x=x,
                                             y=y,
                                             index=np.arange(x.shape[0])))
    line = fig.line(x='x',
                    y='y',
                    source=data_source,
                    legend_label=legend,
                    line_width=2)
    show(fig)

def cal_fft(values):
    values = (values - 32767) * (3.3 / 65535.0)
    return np.absolute(np.fft.rfft(values))[1:]

def get_freqs(signal_length):
    sampling_frequency = 1000
    return np.arange(signal_length/2) * sampling_frequency / signal_length

def plot_fft(y):
    if y.shape[0] % 2 != 0:
        y = y[:-1]
    
    fft = cal_fft(y)
    freqs = get_freqs(signal_length = y.shape[0])
    assert (freqs.shape[0] == fft.shape[0]), f'fft and freqs cannot have different shape, fft.shape: {fft.shape} vs freqs.shape {freqs.shape}'
    plot_raw(x= freqs, y= fft, legend = 'fft', title='fft', yrange = None)
