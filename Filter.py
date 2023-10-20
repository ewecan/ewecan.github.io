#!/usr/bin/env python3

usage_guide = """\ 
    这个脚本使用SciPy信号处理库生成数字IIR滤波器的C/C++代码。它使用著名的Butterworth设计,
    优化平坦的频率响应。

    脚本参数指定了滤波器的特性：
        滤波器类型确定了要阻止的频率范围。
        低通滤波器通过阻止高频信号来平滑信号；
        高通滤波器通过阻止低频信号来去除常量分量；
        带通滤波器阻止低频和高频信号以强调感兴趣的范围；
        带阻滤波器阻止一段频率范围以去除特定的非期望频率分量，如周期性噪声源。

    采样频率: 传感器信号采样的恒定速率，以每秒样本数指定。
    滤波器阶数:确定了状态变量的数量和频率响应的陡峭程度。它指定了定义滤波器的频率空间多项式中的项数。

    对于低通和高通滤波器，临界频率指定了理想滤波器曲线的转折点，单位为赫兹。
    滤波器具有衰减，因此随着频率超出这个转折点进入被阻止范围，阻止强度增加。

    对于带通和带阻滤波器，临界频率是频带的中心，宽度是频带的总宽度。

    可选的绘图显示了响应幅度与频率的关系。这种方式不太常见，但避免了分贝或对数垂直刻度的潜在混淆。

    参考资料：
    https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html
    https://docs.scipy.org/doc/scipy/reference/tutorial/signal.html
    https://en.wikipedia.org/wiki/Butterworth_filter
    https://en.wikipedia.org/wiki/Digital_filter#Direct_form_II
"""


################################################################
# Standard Python libraries.
import sys, argparse, logging

# Set up debugging output.
logging.getLogger().setLevel(logging.DEBUG)

# Extension libraries.
import numpy as np
import scipy.signal

type_print_form = {'lowpass' : 'Low-Pass', 'highpass' : 'High-Pass', 'bandpass' : 'Band-Pass', 'bandstop' : 'Band-Stop'}

################################################################
# 可选择生成滤波器属性的图形。
# Refs: https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.sosfreqz.html
def make_plots(filename, sos, fs, order, freqs, name):
    try:
        import matplotlib.pyplot as plt
    except:
        print("Warning, matplotlib not found, skipping plot generation.")
        return
            
    # N.B. response is a vector of complex numbers
    freq, response = scipy.signal.sosfreqz(sos, fs=fs)
    fig, ax = plt.subplots(nrows=1)
    fig.set_dpi(160)
    fig.set_size_inches((8,6))
    ax.plot(freq, np.abs(response))
    ax.set_title(f"Response of {freqs} Hz {name} Filter of Order {order}")
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Magnitude of Transfer Ratio")
    fig.savefig(filename)


################################################################
def emit_biquad_code(stream, coeff):
    # 发出直接形式II中单个双二阶段的C代码。
    # 为简单起见，滤波器状态直接嵌入在块中作为静态变量，并且系数直接出现在表达式中。

    # b0、b1、b2是分子系数
    # a0、a1、a2是分母系数
    # 系数被归一化，使得a0==1
    b0, b1, b2, a0, a1, a2 = coeff
    stream.write(f"""\
  {{
    static float z1, z2; // filter section state
    float x = output - {a1:.8f}*z1 - {a2:.8f}*z2;
    output = {b0:.8f}*x + {b1:.8f}*z1 + {b2:.8f}*z2;
    z2 = z1;
    z1 = x;
  }}
""")

################################################################    
def emit_filter_function(stream, name, sos):
    stream.write(f"""\
float {name}(float input)
{{
  float output = input;
""")
    for section in sos:
        emit_biquad_code(stream, section)

    stream.write("  return output;\n}\n")

################################################################
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""Generate C code for Butterworth IIR digital filters.""",
                                     formatter_class=argparse.RawDescriptionHelpFormatter,
                                     epilog=usage_guide)
    parser.add_argument('--type',  default='lowpass',  type=str,
                        choices = ['lowpass', 'highpass', 'bandpass', 'bandstop'], 
                        help = 'Filter type: lowpass, highpass, bandpass, bandstop (default lowpass).')
    
    parser.add_argument('--rate',  default=500,  type=float, help = 'Sampling frequency in Hz (default 500).')
    parser.add_argument('--order', default=4,   type=int,   help = 'Filter order (default 4).')
    parser.add_argument('--freq',  default=1.0, type=float, help = 'Critical frequency (default 1.0 Hz).')
    parser.add_argument('--width', default=1.0, type=float, help = 'Bandwidth (for bandpass or bandstop) (default 1.0 Hz).')
    parser.add_argument('--name',  type=str, help = 'Name of C filter function.')
    parser.add_argument('--out',   type=str, help='Path of C output file for filter code.')
    parser.add_argument('--plot',  type=str, help='Path of plot output image file.')
    args = parser.parse_args()

    if args.type == 'lowpass':
        freqs = args.freq
        funcname = 'lowpass' if args.name is None else args.name
        
    elif args.type == 'highpass':
        freqs = args.freq
        funcname = 'highpass' if args.name is None else args.name
     
    elif args.type == 'bandpass':
        freqs = [args.freq - 0.5*args.width, args.freq + 0.5*args.width]
        funcname = 'bandpass' if args.name is None else args.name
        
    elif args.type == 'bandstop':                
        freqs = [args.freq - 0.5*args.width, args.freq + 0.5*args.width]
        funcname = 'bandstop' if args.name is None else args.name    

    # 生成一个巴特沃斯滤波器，作为一个串联的二阶数字滤波器系列（也称为二阶段或双二次滤波器）。
    sos = scipy.signal.butter(N=args.order, Wn=freqs, btype=args.type, analog=False, output='sos', fs=args.rate)

    logging.debug("SOS filter: %s", sos)

    filename = args.type + '.ino' if args.out is None else args.out
    stream = open(filename, "w")

    printable_type = type_print_form[args.type]
    stream.write(f"// {printable_type} Butterworth IIR digital filter, generated using filter_gen.py.\n")
    stream.write(f"// Sampling rate: {args.rate} Hz, frequency: {freqs} Hz.\n")
    stream.write(f"// Filter is order {args.order}, implemented as second-order sections (biquads).\n")
    stream.write("// Reference: https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html\n")
    funcname = args.type if args.name is None else args.name    
    emit_filter_function(stream, funcname, sos)
    stream.close()

    if args.plot is not None:
        make_plots(args.plot, sos, args.rate, args.order, freqs, printable_type)
        
################################################################
