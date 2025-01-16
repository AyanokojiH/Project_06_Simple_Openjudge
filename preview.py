import curses
import subprocess
import os
import sys
import re
import time

ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

def main(stdscr):
    curses.curs_set(0)  # 隐藏光标
    stdscr.clear()
    stdscr.refresh()
    curses.start_color()
    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)  # 红色文本
    curses.init_pair(2, curses.COLOR_BLUE, curses.COLOR_BLACK)  # 蓝色文本
    curses.init_pair(3, curses.COLOR_GREEN, curses.COLOR_BLACK)
    red = curses.color_pair(1)
    blue = curses.color_pair(2)
    green = curses.color_pair(3)

    # 题目选择
    questions = ["题目1>    A+B Problem                 难度:Low           分类:基础I/O", 
                 "题目2>    爬楼梯                      难度:Low           分类:递推与递归", 
                 "题目3>    移除k位数字                 难度:Mid           分类:单调栈/dp",
                 "题目4>    最大宽度坡                  难度:Mid           分类:单调栈",
                 "题目5>    最多能完成排序的块          难度:High          分类:单调栈",
                 "题目6>    序列的中位数                难度:Mid           分类:优先队列/堆",
                 "题目7>    滑动窗口                    难度:High          分类:优先队列/单调队列",
                 "题目8>    堆栈基本操作                难度:Low           分类:栈"]
    current_row = 0
    while True:
        stdscr.clear()
        for i, question in enumerate(questions):
            if i == current_row:
                stdscr.addstr(f"> {question}\n")
            else:
                stdscr.addstr(f"  {question}\n")
        stdscr.refresh()
        
        key = stdscr.getch()
        if key == curses.KEY_UP and current_row > 0:
            current_row -= 1
        elif key == curses.KEY_DOWN and current_row < len(questions) - 1:
            current_row += 1
        elif key == ord('\n'):
            break

    selected_question = questions[current_row]
    # 使用正则表达式提取题目编号
    match = re.search(r'题目(\d+)', selected_question)
    if match:
        question_num = match.group(1)
    else:
        question_num = "1"  # 默认值，如果未匹配到编号

    question_file = f"info{question_num}/question{question_num}.md"

    # 加载题目描述
    if os.path.exists(question_file):
        with open(question_file, "r", encoding="utf-8") as file:
            question_desc = file.read()
    else:
        question_desc = "题目描述文件不存在。"

    height, width = stdscr.getmaxyx()
    question_desc_lines = question_desc.split('\n')

    # 滚动显示内容
    scroll_pos = 0
    while True:
        stdscr.clear()
        for i in range(height - 3):
            line_idx = i + scroll_pos
            if line_idx < len(question_desc_lines):
                stdscr.addstr(i, 0, question_desc_lines[line_idx][:width-1])
        stdscr.addstr(height - 3, 0, "Press Enter to continue, Esc to leave...")
        stdscr.refresh()

        key = stdscr.getch()
        if key == ord('\n'):  # Enter键
            break
        elif key == 27:  # Esc键
            stdscr.clear()
            return
        elif key == curses.KEY_UP and scroll_pos > 0:
            scroll_pos -= 1
        elif key == curses.KEY_DOWN and scroll_pos < len(question_desc_lines) - (height - 3):
            scroll_pos += 1

    # 隐藏光标
    curses.curs_set(0)

    command = f"make && ./code {question_num}"
    # 在TUI中运行命令并实时显示输出
    curses.endwin()  # 结束 curses 模式，以便可以在终端上打印输出

    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # 等待进程结束
    stdout, stderr = process.communicate()
    
    # 移除ANSI转义序列
    stdout = ansi_escape.sub('', stdout)
    stderr = ansi_escape.sub('', stderr)

    # 显示输出
    stdscr.clear()
    if stdout:
        lines = stdout.split('\n')
        for line in lines:
            # 替换颜色宏并使用颜色对
            line = line.replace("RED", "").replace("RESET", "").replace("BLUE", "")
            if "MEMORY LIMIT EXCEEDED" in line or "TIME LIMIT EXCEEDED" in line or "WRONG ANSWER" in line:
                stdscr.addstr(line + '\n', red)
            elif "ACCEPTED" in line:
                stdscr.addstr(line + '\n', blue)
            else:
                stdscr.addstr(line + '\n')
    if stderr:
        stdscr.addstr("COMPILE ERROR\n\n", green)
        stdscr.addstr("Error:\n", red)
        stdscr.addstr(stderr, red)
    stdscr.refresh()
    
    stdscr.addstr("\nPress Enter to exit...")
    stdscr.refresh()
    stdscr.getch()  # 等待用户按键

    # 结束curses模式
    curses.endwin()

if __name__ == "__main__":
    try:
        curses.wrapper(main)
    except Exception as e:
        # 如果捕获到异常，可以在这里处理，例如打印到日志文件或者忽略
        pass
    finally:
        # 确保终端状态被正确恢复
        try:
            curses.endwin()
        except:
            pass
        sys.exit(0)