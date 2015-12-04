import subprocess
import threading


def genHosts(hosts, base, M):
    for i in range(1, M):
       if i < 10:
          host = base + "0" + str(i)
       else:
          host = base + str(i)
       hosts.append(host)

def clean(host):
    subprocess.call(['ssh', host, 'pkill','-u','sturdeva'])

hosts = []
threads = []

genHosts(hosts, 'galapagos-', 31)
genHosts(hosts, 'king-', 11)
#genHosts(hosts, 'adelie-', 8)
#genHosts(hosts, 'macaroni-', 10)
for host in hosts:
    thread = threading.Thread(target=clean, args=[host])
    thread.start()
    threads.append(thread)

for thread in threads:
    thread.join()
