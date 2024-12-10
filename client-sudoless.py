import socket
import requests
import threading
import sys
from cfg import *
import time


def get_peers():
    return requests.get(f"http://{RELAY_SERVER_IP}:{RELAY_SERVER_HTTP_PORT}").content.decode().split("\n")

def init_connection():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    s.bind((sys.argv[1], reverse_endianness(SRC_PORT)))
    #send empty payload
    s.sendto(b'Placeholder payload for discovery', (RELAY_SERVER_IP, RELAY_SERVER_PORT))
    return s

def reverse_endianness(n : int) -> int:
    return int.from_bytes(n.to_bytes(2), "little")



def recv_loop(s : socket.socket):
    while(True):
        data = s.recvfrom(1024)
        print("Recieved: " + data[0].decode())


def main():
    sock = init_connection()
    threading.Thread(target=recv_loop, args=(sock,),daemon=True).start()

    while(True):
        #ping
        sock.sendto(b'Placeholder payload for discovery', (RELAY_SERVER_IP, RELAY_SERVER_PORT))
        peers = get_peers()
        print(peers)
        
        time.sleep(1000)
        



if __name__ == "__main__":
    main()