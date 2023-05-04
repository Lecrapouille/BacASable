import zmq

# create a zmq subscriber socket
context = zmq.Context()
socket = context.socket(zmq.SUB)

# connect the socket to the publisher
socket.connect("tcp://localhost:5555")

# subscribe to all messages
socket.subscribe("")

# receive and print messages
while True:
    message = socket.recv()
    print(f"Received: {message.decode()}")
