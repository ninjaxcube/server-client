import socket
import struct
import sys
import cmd

def craft_message(id: int, data: str):
    data_bytes = data.encode('utf-8')
    data_length = len(data_bytes)
    fmt = f'!II{data_length}s'
    return struct.pack(fmt, id, data_length, data_bytes)

def recieve_message(sock: socket.socket):
    header_size = struct.calcsize('!II')
    header = sock.recv(header_size)
    print(f"header response: {header}")
    


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

    def do_message(self, data):
        msg = craft_message(1, data)
        self.server_socket.send(msg)
        recieve_message(self.server_socket)

    def do_exit(self, arg):
        print("Shutting down..")
        sys.exit(0)

    def emptyline(self):
        pass

    def postloop(self):
        self.server_socket.close()

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
            print("Connection refused. Try reconnecting when the server is ready..")

        client(server_ip, server_port, sock).cmdloop()

if __name__ == "__main__":
    __main__()