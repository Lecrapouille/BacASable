from tkinter import *
import numpy as np
from pyswip import Prolog # pip install pyswip

win = Tk()
canvas = Canvas(win, width=1024, height=270)

###############################################################################
class Rect:
    def __init__(self, x1_, y1_, x2_, y2_):
        self.x1 = x1_
        self.x2 = x2_
        self.y1 = y1_
        self.y2 = y2_

    def overlap(self, other):
        return (self.x1 <= other.x2) and (other.x1 <= self.x2) and (self.y1 <= other.y2) and (other.y1 <= self.y2)

###############################################################################
class Node:
    def __init__(self, name_, x_, y_):
        self.name = name_
        self.x = x_
        self.y = y_

    def draw(self):
        radius = 5
        x0 = self.x - radius
        y0 = self.y - radius
        x1 = self.x + radius
        y1 = self.y + radius
        canvas.create_oval(x0, y0, x1, y1, fill="blue")
        canvas.create_text(self.x + 1, self.y - 12, anchor=W, font="Purisa", text=self.name)

###############################################################################
class Arc:
    def __init__(self, nodes_, name_):
        self.name = name_
        self.orig = nodes_[self.name[0]]
        self.dest = nodes_[self.name[1]]

    def draw(self):
        canvas.create_line(self.orig.x, self.orig.y, self.dest.x, self.dest.y, fill="blue", width=2)

###############################################################################
class Bot:
    def __init__(self, name_, x_, y_):
        self.name = name_
        self.radius = 30
        self.x = x_
        self.y = y_
        self.speed_x = 0
        self.speed_y = 0
        self.collided = False
        self.destination = ''

    def set_speed(self, x_, y_):
        self.speed_x = x_
        self.speed_y = y_

    def update(self, dt):
        if self.collided:
            self.speed_x = 0
            self.speed_y = 0
            return
        self.x = self.x + self.speed_x * dt
        self.y = self.y + self.speed_y * dt

    def collides(self, other):
        if self.name == other.name:
            return False
        r = self.radius
        r1 = Rect(self.x - r, self.y - r, self.x + r, self.y + r)
        other = Rect(other.x - r, other.y - r, other.x + r, other.y + r)
        return r1.overlap(other)

    def draw(self):
        r = self.radius
        color = "#f00" if self.collided else "#0f0"
        canvas.create_rectangle(self.x - r, self.y - r, self.x + r, self.y + r, outline="#000", fill=color, width=2)
        canvas.create_text(self.x - 20, self.y, anchor=W, font="Purisa", text=self.name)

###############################################################################
class Deck:
    def __init__(self):
        self.nodes = {
            # 1ere ligne de noeuds
            "A": Node("A", 40, 53),
            "B": Node("B", 117, 53),
            "C": Node("C", 222, 53),
            "D": Node("D", 889, 53),
            "E": Node("E", 966, 53),

            # 2ieme ligne de noeuds
            "F": Node("F", 40, 130),
            "G": Node("G", 117, 130),
            "H": Node("H", 222, 130),
            "I": Node("I", 889, 130),
            "J": Node("J", 966, 130),

            # 3ieme ligne de noeuds
            "K": Node("K", 40, 235),
            "L": Node("L", 117, 235),
            "M": Node("M", 661, 235),
            "N": Node("N", 889, 235),
            "O": Node("O", 966, 235),
        }

        self.arcs = {
            # 1ere ligne horizontale
            "AB": Arc(self.nodes, "AB"),
            "BC": Arc(self.nodes, "BC"),
            "CD": Arc(self.nodes, "CD"),
            "DE": Arc(self.nodes, "DE"),

            # 2ieme ligne horizontale
            "FG": Arc(self.nodes, "FG"),
            "GH": Arc(self.nodes, "GH"),
            "HI": Arc(self.nodes, "HI"),
            "IJ": Arc(self.nodes, "IJ"),

            # 3ieme ligne horizontale
            "KL": Arc(self.nodes, "KL"),
            "MN": Arc(self.nodes, "MN"),
            "NO": Arc(self.nodes, "NO"),

            # 1ere ligne verticale
            "AF": Arc(self.nodes, "AF"),
            "FK": Arc(self.nodes, "FK"),

            # 2ieme ligne verticale
            "BG": Arc(self.nodes, "BG"),
            "GL": Arc(self.nodes, "GL"),

            # 3ieme ligne verticale
            "CH": Arc(self.nodes, "CH"),

            # 4ieme ligne verticale
            "DI": Arc(self.nodes, "DI"),
            "IN": Arc(self.nodes, "IN"),

            # 5ieme ligne verticale
            "EJ": Arc(self.nodes, "EJ"),
            "JO": Arc(self.nodes, "JO"),
        }

    def to_prolog(self):
        f = open("deck.pl", "w")
        for a in self.arcs.values():
            f.write("arc(" + a.orig.name.lower() + ", " + a.dest.name.lower() + ").\n")
            f.write("arc(" + a.dest.name.lower() + ", " + a.orig.name.lower() + ").\n")
        f.close()

    def draw(self):
        for n in self.nodes.values():
            n.draw()
        for a in self.arcs.values():
            a.draw()

###############################################################################
class Maestro:
    def __init__(self, nodes):
        self.deck = Deck()
        self.prolog = Prolog()
        self.collision = False
        self.bots = []
        for i in range(len(nodes)):
            self.bots.append(Bot("Bot" + str(i), self.deck.nodes[nodes[i]].x, self.deck.nodes[nodes[i]].y))
        self.to_prolog(nodes)

    def draw(self):
        self.deck.draw()
        for bot in self.bots:
            bot.draw()

    def to_prolog(self, nodes):
        self.deck.to_prolog()
        f = open("bots.pl", "w")
        for bot in range(len(self.bots)):
            f.write("sur(" + self.bots[bot].name.lower() + ", " + nodes[bot].lower() + ").\n")
        f.close()

    def plannification(self, dt):
        for b in range(len(self.bots)):
            bot = self.bots[b]
            nodes = self.deck.nodes

            # No destination => no speed
            if bot.destination == '':
                bot.set_speed(0, 0)
                continue

            # Reached destination ? => no speed
            n = nodes[bot.destination]
            dx = n.x - bot.x
            dy = n.y - bot.y
            distance = np.sqrt(dx**2 + dy**2)
            if distance == 0:
                bot.destination = ''
                bot.set_speed(0, 0)
                continue

            velocity = 1/dt
            speed_x = velocity * np.sign(dx)
            speed_y = velocity * np.sign(dy)
            bot.set_speed(speed_x, speed_y)

    def update(self, dt):
        if self.collision:
            return
        self.plannification(dt)
        for bot in self.bots:
            bot.update(dt)
            for other in self.bots:
                if bot.collides(other):
                    self.collided = True
                    other.collided = True
                    continue

    def foobar(self):
        self.prolog.consult("orchestration.pl")
        for res in self.prolog.query("noeud(X)."):
            print(res)
            print('===')
        for res in self.prolog.query("noeuds(X)."):
            print(res)

###############################################################################
class Application:
    def __init__(self, bots):
        self.timing = 10 # reschedule event in 10 ms
        self.backup = bots
        self.count_bots = len(bots)

        win.title("Bots orchestration")
        win.geometry("1024x320")
        canvas.pack()

        button_reset = Button(win, text="Reset", command=self.reset)
        button_reset.pack(side="left")

        self.entries = []
        for i in range(self.count_bots):
            Label(win, text="Bot" + str(i)).pack(side="left")
            entry = Entry(win)
            entry.pack(side="left")
            self.entries.append(entry)

        button_go = Button(win, text="Go", command=self.go)
        button_go.pack(side="left")

        self.maestro = Maestro(bots)
        win.after(self.timing, self.task)

    def run(self):
        win.mainloop()

    def task(self):
        self.maestro.update(0.01)
        canvas.delete("all")
        self.maestro.draw()
        win.after(self.timing, self.task)

    def reset(self):
        self.maestro = Maestro(self.backup)

    def go(self):
        for i in range(self.count_bots):
            self.maestro.bots[i].destination = self.entries[i].get()

###############################################################################
if __name__ == '__main__':
    app = Application(["J", "I"])
    app.run()
