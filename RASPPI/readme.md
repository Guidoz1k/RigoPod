The raspberry pi 5 only runs a simple python script and is accessed via VNC (VNC Viewer)

for this project to run properly it needs the VCN, I²C and Serial Port enabled in the Interfaces UI


the python code runs as soon as the rasp boots, and for that:
my user is rigo, and I clone the repo to the desktop...

1 Create the autostart folder and file:

	bash

	mkdir -p ~/.config/autostart
	nano ~/.config/autostart/rigopod.desktop


2 Paste this exact configuration (we include bash -c and a short sleep 3 so the desktop
  and camera drivers finish initializing before your window opens)
  (lxterminal -e  for a terminal window to stay open to show print() messages):

	ini

	[Desktop Entry]
	Type=Application
	Name=RigoPod
	Path=/home/rigo/Desktop/RigoPod/RASPPI/
	Exec=lxterminal -e bash -c "sleep 3 && /usr/bin/python3 /home/rigo/Desktop/RigoPod/RASPPI/main.py"

3 Save and exit (Ctrl + X, then Y, then Enter).

4 Create the labwc config folder and open its autostart file:

	mkdir -p ~/.config/labwc
	nano ~/.config/labwc/autostart

5 Add this line at the bottom (it waits 3 seconds, runs your script, and saves any terminal output/errors
  into a log.txt file on your Desktop):

	lxterminal -e bash -c "sleep 3 && /usr/bin/python3 /home/rigo/Desktop/RigoPod/RASPPI/main.py; exec bash" &


6 Save and exit (Ctrl + X, then Y, then Enter).



