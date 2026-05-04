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
        data = b""
        while(len(data)<7):
            data += client.recv(BUF_SIZE)
        header_data = data[:7]
        print(f"HEADER: {header_data}")
        if(len(data)>=7):
            data = data[7:]
        else:
            data = b""
        magic, cmd, length = struct.unpack("!HBI", header_data)

        if(cmd == 0x01):    # list: 0x01
            data = b""
            file_lists = os.listdir(PATH)
            for filename in file_lists:
                data += f"{filename}\n".encode()
        
            length = len(data)
            header_data = struct.pack("!HBI", 0x4D46, 0x04, length)
            client.send(header_data)
            client.send(data)
        elif(cmd == 0x02):  # get: 0x02
            data = b""
            while(len(data) < length):
                data += client.recv(BUF_SIZE)
            filename = data.decode()
            print(filename)
            file_data = b""
            try:
                with open(os.path.join(PATH, filename), "rb") as file:
                    file_data = file.read()
            except Exception as e:
                file_data = f"ERROR: {e}".encode()
            
            
            length = len(file_data)
            header_data = struct.pack("!HBI", 0x4D46, 0x04, length)
            client.send(header_data)
            client.send(file_data)
        elif(cmd == 0x03):  #quit: 0x03
            break
        else:
            print(f"ERORR: {cmd} is an invalid cmd")
except Exception as e:
    print(f"{e}")
finally:
    client.close()
    server.close()

print("END OF PROGRAM...")
