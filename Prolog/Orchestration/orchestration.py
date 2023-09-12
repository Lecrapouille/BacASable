from tkinter import *
import numpy as np
from pyswip import Prolog # pip install pyswip

win = Tk()
canvas = Canvas(win, width=1024, height=270)

###############################################################################
def distance(a, b):
    dx = a.x - b.x
    dy = a.y - b.y
    return np.sqrt(dx**2 + dy**2)

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
        canvas.create_text(self.x + 1, self.y - 12, anchor=W, font="Purisa", text=self.name.upper())

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
    def __init__(self, name_, x_, y_, orig_):
        self.name = name_
        self.radius = 30
        self.x = x_
        self.y = y_
        self.speed_x = 0
        self.speed_y = 0
        self.collided = False
        self.path = []
        self.origin = orig_
        self.final_destination = ''
        self.local_destination = ''
        self.status = StringVar()

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
            "a": Node("a", 40, 53),
            "b": Node("b", 117, 53),
            "c": Node("c", 222, 53),
            "d": Node("d", 889, 53),
            "e": Node("e", 966, 53),

            # 2ieme ligne de noeuds
            "f": Node("f", 40, 130),
            "g": Node("g", 117, 130),
            "h": Node("h", 222, 130),
            "i": Node("i", 889, 130),
            "j": Node("j", 966, 130),

            # 3ieme ligne de noeuds
            "k": Node("k", 40, 235),
            "l": Node("l", 117, 235),
            "m": Node("m", 661, 235),
            "n": Node("n", 889, 235),
            "o": Node("o", 966, 235),
        }

        self.arcs = {
            # 1ere ligne horizontale
            "ab": Arc(self.nodes, "ab"),
            "bc": Arc(self.nodes, "bc"),
            "cd": Arc(self.nodes, "cd"),
            "de": Arc(self.nodes, "de"),

            # 2ieme ligne horizontale
            "fg": Arc(self.nodes, "fg"),
            "gh": Arc(self.nodes, "gh"),
            "hi": Arc(self.nodes, "hi"),
            "ij": Arc(self.nodes, "ij"),

            # 3ieme ligne horizontale
            "kl": Arc(self.nodes, "kl"),
            "mn": Arc(self.nodes, "mn"),
            "no": Arc(self.nodes, "no"),

            # 1ere ligne verticale
            "af": Arc(self.nodes, "af"),
            "fk": Arc(self.nodes, "fk"),

            # 2ieme ligne verticale
            "bg": Arc(self.nodes, "bg"),
            "gl": Arc(self.nodes, "gl"),

            # 3ieme ligne verticale
            "ch": Arc(self.nodes, "ch"),

            # 4ieme ligne verticale
            "di": Arc(self.nodes, "di"),
            "in": Arc(self.nodes, "in"),

            # 5ieme ligne verticale
            "ej": Arc(self.nodes, "ej"),
            "jo": Arc(self.nodes, "jo"),
        }

    def to_prolog(self):
        f = open("deck.pl", "w")
        for a in self.nodes.values():
            f.write("arc(" + a.name + ", " + a.name + ", 0).\n")
        for a in self.arcs.values():
            d = distance(a.orig, a.dest)
            f.write("arc(" + a.orig.name + ", " + a.dest.name + ", " + str(d) + ").\n")
            f.write("arc(" + a.dest.name + ", " + a.orig.name + ", " + str(d) + ").\n")
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
            n = nodes[i].lower()
            self.bots.append(Bot("Bot" + str(i), self.deck.nodes[n].x, self.deck.nodes[n].y, n))
        self.to_prolog(nodes)

    def draw(self):
        self.deck.draw()
        for bot in self.bots:
            bot.draw()

    def to_prolog(self, nodes):
        self.deck.to_prolog()
        f = open("bots.pl", "w")
        for bot in range(len(self.bots)):
            n = nodes[bot].lower()
            f.write("bot(" + self.bots[bot].name.lower() + ", " + n + ", " + n + ").\n")
        f.close()

    def query_prolog(self, orig, dest):
        self.prolog.consult("orchestration.pl")
        #query = "findapath({},{},Path,[]).".format(orig, dest)
        query = "findminpath({},{},Distance,Path).".format(orig, dest)
        print("prolog query:", query) 
        for result in self.prolog.query(query):
            r = list(result["Path"])[1:]
            print("prolog solution:", r)
            return r
        return None

    def plannification(self, dt):
        for b in range(len(self.bots)):
            bot = self.bots[b]
            nodes = self.deck.nodes

            # No destination => no speed
            if (bot.final_destination == ''):
                bot.status.set("no destination given")
                bot.set_speed(0, 0)
                continue

            # Give a path
            if len(bot.path) == 0:
                bot.path = self.query_prolog(bot.origin, bot.final_destination)
                bot.local_destination = bot.path[0]
                bot.status.set("going to " + bot.local_destination)

            # Reached a destination ?
            n = nodes[bot.local_destination]
            dx = n.x - bot.x
            dy = n.y - bot.y
            distance = np.sqrt(dx**2 + dy**2)
            if distance == 0:
                # Reached the final destination ? no speed
                if bot.local_destination == bot.final_destination:
                    bot.status.set("reached destination") # FIXME never displayed
                    bot.origin = bot.final_destination
                    bot.final_destination = ''
                    bot.path = []
                    bot.set_speed(0, 0)
                    continue
                else:
                    bot.path.pop(0)
                    bot.local_destination = bot.path[0]
                    bot.status.set("going to " + bot.local_destination)

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

###############################################################################
class Application:
    def __init__(self, bots):
        self.timing = 10 # reschedule event in 10 ms
        self.backup = bots
        self.count_bots = len(bots)
        self.maestro = Maestro(bots)

        win.title("Bots orchestration")
        win.geometry("1024x320")
        canvas.pack()

        button_reset = Button(win, text="Reset", command=self.reset)
        button_reset.pack(side="left")

        self.entries = []
        self.labels = []
        for i in range(self.count_bots):
            Label(win, text="Bot" + str(i)).pack(side="left")
            entry = Entry(win)
            entry.pack(side="left")
            self.entries.append(entry)
            label = Label(win, textvariable = self.maestro.bots[i].status)
            label.pack(side="left")
            self.labels.append(label)

        button_go = Button(win, text="Go", command=self.go)
        button_go.pack(side="left")

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
            self.maestro.bots[i].final_destination = self.entries[i].get().lower()

###############################################################################
if __name__ == '__main__':
    app = Application(["I"])
    app.run()
