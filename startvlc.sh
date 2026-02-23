#!/bin/bash
defaultVol=10
sleep 15
export DISPLAY=:0
export WAYLAND_DISPLAY=wayland-0
export XDG_RUNTIME_DIR=/run/user/1000
amixer set PCM $defaultVol%
taskset -c 2 cvlc --loop /home/upl/train.m4a&
sleep 5
taskset -c 2 vlc --loop --fullscreen /home/upl/train.mp4&
sleep 5
inc=1
loopCount=$defaultVol
cd /home/upl/notcursesTesting
sleep 15
taskset -c 3 foot --window-size-pixels=1200x500 bash -c "/home/upl/notcursesTesting/myapp; exec bash" &
while true; do
	echo "$inc"
	echo "$loopCount"
	if [[ "$inc" -eq 1 && "$loopCount" -lt 60 ]]; then
		sleep 10
		loopCount=$(($loopCount + 1))
		amixer set PCM ${loopCount}%
	elif [[ "$inc" -eq 0 && "$loopCount" -gt $defaultVol ]]; then
		sleep 10
		loopCount=$(($loopCount - 1))
		amixer set PCM ${loopCount}%
	elif [ "$loopCount" -ge 60 ]; then
		inc=0
	else
		inc=1
		taskset -c 3 foot --window-size-pixels=1200x500 bash -c "/home/upl/notcursesTesting/myapp; exec bash" &
		sleep 1200
	fi
done

