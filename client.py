from scapy.all import *
import socket
import requests
import threading

RELAY_SERVER_IP = "127.0.0.1"
RELAY_SERVER_PORT = 13370
RELAY_SERVER_HTTP_PORT = 13371
RELAY_DST_PORT = 6667

SRC_PORT = 7777



def get_peers():
    return requests.get(f"http://{RELAY_SERVER_IP}:{RELAY_SERVER_HTTP_PORT}").content.decode().split("\n")

def init_connection():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    s.bind((sys.argv[1], reverse_endianness(SRC_PORT)))
    #send empty payload
    s.sendto(b'', (RELAY_SERVER_IP, RELAY_SERVER_PORT))
    return s

def reverse_endianness(n : int) -> int:
    return int.from_bytes(n.to_bytes(2), "little")

def send_spoofed(remote_peer_ip, remote_peer_port, payload : bytes):
    #somehow dport works properly? idk
    remote_peer_port = reverse_endianness(remote_peer_port)
    packet = IP(src=remote_peer_ip, dst=RELAY_SERVER_IP)/UDP(sport=remote_peer_port, dport=RELAY_SERVER_PORT)/Raw(load=payload)
    send(packet)


def recv_loop(s : socket.socket):
    while(True):
        data = s.recvfrom(1024)
        print("Recieved: ")
        print(data[0])
        print("\n")




def main():
    sock = init_connection()
    threading.Thread(target=recv_loop, args=(sock,),daemon=True).start()

    while(True):
        peers = get_peers()
        
        payload = "hello :D"

        for peer in peers:
            payload = input(f"Enter payload for {peer}: ")
            if(len(payload) == 0):
                continue
            peer = peer.split(":")
            send_spoofed(peer[0], int(peer[1]), payload)
    




if __name__ == "__main__":
    main()