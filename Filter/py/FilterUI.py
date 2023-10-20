import dearpygui.dearpygui as dpg
import numpy as np
import scipy.signal

serlist = ["低通 LowPass","高通 HighPass","带通 BandPass","带阻 BandStop" ]

# **************************** 回调函数 ****************************
def UIDebug(text):
    dpg.set_value("textDebugt", dpg.get_value("textDebugt") + "\n" + text)
def UIShow(text):
    dpg.set_value("textOut", dpg.get_value("textOut") + "\n" +text)

def button1CB():
    dpg.set_value("textOut", "")
    dpg.configure_item("button1", show=True)              # 隐藏开始按钮
    
    if dpg.get_value("textOrder").isdigit() and dpg.get_value("textFreqs").isdigit() and  dpg.get_value("textRate").isdigit() and dpg.get_value("textWidth").isdigit():
        order=int(dpg.get_value("textOrder"))
        freqs=int( dpg.get_value("textFreqs"))
        rate= int(dpg.get_value("textRate") )
        width = int(dpg.get_value("textWidth"))
        type= dpg.get_value("comboFilter")
        if(type==serlist[0]):
            type="lowpass"
        elif(type==serlist[1]):
            type="highpass"
        elif(type==serlist[2]):
            type='bandpass'
            freqs = [freqs-0.5*width,freqs+0.5*width]
        elif(type==serlist[3]):
            type='bandstop'
            freqs = [freqs-0.5*width,freqs+0.5*width]

        UIDebug("滤波阶数:  {0}\n截止频率:  {1}\n滤波器类型:  {2}\n采样率:  {3}".format(order,freqs,type,rate)) 
        sos = scipy.signal.butter(N=order, Wn=freqs, btype=type, analog=False, output='sos', fs=rate)
        UIDebug("通用滤波:\n{}".format(str(sos)))
        UIDebug("==============================================================")
        UIShow("// >>> Butterworth IIR Digital Filter: {0}".format(type))
        UIShow("// \tSampling Rate:{0} Hz ,Frequency:{1} Hz".format(rate,freqs))
        UIShow("// \tOrder: {0} ,implemented as second-order sections (biquads)".format(order))

        UIShow("float Filter(float input)\n{ \n\tfloat output = input;")


        for section in sos:
            b0, b1, b2, a0, a1, a2 = section
            UIShow(f"""\
    {{
        static float z1, z2; // filter section state
        float x = output - ({a1:.8f}*z1 )- ({a2:.8f}*z2);
        output = {b0:.8f}*x + ({b1:.8f}*z1 )+ ({b2:.8f}*z2);
        z2 = z1;
        z1 = x;
    }}
    """)
        UIShow("""\treturn output\n}""")

    else:
        UIShow("输入错误")
    



#******************************* UI ***********************************

dpg.create_context()
dpg.setup_dearpygui()
dpg.create_viewport(title='Butterworth IIR Dgital Filter -- By Vecang', width=400, height=647)

# ************************** *1* 主题 ****************************
# 设置主窗口主题
with dpg.theme() as data_theme:
    with dpg.theme_component(dpg.mvAll):
        # 设置窗口背景颜色#081e30  #00101c
        dpg.add_theme_color(dpg.mvThemeCol_WindowBg,(26, 30, 32), category=dpg.mvThemeCat_Core)  # 设置窗口背景颜色
        dpg.add_theme_color(dpg.mvThemeCol_ChildBg, (36, 40, 42), category=dpg.mvThemeCat_Core)  # 设置子窗口背景颜色
        dpg.add_theme_style(dpg.mvStyleVar_ChildRounding, 8, category=dpg.mvThemeCat_Core)       # 设置子窗口圆角
        dpg.add_theme_style(dpg.mvStyleVar_FrameRounding, 8, category=dpg.mvThemeCat_Core)       # 设置框架圆角
        dpg.add_theme_style(dpg.mvStyleVar_ChildBorderSize, 0, category=dpg.mvThemeCat_Core)     

# 设置按钮主题
with dpg.theme() as button_theme1:
    with dpg.theme_component(dpg.mvAll):
        dpg.add_theme_color(dpg.mvThemeCol_Text, (30, 128, 220), category=dpg.mvThemeCat_Core)  # 文字颜色
        dpg.add_theme_color(dpg.mvThemeCol_Border,(30, 128, 220), category=dpg.mvThemeCat_Core) # 背景颜色
        dpg.add_theme_style(dpg.mvStyleVar_FrameBorderSize, 2, category=dpg.mvThemeCat_Core)    # 边框大小:2
        # 设置字体
        with dpg.font_registry():
            with dpg.font("resource/SmileySans-Oblique.ttf", 21) as font_regular_21_ch:
                dpg.add_font_range_hint(dpg.mvFontRangeHint_Chinese_Full)

# ************************* 主窗口 开始 ***************************
with dpg.window(tag="main_window") as window:

# ************************* 子窗口 开始****************************
    with dpg.child_window(width=360, height=210,  pos=[10, 10] ):  
        # 添加 文本
        dpg.add_combo(  tag="comboFilter", label="   滤波器类型", 
                    width=148, pos=[10, 10],
                    default_value=serlist[0], items=serlist)   
        dpg.add_input_text(label="   采样率(Hz)",   default_value="500", tag="textRate",  multiline=True, pos=[10, 50], width=200, height=30,   tab_input=True,readonly=False)
        dpg.add_input_text( label="  阶数",        default_value="4",   tag="textOrder", multiline=True, pos=[10, 90],  width=200, height=30,   tab_input=True,readonly=False)
        dpg.add_input_text( label="  截止频率(Hz)", default_value="70",  tag="textFreqs", multiline=True, pos=[10, 130], width=200, height=30,  tab_input=True,readonly=False)
        dpg.add_input_text(label="   带宽(Hz)",     default_value="1",   tag="textWidth", multiline=True, pos=[10, 170], width=200, height=30,  tab_input=True,readonly=False)
        dpg.bind_font(font_regular_21_ch) 
         
    dpg.add_input_text(default_value="Hello :)", tag="textOut",  multiline=True, pos=[10, 240], width=360, height=280,   tab_input=True,readonly=False)
    dpg.add_input_text(default_value="Author: Vecang\n==============================================================", tag="textDebugt",  multiline=True, pos=[360+30, 10], width=560, height=580, tab_input=True,readonly=True)
 
# ************************* 子窗口 结束 ***************************

    # 添加 按钮
    dpg.add_button(label="生成", tag="button1",show=True,   # 名字,标记,显示
                   width=360, height=40, pos=[10, 550],     # 大小、位置
                   callback=button1CB)                       # 回调函数
    dpg.bind_item_theme(dpg.last_item(), button_theme1)      # 设置主题

 
# ************************ 主窗口 结束 ***************************
   
dpg.set_primary_window("main_window", True)         # 将名为"main_window"的窗口设置为主窗口 
dpg.bind_item_theme(window, data_theme)

dpg.show_viewport()
dpg.start_dearpygui()
dpg.destroy_context()