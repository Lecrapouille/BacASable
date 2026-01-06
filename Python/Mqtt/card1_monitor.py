import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt
import json
import threading

class Carte1Monitor:
    def __init__(self, root):
        self.root = root
        self.root.title("Board 1 - 3 Sensors Monitor")
        self.root.geometry("400x350")

        # Sensor states (True = OK, False = KO)
        self.sensors = {
            "sensor_1": True,
            "sensor_2": True,
            "sensor_3": True
        }

        # Flag to track if states have been restored from retained message
        self.states_restored = False

        # MQTT Configuration
        self.client = mqtt.Client(client_id="card1_monitor", clean_session=False)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect

        self.broker = "localhost"
        self.port = 1883
        self.topic_publish = "sensors/card1"
        self.topic_restore = "sensors/all"

        self.setup_gui()
        self.connect_mqtt()

    def setup_gui(self):
        # Title
        title = tk.Label(self.root, text="🔌 Board 1 - Sensors",
                         font=("Arial", 16, "bold"), pady=10)
        title.pack()

        # Connection status
        self.status_label = tk.Label(self.root, text="⚠️ Disconnected",
                                     fg="red", font=("Arial", 10))
        self.status_label.pack(pady=5)

        # Frame for sensors
        frame = ttk.LabelFrame(self.root, text="Sensor States", padding=10)
        frame.pack(padx=20, pady=10, fill="both", expand=True)

        self.buttons = {}
        for i, (sensor, state) in enumerate(self.sensors.items(), 1):
            row_frame = tk.Frame(frame)
            row_frame.pack(pady=8, fill="x")

            label = tk.Label(row_frame, text=f"Sensor {i}:",
                             font=("Arial", 11), width=12, anchor="w")
            label.pack(side="left", padx=5)

            btn = tk.Button(row_frame, text="✓ OK", bg="#4CAF50", fg="white",
                            font=("Arial", 10, "bold"), width=12,
                            command=lambda s=sensor: self.toggle_sensor(s))
            btn.pack(side="left", padx=5)
            self.buttons[sensor] = btn

        # Manual publish button
        publish_btn = tk.Button(self.root, text="📤 Publish States",
                                command=self.publish_states,
                                bg="#2196F3", fg="white",
                                font=("Arial", 11, "bold"), pady=8)
        publish_btn.pack(pady=10, padx=20, fill="x")

        # Info QoS
        info = tk.Label(self.root, text="QoS 2 - Retained messages",
                        font=("Arial", 9), fg="gray")
        info.pack()

    def toggle_sensor(self, sensor):
        self.sensors[sensor] = not self.sensors[sensor]
        self.update_button(sensor)
        self.publish_states()

    def update_button(self, sensor):
        btn = self.buttons[sensor]
        if self.sensors[sensor]:
            btn.config(text="✓ OK", bg="#4CAF50")
        else:
            btn.config(text="✗ KO", bg="#f44336")

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.status_label.config(text="✓ Connected to broker", fg="green")
            # Subscribe to sensors/all for state restoration (via bridge)
            self.client.subscribe(self.topic_restore, qos=2)
            # Schedule publish after a short delay to allow restoration
            self.root.after(500, self.initial_publish)
        else:
            self.status_label.config(text=f"✗ Connection error: {rc}", fg="red")

    def initial_publish(self):
        """Publish states after connection, whether restored or not"""
        self.publish_states()

    def on_message(self, client, userdata, msg):
        """Handle incoming messages for state restoration"""
        try:
            if msg.topic == self.topic_restore:
                data = json.loads(msg.payload.decode())
                print(f"Received sensors/all for restoration: {data}")
                
                # Restore only our sensors (1-3)
                restored = False
                for sensor in self.sensors.keys():
                    if sensor in data and data[sensor] is not None:
                        self.sensors[sensor] = data[sensor]
                        self.update_button(sensor)
                        restored = True
                
                if restored and not self.states_restored:
                    self.states_restored = True
                    print(f"Sensors 1-3 restored: {self.sensors}")
                    # Republish to confirm our state
                    self.publish_states()

        except Exception as e:
            print(f"Error processing message: {e}")

    def on_disconnect(self, client, userdata, rc):
        self.status_label.config(text="⚠️ Disconnected", fg="red")
        if rc != 0:
            print("Unexpected disconnection, reconnecting...")

    def connect_mqtt(self):
        try:
            self.client.connect(self.broker, self.port, 60)
            thread = threading.Thread(target=self.client.loop_forever, daemon=True)
            thread.start()
        except Exception as e:
            self.status_label.config(text=f"✗ Error: {str(e)}", fg="red")

    def publish_states(self):
        message = json.dumps(self.sensors)
        # QoS 2 = exactly once, retain=True to keep the latest message
        result = self.client.publish(self.topic_publish, message, qos=2, retain=True)
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"States published to {self.topic_publish}: {message}")

    def on_closing(self):
        self.client.disconnect()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = Carte1Monitor(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
