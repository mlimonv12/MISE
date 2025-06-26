import socket
import json
import time

# Define server settings
HOST = '0.0.0.0'  # Accept connections from any IP
PORT = 8082  # Listening port (updated from user's provided snippet)

# Robot connection settings
ROBOT_IP = "192.168.0.143"  # Updated from user's provided snippet
ROBOT_PORT = 80 # Updated from user's provided snippet

# Robot command mappings
comandos = {"LEDS RGB": 0x19, "Motores": 0x20}
identificadores = {"Left": 1, "Right": 2, "Both": 0xFE}
estados = {"OFF": 0, "FORWARD": 1, "BACKWARD": 2}

# Global variables for robot state
current_speed = 0  # 0-255 for motors, from the 'Speed' slider
last_command_sent_time = 0.0
COMMAND_INTERVAL_MS = 100 # Updated from user's provided snippet
COMMAND_INTERVAL_SEC = COMMAND_INTERVAL_MS / 1000.0

# Global variables to track last LED states to prevent redundant commands
last_left_led_color = -1 # Initialize with invalid value to ensure first command sends
last_right_led_color = -1

# Function to calculate checksum for Bioloid protocol
def checksum(packet_data):
    chksum = sum(packet_data)
    return (~chksum & 0xFF)

# Function to create a command packet for the robot
def create_command_packet(command_name, target_id_name, parameters):
    instruction = 0x03  # INSTR_WRITE for writing to registers
    command_byte = comandos[command_name]
    target_id_byte = identificadores[target_id_name]
    
    # Length: (Instruction byte) + (Command byte) + (num_parameters) + (checksum byte)
    length = 2 + len(parameters) + 1 
    
    packet_core = [target_id_byte, length, instruction, command_byte] + parameters
    
    chksum = checksum(packet_core)
    
    packet = bytearray([0xFF, 0xFF] + packet_core + [chksum])
    return packet

# Function to ensure minimum time between commands
def wait_for_next_command():
    global last_command_sent_time
    time_since_last_command = time.time() - last_command_sent_time
    if time_since_last_command < COMMAND_INTERVAL_SEC:
        time_to_wait = COMMAND_INTERVAL_SEC - time_since_last_command
        time.sleep(time_to_wait)
    last_command_sent_time = time.time()

# Create a TCP server (for Android Joypad)
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # Allow port reuse
server_socket.bind((HOST, PORT))
server_socket.listen()
server_socket.settimeout(1.0) # Set a timeout for server_socket.accept()
print(f"Server listening on {HOST}:{PORT}")

# Connect to the robot (as a client)
robot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    robot_socket.connect((ROBOT_IP, ROBOT_PORT))
    print(f"Connected to robot at {ROBOT_IP}:{ROBOT_PORT}")
except ConnectionRefusedError:
    print(f"ERROR: Could not connect to the robot at {ROBOT_IP}:{ROBOT_PORT}. Is it running?")
    server_socket.close()
    exit()
except Exception as e:
    print(f"An unexpected error occurred while connecting to the robot: {e}")
    server_socket.close()
    exit()

try:
    client_socket = None
    # Loop to continuously try accepting new client connections with timeout
    # This allows Ctrl+C to be caught even before a client connects
    while client_socket is None:
        try:
            print("Waiting for client connection...")
            client_socket, client_address = server_socket.accept()
            print(f"Accepted connection from {client_address}")
            client_socket.settimeout(1.0) # Set a timeout for client_socket.recv()
        except socket.timeout:
            # No client connected within the timeout, loop again
            continue
        except Exception as e:
            print(f"Error accepting client connection: {e}")
            break # Exit if other errors occur during accept (e.g., server_socket closed)

    if client_socket is None: # If we broke the loop without accepting a client (e.g., due to an error)
        raise KeyboardInterrupt # Simulate Ctrl+C to trigger graceful shutdown

    # Use a buffer to handle partial JSON messages
    buffer = ""

    while True:
        try:
            data = client_socket.recv(1024)
            if not data:
                print("Client disconnected.")
                break
            
            message_str = data.decode('utf-8')
            buffer += message_str

            # Try to parse all complete JSON objects in the buffer
            while True:
                try:
                    # Find the end of the first complete JSON object
                    idx = buffer.find('}{') 
                    if idx == -1:
                        # If no '}{' found, check for a single complete object
                        json_obj = json.loads(buffer)
                        message = buffer
                        buffer = "" # Clear buffer after successful parse
                    else:
                        # Extract the first complete JSON object
                        message = buffer[:idx+1]
                        json_obj = json.loads(message)
                        buffer = buffer[idx+1:] # Keep the rest in the buffer

                    # --- Process the JSON message ---
                    event_id = json_obj.get('id')
                    event_type = json_obj.get('type')

                    if event_id == 'Direction' and event_type == 'DPAD':
                        button = json_obj.get('button')
                        state = json_obj.get('state')
                        print(f"DPAD command: {button} {state}")

                        motor_command_packet = None
                        
                        if state == "PRESS":
                            if button == "UP":
                                # Both motors FORWARD
                                params = [current_speed, estados["FORWARD"], current_speed, estados["FORWARD"]]
                                motor_command_packet = create_command_packet("Motores", "Both", params)
                                print(f"Sending Both Motors FORWARD command: {motor_command_packet.hex()}")
                            elif button == "DOWN":
                                # Both motors BACKWARD
                                params = [current_speed, estados["BACKWARD"], current_speed, estados["BACKWARD"]]
                                motor_command_packet = create_command_packet("Motores", "Both", params)
                                print(f"Sending Both Motors BACKWARD command: {motor_command_packet.hex()}")
                            elif button == "LEFT":
                                # Only RIGHT motor FORWARD
                                params = [current_speed, estados["FORWARD"]]
                                motor_command_packet = create_command_packet("Motores", "Right", params)
                                print(f"Sending Right Motor FORWARD command (Left Turn): {motor_command_packet.hex()}")
                            elif button == "RIGHT":
                                # Only LEFT motor FORWARD
                                params = [current_speed, estados["FORWARD"]]
                                motor_command_packet = create_command_packet("Motores", "Left", params)
                                print(f"Sending Left Motor FORWARD command (Right Turn): {motor_command_packet.hex()}")
                        elif state == "RELEASE":
                            # On RELEASE, explicitly halt both motors using the "Both" command
                            params = [0, estados["OFF"], 0, estados["OFF"]]
                            motor_command_packet = create_command_packet("Motores", "Both", params)
                            print(f"Sending Both Motors STOP command: {motor_command_packet.hex()}")
                        
                        if motor_command_packet:
                            wait_for_next_command() # Apply rate limit before sending
                            try:
                                robot_socket.send(motor_command_packet)
                            except Exception as e:
                                print(f"Error sending motor command to robot: {e}")

                    elif event_id == 'Speed' and event_type == 'SLIDER':
                        value = json_obj.get('value', 0)
                        current_speed = int(value) # Convert float to int
                        current_speed = max(0, min(255, current_speed)) # Clamp to 0-255
                        print(f"Updated speed: {current_speed}")

                    elif (event_id == 'LEFT_LED' or event_id == 'RIGHT_LED') and event_type == 'SLIDER':
                        # Declare global variables BEFORE any usage or assignment in this block
                        #global last_left_led_color, last_right_led_color 

                        value = json_obj.get('value', 0)
                        led_color = int(value) # Convert float to int
                        led_color = max(0, min(7, led_color)) # Clamp to 0-7 (RGB values)

                        target_id = "Left" if event_id == 'LEFT_LED' else "Right"
                        
                        current_led_state = last_left_led_color if target_id == "Left" else last_right_led_color

                        if led_color != current_led_state: # Only send if color changed
                            led_command = create_command_packet("LEDS RGB", target_id, [led_color])
                            
                            print(f"Sending LED command ({event_id}): {led_command.hex()}")
                            wait_for_next_command() # Apply rate limit before sending
                            try:
                                robot_socket.send(led_command)
                                # Update the last sent color for this LED
                                if target_id == "Left":
                                    last_left_led_color = led_color
                                else:
                                    last_right_led_color = led_color
                            except Exception as e:
                                print(f"Error sending LED command to robot: {e}")

                except json.JSONDecodeError:
                    # Partial JSON or malformed JSON, break from inner loop to receive more data
                    break 
                except IndexError as e:
                    print(f"Error processing JSON: {e}. Message: {message}. Buffer: {buffer}")
                    buffer = "" # Clear buffer on unexpected error to prevent loop
                    break
                except Exception as e:
                    print(f"An unexpected error occurred during message processing: {e}")
                    buffer = "" # Clear buffer on unexpected error
                    break
        
        except socket.timeout:
            # No data received within timeout from client, continue loop
            continue
        except ConnectionResetError:
            print("Client forcibly closed the connection.")
            break
        except Exception as e:
            print(f"An error occurred during client communication: {e}")
            break

except KeyboardInterrupt:
    print("Ctrl+C detected. Shutting down server.")
except Exception as e:
    print(f"An unexpected error occurred in the main loop: {e}")
finally:
    if 'client_socket' in locals() and client_socket:
        client_socket.close()
    if server_socket:
        server_socket.close()
    if robot_socket:
        robot_socket.close()
    print("Sockets closed. Exiting.")