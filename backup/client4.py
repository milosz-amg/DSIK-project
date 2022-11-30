import socket
import signal
import sys

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 7777
BUFSIZE = 512

s = socket.socket()
print(f"[*] Connecting to {SERVER_HOST}:{SERVER_PORT}...")
s.connect((SERVER_HOST, SERVER_PORT))
print("[+] Connected.")

username = input("Enter your name: ")
hostname=socket.gethostname()
IPAddr=socket.gethostbyname(hostname)


def recFile(filename):
    print("file reciving...")

    received=0
    buf=s.recv(BUFSIZE).decode()
    buf=buf.strip().strip('\x00')
    size_num = int(buf)
    f=open(filename,"wb")
    while received<size_num:
        recbuf = s.recv(BUFSIZE)    #bez decode przesyla inne pliki niz tylko png
        recbuf
        f.write(recbuf)
        received=received+BUFSIZE
    f.close()
    print("file",filename,"received")

def recMessage():
    buf=s.recv(BUFSIZE)


while True:
        msg = input()
        if msg == 'EXIT':
            break

        elif msg.startswith(">>"):
            s.send(msg.encode())
            msg.strip('>')
            filename=msg.replace(">","")
            recFile(filename)

        else:
            msg = f"[{IPAddr}][{hostname}][{username}]: {msg}"            
            s.send(msg.encode())

s.close()
