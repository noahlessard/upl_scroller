#!/bin/bash
defaultVol=10
BLANKED=0
sleep 15
export DISPLAY=:0
export WAYLAND_DISPLAY=wayland-0
export XDG_RUNTIME_DIR=/run/user/1000

# Function to check if current time is within active hours (7PM-3AM)
is_active_hours() {
    current_hour=$(date +%H)
    if [[ $current_hour -ge 19 ]] || [[ $current_hour -lt 3 ]]; then
        return 0  # true - active hours
    else
        return 1  # false - blank hours
    fi
}

# Function to blank the screen and mute audio
screen_blank() {
    vcgencmd display_power 0
    amixer set PCM 0%
    BLANKED=1
}

# Function to unblank the screen and unmute audio
screen_unblank() {
    vcgencmd display_power 1
    amixer set PCM $defaultVol%
    BLANKED=0
}

# Initialize: unblank screen and unmute audio
screen_unblank

# Start audio loop
taskset -c 2 cvlc --loop /home/upl/train.m4a &

sleep 5

# Start MPV fullscreen with IPC socket for overlay-add support
# --hwdec=auto  : use Pi hardware video decoder (saves CPU)
# --no-terminal : suppress terminal output
taskset -c 2 mpv --loop --fullscreen --no-terminal \
    --hwdec=auto \
    --input-ipc-server=/tmp/mpvsock \
    /home/upl/train.mp4 &

sleep 5

# Start the overlay app (connects to MPV socket, draws via overlay-add)
cd /home/upl/notcursesTesting
sleep 10
taskset -c 3 /home/upl/notcursesTesting/upl_scroller &

# Sleep for 2 minutes (testing window)
sleep 120

# Main scheduling loop
while true; do
    if is_active_hours; then
        # Currently active hours
        if [[ $BLANKED -eq 1 ]]; then
            screen_unblank
        fi
        # Volume ramp logic
        inc=1
        loopCount=$defaultVol
        while true; do
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
                # Volume back to minimum — restart overlay app for next cycle
                inc=1
                taskset -c 3 /home/upl/notcursesTesting/upl_scroller &
                sleep 1200
            fi
        done
    else
        # Currently blank hours
        if [[ $BLANKED -eq 0 ]]; then
            screen_blank
        fi
    fi
    sleep 60
done
