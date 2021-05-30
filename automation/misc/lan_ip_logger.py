import datetime
import subprocess

ip = subprocess.getoutput("ipconfig").split('Wireless LAN adapter Wi-Fi:\n\n')[1].split('\n\n')[0].split('IPv4 Address')[1].split('\n')[0].split(': ')[1]

time = datetime.datetime.now().strftime('%Y/%m/%d %I:%M:%S %p')

with open('ip.log','a') as f:
	f.write(time+' '+ip+'\n')

print(time+' '+ip+'\n')