import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt
import json
import threading

class card2Monitor:
    def __init__(self, root):
        self.root = root
        self.root.title("Board 2 - 3 Sensors Monitor")
        self.root.geometry("400x450")
        
        # Local sensor states (True = OK, False = KO)
        self.local_sensors = {
            "sensor_4": True,
            "sensor_5": True,
            "sensor_6": True
        }
        
        # States of Board 1 sensors (received via MQTT)
        self.card1_sensors = {
            "sensor_1": None,
            "sensor_2": None,
            "sensor_3": None
        }

        # Flag to track if local states have been restored from retained message
        self.states_restored = False
        
        # MQTT Configuration - Connect to Broker 2 (port 1884)
        self.client = mqtt.Client(client_id="card2_monitor", clean_session=False)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        
        self.broker = "localhost"
        self.port = 1884  # Broker 2
        self.topic_card1 = "sensors/card1"  # Received via bridge from Broker 1
        self.topic_all = "sensors/all"  # Published to Broker 2, also used for restoration
        
        self.setup_gui()
        self.connect_mqtt()
        
    def setup_gui(self):
        # Title
        title = tk.Label(self.root, text="🔌 Board 2 - Sensors", 
                        font=("Arial", 16, "bold"), pady=10)
        title.pack()
        
        # Connection Status
        self.status_label = tk.Label(self.root, text="⚠️ Disconnected", 
                                     fg="red", font=("Arial", 10))
        self.status_label.pack(pady=5)
        
        # Frame for local sensors
        frame_local = ttk.LabelFrame(self.root, text="Local Sensors (4-6)", padding=10)
        frame_local.pack(padx=20, pady=5, fill="both", expand=True)
        
        self.buttons_local = {}
        for i, (sensor, state) in enumerate(self.local_sensors.items(), 4):
            row_frame = tk.Frame(frame_local)
            row_frame.pack(pady=5, fill="x")
            
            label = tk.Label(row_frame, text=f"Sensor {i}:", 
                           font=("Arial", 11), width=12, anchor="w")
            label.pack(side="left", padx=5)
            
            btn = tk.Button(row_frame, text="✓ OK", bg="#4CAF50", fg="white",
                          font=("Arial", 10, "bold"), width=12,
                          command=lambda s=sensor: self.toggle_sensor(s))
            btn.pack(side="left", padx=5)
            self.buttons_local[sensor] = btn
        
        # Frame for received sensors
        frame_remote = ttk.LabelFrame(self.root, text="Board 1 Sensors (1-3)", padding=10)
        frame_remote.pack(padx=20, pady=5, fill="both", expand=True)
        
        self.labels_remote = {}
        for i, sensor in enumerate(self.card1_sensors.keys(), 1):
            row_frame = tk.Frame(frame_remote)
            row_frame.pack(pady=5, fill="x")
            
            label = tk.Label(row_frame, text=f"Sensor {i}:", 
                           font=("Arial", 11), width=12, anchor="w")
            label.pack(side="left", padx=5)
            
            status = tk.Label(row_frame, text="⚪ Unknown", bg="#9E9E9E", fg="white",
                            font=("Arial", 10, "bold"), width=12)
            status.pack(side="left", padx=5)
            self.labels_remote[sensor] = status
        
        # Publish button
        publish_btn = tk.Button(self.root, text="📤 Publish All States", 
                               command=self.publish_all_states,
                               bg="#2196F3", fg="white", 
                               font=("Arial", 11, "bold"), pady=8)
        publish_btn.pack(pady=10, padx=20, fill="x")
        
        # QoS Info
        info = tk.Label(self.root, text="QoS 2 - Retained messages - Broker 2 (1884)", 
                       font=("Arial", 9), fg="gray")
        info.pack()
        
    def toggle_sensor(self, sensor):
        self.local_sensors[sensor] = not self.local_sensors[sensor]
        self.update_button(sensor)
        self.publish_all_states()
        
    def update_button(self, sensor):
        btn = self.buttons_local[sensor]
        if self.local_sensors[sensor]:
            btn.config(text="✓ OK", bg="#4CAF50")
        else:
            btn.config(text="✗ KO", bg="#f44336")
            
    def update_remote_label(self, sensor, state):
        label = self.labels_remote[sensor]
        if state is True:
            label.config(text="✓ OK", bg="#4CAF50")
        elif state is False:
            label.config(text="✗ KO", bg="#f44336")
        else:
            label.config(text="⚪ Unknown", bg="#9E9E9E")
            
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.status_label.config(text="✓ Connected to Broker 2", fg="green")
            # Subscribe to Board 1's topic (received via bridge) with QoS 2
            self.client.subscribe(self.topic_card1, qos=2)
            # Subscribe to sensors/all for local state restoration
            self.client.subscribe(self.topic_all, qos=2)
            # Schedule initial publish after allowing time for restoration
            self.root.after(500, self.initial_publish)
        else:
            self.status_label.config(text=f"✗ Connection error: {rc}", fg="red")

    def initial_publish(self):
        """Publish states after connection, whether restored or not"""
        self.publish_all_states()
            
    def on_message(self, client, userdata, msg):
        try:
            data = json.loads(msg.payload.decode())
            
            if msg.topic == self.topic_card1:
                # Received Board 1 sensor states via bridge
                print(f"Received states from Board 1: {data}")
                for sensor in self.card1_sensors.keys():
                    if sensor in data:
                        self.card1_sensors[sensor] = data[sensor]
                        self.update_remote_label(sensor, data[sensor])
                # Publish combined states
                self.publish_all_states()
                
            elif msg.topic == self.topic_all:
                # Restore local sensors 4-6 from retained message
                if not self.states_restored:
                    print(f"Received sensors/all for restoration: {data}")
                    restored = False
                    for sensor in self.local_sensors.keys():
                        if sensor in data and data[sensor] is not None:
                            self.local_sensors[sensor] = data[sensor]
                            self.update_button(sensor)
                            restored = True
                    
                    if restored:
                        self.states_restored = True
                        print(f"Sensors 4-6 restored: {self.local_sensors}")

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
            
    def publish_all_states(self):
        # Publish all states (local + received) for display and restoration
        all_states = {**self.card1_sensors, **self.local_sensors}
        message = json.dumps(all_states)
        result = self.client.publish(self.topic_all, message, qos=2, retain=True)
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"All states published to {self.topic_all}: {message}")
        
    def on_closing(self):
        self.client.disconnect()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = card2Monitor(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
