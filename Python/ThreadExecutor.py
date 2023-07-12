from tkinter import *
from concurrent.futures import ThreadPoolExecutor
import time

win = Tk()
win.geometry("480x240")

def go():
  future = executor.submit(load_big_data)
  future.add_done_callback(data_received)

# Loading data take a loooonng time
def load_big_data():
  time.sleep(3)
  return "This is the data requested"

def data_received(future):
  data = future.result()
  label['text'] = data

def on_shutdown():
  executor.shutdown()
  win.destroy()

go = Button(win, text="GO", command=go)
go.pack()
label = Label(win, text="Waiting for data...")
label.pack()

canvas = Canvas(win, width=1024, height=300)
canvas.pack()

executor = ThreadPoolExecutor(max_workers=1)

win.protocol("WM_DELETE_WINDOW", on_shutdown)
win.mainloop()
