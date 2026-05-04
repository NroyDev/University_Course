import socket
import struct
import os

server_IP = "localhost"
server_port = 18787
BUF_SIZE = 4096
PATH = os.path.join(".", "client_dir")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.connect((server_IP, server_port))

while True:
    usr_input = input().split()
    cmd = usr_input[0]
    if(cmd == "list"):
        header_data = struct.pack("!HBI", 0x4D46, 0x01, 0)
        server.send(header_data)

        data = b""
        while(len(data)<7):
            data += server.recv(BUF_SIZE)
        header_data = data[:7]
        if(len(data)>=7):
            data = data[7:]
        else:
            data = b""
        magic, cmd, length = struct.unpack("!HBI", header_data)
        while(len(data) < length):
            data += server.recv(BUF_SIZE)
        print(data.decode(), end='')
    elif(cmd == "get"):
        if(len(usr_input)<2):
            print("ERROR")
            continue
        arg = usr_input[1]
        print("debug",arg)
        header_data = struct.pack("!HBI", 0x4D46, 0x02, len(arg))
        server.send(header_data)
        server.send(arg.encode())
        
        data = b""
        while(len(data)<7):
            data += server.recv(BUF_SIZE)
        header_data = data[:7]
        if(len(data)>=7):
            data = data[7:]
        else:
            data = b""
        magic, cmd, length = struct.unpack("!HBI", header_data)
        while(len(data) < length):
            data += server.recv(BUF_SIZE)

        file_path = os.path.join(PATH, arg)
        with open(file_path, "wb") as file:
            file.write(data)
    elif(cmd == "quit"):
        header_data = struct.pack("!HBI", 0x4046, 0x03, 0)
        server.send(header_data)
        break
    else:
        print(f"ERROR: {cmd} is not a vaild command")

print("End of program...")
