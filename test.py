import serial.tools.list_ports
import os

os.chdir("C:\\Users\\Krzysiu\\OneDrive\\Desktop\\kod_do_frezarki\\gcode_pliki")

serial.Timeout(10)

ports = serial.tools.list_ports.comports()
known={"USB VID:PID=2E8A:F00A SER=E6616408432C9035 LOCATION=1-2":"Rasberry Pi pico 2"}
if not ports:
    print("brak portow")
else:
    for i in range(len(ports)):
        if(ports[i].hwid not in known):
            print(f"{i+1}) {ports[i].device} - nieznane urządzenie . . . . {ports[i].hwid}")
        else:
            print(f"{i+1}) {ports[i].device} - {known[ports[i].hwid]}")
selected=int(input("wybierz port: "))-1
ser = serial.Serial(
    port=ports[selected].device,
    baudrate=115200,
    timeout=None,
    write_timeout=5
)

if ser.is_open:
    print(f"Port {ser.name} jest otwarty.")

files=os.listdir()
print("Pliki:")
for i in range(len(files)):
    if(files[i]!="Gcodereader"):
        print(f"{i+1}) {files[i]}")

chosen=int(input("Wybierz plik: "))
print(f'plik "{files[chosen-1]}" wybrany')

recorded=""
with open(files[chosen-1],"r") as opened:
    recorded=input()
    ser.write(recorded.encode())
    print(ser.readline().decode())
    while(line:=opened.readline()):
        if(line[0]=='G'):
            ser.write(line.encode())
            print(f"sent - {line.strip()}")
            recived = ser.readline().decode().strip()
            while(recived!=line.strip()):
                print(f"recived - {recived}")
                recived = ser.readline().decode().strip()
            print(f"recived - {recived}")
            print("sent - ok")
            ser.write(b"ok")
            recived = ser.readline().decode().strip()
            while (recived != "gotowe"):
                print(f"recived - {recived}")
                recived = ser.readline().decode().strip()
            print(f"recived - {recived}\n")