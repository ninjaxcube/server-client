import socket
import struct
import sys

def send_message(sock: socket.socket, id: int, data: str):
    data_bytes = data.encode('utf-8')
    length = len(data_bytes)
    header = struct.pack('!II', id, length)
    sock.sendall(header + data_bytes)

def __main__():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <server_ip> <server_port>")
        sys.exit(1)

    server_ip = sys.argv[1]
    server_port = int(sys.argv[2])

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((server_ip, server_port))
        message = input("Enter message to send: ")
        while message.strip() != "":
            send_message(sock, 1, message)
            message = input("Enter message to send: ")

if __name__ == "__main__":
    __main__()