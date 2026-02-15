# REQUISITES:
## 1. pip install paho-mqtt flask
## 2. sudo apt install mosquitto mosquitto-clients
## 3. ./server/broker.sh
# If running through python virtual environment (needed for WSL testing):
## 4. source venv/bin/activate

import json
from flask import Flask, jsonify, render_template
import paho.mqtt.client as mqtt
from collections import defaultdict
import os

app = Flask(__name__)

# JSON file to persist scores
SCORES_FILE = "scores.json"

# For each game there is a list of scores
scores_by_game = defaultdict(list)

# Load previous scores if the file exists
if os.path.exists(SCORES_FILE):
    try:
        with open(SCORES_FILE, "r") as f:
            data = json.load(f)
            for game, entries in data.items():
                scores_by_game[game].extend(entries)
        print(f"Loaded previous scores from {SCORES_FILE}")
    except Exception as e:
        print("Error loading scores.json:", e)

# Function to save and sort scores to JSON file
def save_scores():
    try:
        # Sort each game's list so highest scores are first
        for game in scores_by_game:
            scores_by_game[game].sort(key=lambda x: x["score"], reverse=True)

        with open(SCORES_FILE, "w") as f:
            json.dump(scores_by_game, f, indent=2)
    except Exception as e:
        print("Error saving scores to JSON:", e)

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    client.subscribe("fpga/score")

# MQTT callback when receiving a message
def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())

        game = data.get("game", "unknown")
        player = data.get("player", "unknown")
        score = data.get("score", 0)

        scores_by_game[game].append({"player": player, "score": score})
        save_scores()  # This sorts and writes to JSON
    except Exception as e:
        print("Error processing MQTT message:", e)

# MQTT client
## MQTT broker is assumed to be running locally in the background, on default port 1883
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect("localhost", 1883, 60)
mqtt_client.loop_start()

# Flask routes
## Server static HTML page
@app.route("/")
def index():
    return render_template("index.html")

## API endpoint to get scores by game
@app.route("/scores")
def get_scores():
    return jsonify(scores_by_game)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
