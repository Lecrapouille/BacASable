import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt
import json
import threading
from datetime import datetime

class GlobalDisplay:
    def __init__(self, root):
        self.root = root
        self.root.title("Global Display - All Sensors")
        self.root.geometry("500x550")

        # State of all sensors
        self.sensors = {
            "sensor_1": None,
            "sensor_2": None,
            "sensor_3": None,
            "sensor_4": None,
            "sensor_5": None,
            "sensor_6": None
        }

        self.last_update = {}

        # MQTT Configuration
        self.client = mqtt.Client(client_id="global_display", clean_session=False)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect

        self.broker = "localhost"
        self.port = 1884  # Broker 2

        self.setup_gui()
        self.connect_mqtt()

    def setup_gui(self):
        # Title
        title = tk.Label(self.root, text="📊 Global Display",
                         font=("Arial", 18, "bold"), pady=15)
        title.pack()

        # Connection Status
        self.status_label = tk.Label(self.root, text="⚠️ Disconnected from broker",
                                     fg="red", font=("Arial", 11, "bold"))
        self.status_label.pack(pady=5)

        # Separator
        ttk.Separator(self.root, orient="horizontal").pack(fill="x", padx=20, pady=10)

        # Frame for Board 1
        frame1 = ttk.LabelFrame(self.root, text="🔌 Board 1 - Sensors 1 to 3",
                                padding=15)
        frame1.pack(padx=20, pady=10, fill="both", expand=True)

        self.labels_card1 = {}
        for i in range(1, 4):
            sensor = f"sensor_{i}"
            self.create_sensor_row(frame1, sensor, i)
            self.labels_card1[sensor] = self.sensor_widgets[sensor]

        # Frame for Board 2
        frame2 = ttk.LabelFrame(self.root, text="🔌 Board 2 - Sensors 4 to 6",
                                padding=15)
        frame2.pack(padx=20, pady=10, fill="both", expand=True)

        self.labels_card2 = {}
        for i in range(4, 7):
            sensor = f"sensor_{i}"
            self.create_sensor_row(frame2, sensor, i)
            self.labels_card2[sensor] = self.sensor_widgets[sensor]

        # Global Statistics
        stats_frame = tk.Frame(self.root)
        stats_frame.pack(padx=20, pady=10, fill="x")

        self.stats_label = tk.Label(stats_frame,
                                    text="✓ OK: 0 | ✗ KO: 0 | ⚪ Unknown: 6",
                                    font=("Arial", 12, "bold"), bg="#f0f0f0",
                                    pady=10, relief="ridge", bd=2)
        self.stats_label.pack(fill="x")

        # QoS Info
        info = tk.Label(self.root, text="QoS 2 - Retained messages - Broker 2 (1884)",
                        font=("Arial", 9), fg="gray")
        info.pack(pady=5)

    def create_sensor_row(self, parent, sensor, number):
        if not hasattr(self, 'sensor_widgets'):
            self.sensor_widgets = {}

        row_frame = tk.Frame(parent)
        row_frame.pack(pady=7, fill="x")

        label = tk.Label(row_frame, text=f"Sensor {number}:",
                         font=("Arial", 12, "bold"), width=12, anchor="w")
        label.pack(side="left", padx=5)

        status = tk.Label(row_frame, text="⚪ Unknown", bg="#9E9E9E", fg="white",
                          font=("Arial", 11, "bold"), width=12, relief="raised", bd=2)
        status.pack(side="left", padx=5)

        time_label = tk.Label(row_frame, text="", font=("Arial", 9), fg="gray")
        time_label.pack(side="left", padx=10)

        self.sensor_widgets[sensor] = {
            'status': status,
            'time': time_label
        }

    def update_sensor_display(self, sensor, state):
        widgets = self.sensor_widgets[sensor]
        if state is True:
            widgets['status'].config(text="✓ OK", bg="#4CAF50")
        elif state is False:
            widgets['status'].config(text="✗ KO", bg="#f44336")
        else:
            widgets['status'].config(text="⚪ Unknown", bg="#9E9E9E")

        # Update timestamp
        now = datetime.now().strftime("%H:%M:%S")
        self.last_update[sensor] = now
        widgets['time'].config(text=f"Updated: {now}")

        self.update_stats()

    def update_stats(self):
        ok_count = sum(1 for v in self.sensors.values() if v is True)
        ko_count = sum(1 for v in self.sensors.values() if v is False)
        unknown_count = sum(1 for v in self.sensors.values() if v is None)

        self.stats_label.config(
            text=f"✓ OK: {ok_count} | ✗ KO: {ko_count} | ⚪ Unknown: {unknown_count}"
        )

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.status_label.config(text="✓ Connected to Broker 2", fg="green")
            # Subscribe only to sensors/all with QoS 2
            # This ensures we only receive data through card2_monitor
            self.client.subscribe("sensors/all", qos=2)
        else:
            self.status_label.config(text=f"✗ Connection error: {rc}", fg="red")

    def on_message(self, client, userdata, msg):
        try:
            data = json.loads(msg.payload.decode())
            print(f"Message received on {msg.topic}: {data}")

            # Update states
            for sensor, state in data.items():
                if sensor in self.sensors:
                    self.sensors[sensor] = state
                    self.update_sensor_display(sensor, state)

        except Exception as e:
            print(f"Error processing message: {e}")

    def on_disconnect(self, client, userdata, rc):
        self.status_label.config(text="⚠️ Disconnected from broker", fg="red")
        if rc != 0:
            print("Unexpected disconnection, reconnecting...")

    def connect_mqtt(self):
        try:
            self.client.connect(self.broker, self.port, 60)
            thread = threading.Thread(target=self.client.loop_forever, daemon=True)
            thread.start()
        except Exception as e:
            self.status_label.config(text=f"✗ Error: {str(e)}", fg="red")

    def on_closing(self):
        self.client.disconnect()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = GlobalDisplay(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
