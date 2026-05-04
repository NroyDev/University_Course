import socket
import struct
import os

server_IP = "localhost"
server_port = 18787
BUF_SIZE = 4096
PATH = os.path.join(".","server_dir")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((server_IP, server_port))
server.listen(5)
print(f"Listening on {server_IP}:{server_port}")

client, addr = server.accept()
print(f"Connected by {addr}")
try:
    while True:
        # ---------------- Recv Header ----------------
        data = b""
        while(len(data)<7):
            data += client.recv(BUF_SIZE)
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
        if(magic != 0x4D46):
            print("[Warning] the magic number not matched")
        if(not (cmd == 0x01 or cmd == 0x02 or cmd == 0x03)):
            print(f"[Warning] the cmd is {cmd}, which is expected in the range of 0x01~0x03")
        print()

        # ---------------- Response cmd ----------------
        if(cmd == 0x01):    # list: 0x01
            # get dir list
            data = b""
            file_lists = os.listdir(PATH)
            for filename in file_lists:
                data += f"{filename}\n".encode()

            # sent to client
            length = len(data)
            header_data = struct.pack("!HBI", 0x4D46, 0x04, length)
            client.send(header_data)
            client.send(data)
        elif(cmd == 0x02):  # get: 0x02
            # get the remaining part of filename
            while(len(data) < length):
                data += client.recv(BUF_SIZE)
            filename = data.decode()
            print(f"Required file: {filename}")

            # get the file binary data
            file_data = b""
            try:
                with open(os.path.join(PATH, filename), "rb") as file:
                    file_data = file.read()
            except Exception as e:
                file_data = f"ERROR: {e}".encode()
            
            # sent to client
            length = len(file_data)
            header_data = struct.pack("!HBI", 0x4D46, 0x04, length)
            client.send(header_data)
            client.send(file_data)
        elif(cmd == 0x03):  #quit: 0x03
            break
        else:
            print(f"ERORR: {cmd} is an invalid cmd")
except Exception as e:
    print(f"[ERROR] {e}")
finally:
    client.close()
    server.close()
    print("Socket closed successfully")

print("END OF PROGRAM...")
