import socket
import struct
import sys
import cmd

class client(cmd.Cmd):
    intro = "Client to echo-server. Type '?' to list commands.\n"
    prompt = '(client) '

    server_ip = ''
    server_port = ''
    server_socket = None

    def __init__(self, ip, port, sock):
        super().__init__()
        self.server_ip = ip
        self.server_port = port
        self.server_socket = sock

    def do_message(self, sock: socket.socket, id: int, data: str):
        data_bytes = data.encode('utf-8')
        length = len(data_bytes)
        header = struct.pack('!II', id, length)
        sock.sendall(header + data_bytes)

    def do_disconnect(self, arg):
        pass

    def do_reconnect(self, arg):
        try:
            self.server_socket.connect((self.server_ip, self.server_port))
        except ConnectionRefusedError:
            print("Connection refused. Try using \"reconnect\" when the server is ready..")

    def do_exit(self, arg):
        print("Shutting down..")
        sys.exit(0)

def __main__():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <server_ip> <server_port>")
        sys.exit(1)

    server_ip = sys.argv[1]
    server_port = int(sys.argv[2])

    print(f"connecting to {server_ip} on port {server_port}")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            sock.connect((server_ip, server_port))
        except ConnectionRefusedError:
            print("Connection refused. Try using \"reconnect\" when the server is ready..")

        client(server_ip, server_port, sock).cmdloop()

if __name__ == "__main__":
    __main__()