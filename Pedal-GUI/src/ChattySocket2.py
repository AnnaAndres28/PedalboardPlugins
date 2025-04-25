import socket
import sys
import threading

def receive_feedback(sock):
    """
    Continuously receive messages from the same socket.
    Runs in a separate thread.
    """
    while True:
        try:
            response = sock.recv(1024).decode()
            if not response:  # Connection closed by server
                print("\nConnection closed by server.")
                break
            print(f"\nReceived feedback: {response}", flush=True)
            print("Enter message (q to quit): ", end='', flush=True)  # Reprint prompt
        except Exception as e:
            print(f"\nError receiving feedback: {e}")
            break

def create_socket_client():
    # Get connection details from user
    host = input("Enter the IP address to connect to: ")
    try:
        port = int(input("Enter the port number: "))
    except ValueError:
        print("Port must be a number")
        sys.exit(1)

    # Create a single socket for both sending and receiving
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect to the server
        sock.connect((host, port))
        print(f"Connected to {host}:{port}")

        # Start the feedback receiving thread
        feedback_thread = threading.Thread(target=receive_feedback, args=(sock,), daemon=True)
        feedback_thread.start()

        # Main loop for sending messages
        while True:
            message = input("Enter message (q to quit): ")
            
            if message.lower() == 'q':
                break

            sock.send(message.encode())

    except ConnectionRefusedError:
        print("Connection failed - server might be offline or wrong IP/port")
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        # Clean up the socket
        sock.close()
        print("\nConnection closed")

if __name__ == "__main__":
    create_socket_client()