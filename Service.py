import subprocess
import os
import sys
import hashlib
import signal

def sha256sum(filename):
    h = hashlib.sha256()
    with open(filename, 'rb', buffering=0) as f:
        for b in iter(lambda : f.read(128*1024), b''):
            h.update(b)
    return h.hexdigest()

def main():
    f = open("replica.config", "r")
    f_tolerent = int(f.readline().split()[1])
    client_num = int(f.readline().split()[1])
    print("The number of tolerated failures is ", f_tolerent)
    print("The number of client is ", client_num)
    replica_num = 2 * f_tolerent + 1

    server_processes = []
    client_processes = []
    CLIENT_PORT_BASE = 9000

    for idx in range(f_tolerent):
        command = "./Server/server " + str(idx)
        print(command)
        f = open("server_log_%s.txt" % (idx), 'w+')
        pro = subprocess.Popen(command, stdout=f, shell=True, preexec_fn=os.setsid) 
        server_processes.append(pro)
        # os.killpg(os.getpgid(pro.pid), signal.SIGTERM)
    
    for idx in range(client_num):
        command = "./Client/server %s %s"  % (str(idx), str(CLIENT_PORT_BASE + idx))
        print(command)
        f = open("client_log_%s.txt" % (idx), 'w+')
        pro = subprocess.Popen(command, stdout=f, shell=True, preexec_fn=os.setsid) 
        client_processes.append(pro)
        
    
    for line in sys.stdin:
        cmd = line.split()
        print(cmd)
        if cmd[0] == 'kill':
            target = cmd[1]
            if target == 's':
                id = cmd[2]
                if id == 'all':
                    killAll(server_processes)
                else:
                    id = int(id)
                    os.killpg(os.getpgid(server_processes[id].pid), signal.SIGTERM)
                    server_processes[id] = -1
            elif target == 'c':
                id = cmd[2]
                if id == 'all':
                    killAll(server_processes)
                else:
                    id = int(id)
                    os.killpg(os.getpgid(client_processes[id].pid), signal.SIGTERM)
                    client_processes[id] = -1
        elif cmd[0] == 'hash':
            for idx in range(f_tolerent):
                filename = "server_log_%s.txt" % (idx)
                print("Hash value for replica %s is %s" % (idx, sha256sum(filename)))
        elif cmd[0] == 'exit':
            killAll(server_processes)                    
            killAll(client_processes)
            exit(1)

def killAll(process):
    for pros in process:
        if pros != -1:
            os.killpg(os.getpgid(pros.pid), signal.SIGTERM)
    process = []
    
                
                
            
    


    




if __name__ == '__main__':
    main()