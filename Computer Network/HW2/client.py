import socket
import struct
import os

server_IP = "localhost"
server_port = 18787
BUF_SIZE = 4096
PATH = os.path.join(".", "client_dir")

def recv_fulldata(socket: socket.socket):
    data = b""
    while(len(data)<7):
        data += socket.recv(BUF_SIZE)
    header_data = data[:7]
    if(len(data)>=7):
        data = data[7:]
    else:
        data = b""
    magic, cmd, length = struct.unpack("!HBI", header_data)
    print("------------[Header]------------")
    print(f"Received Header: {header_data}")
    print(f"received magic# {magic}")
    print(f"cmd: {cmd}")
    print(f"data length: {length}")
    print("------------[Result]------------")
    if(magic != 0x4D46):
        print("[Warning] the magic number not matched")
    if(cmd != 0x04):    # server should only use this cmd
        print(f"[Warning] the cmd is {cmd}, which is expected as 0x04 (data)")
    while(len(data) < length):
        data += socket.recv(BUF_SIZE)

    return data

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.connect((server_IP, server_port))
try:
    while True:
        usr_input = input("> ").split()
        cmd = usr_input[0]
        if(cmd == "list"):
            header_data = struct.pack("!HBI", 0x4D46, 0x01, 0)
            server.send(header_data)
            data = recv_fulldata(socket=server)
            print(data.decode(), end='')
        elif(cmd == "get"):
            if(len(usr_input)<2):
                print("ERROR")
                continue
            arg = usr_input[1]
            header_data = struct.pack("!HBI", 0x4D46, 0x02, len(arg))
            server.send(header_data)
            server.send(arg.encode())
            
            data = recv_fulldata(socket=server)
            file_path = os.path.join(PATH, arg)
            with open(file_path, "wb") as file:
                file.write(data)
            print(f"file successfully saved to {file_path}")
        elif(cmd == "quit"):
            header_data = struct.pack("!HBI", 0x4D46, 0x03, 0)
            server.send(header_data)
            break
        else:
            print(f"[ERROR] {cmd} is not an vaild command")
except Exception as e:
    print(f"[ERROR] {e}")
finally:
    server.close()
    print("Socket closed successfully")

print("END OF PROGRAM...")
