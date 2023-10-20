import dearpygui.dearpygui as dpg

# **************************** 回调函数 ****************************

def button1CB():
    dpg.configure_item("button1", show=False)              # 隐藏开始按钮
    dpg.configure_item("button2", show=True)                # 显示停止按钮

def button2CB():
    dpg.configure_item("button2", show=False)              # 隐藏开始按钮
    dpg.configure_item("button1", show=True)                # 显示停止按钮

serlist = ["0","1","2","3","4","5","6","7","8"]

#******************************* UI ***********************************

dpg.create_context()
dpg.setup_dearpygui()
dpg.create_viewport(title='Vecang', width=400, height=647)

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
# 设置按钮主题
with dpg.theme() as button_theme2:
    with dpg.theme_component(dpg.mvAll):
        dpg.add_theme_color(dpg.mvThemeCol_Text, (249, 122, 94), category=dpg.mvThemeCat_Core)
        dpg.add_theme_color(dpg.mvThemeCol_Border,(249, 122, 94), category=dpg.mvThemeCat_Core)
        dpg.add_theme_style(dpg.mvStyleVar_FrameBorderSize, 2, category=dpg.mvThemeCat_Core)

# ************************* 主窗口 开始 ***************************
with dpg.window(tag="main_window") as window:
    dpg.add_combo(  tag="com_port", label="combo", 
                    width=148, pos=[10, 20],
                    default_value=serlist[0], items=serlist)   

# ************************* 子窗口 开始****************************
    with dpg.child_window(width=360, height=430,  pos=[10, 100] ):  
        # 添加 文本
        dpg.add_text(" Hello world!\n This is a dearpygui GUI demo.")

# ************************* 子窗口 结束 ***************************

    # 添加 按钮
    dpg.add_button(label="HELLO", tag="button1",show=True,   # 名字,标记,显示
                   width=100, height=40, pos=[140, 550],     # 大小、位置
                   callback=button1CB)                       # 回调函数
    dpg.bind_item_theme(dpg.last_item(), button_theme1)      # 设置主题

    # 添加 按钮
    dpg.add_button(label="World", tag="button2",show=False,   # 名字,标记,显示
                   width=100, height=40, pos=[140, 550],      # 大小、位置
                   callback=button2CB)                        # 回调函数
    dpg.bind_item_theme(dpg.last_item(), button_theme2)       # 设置主题

# ************************ 主窗口 结束 ***************************
   
dpg.set_primary_window("main_window", True)         # 将名为"main_window"的窗口设置为主窗口 
dpg.bind_item_theme(window, data_theme)

dpg.show_viewport()
dpg.start_dearpygui()
dpg.destroy_context()