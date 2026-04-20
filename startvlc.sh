#!/bin/bash
defaultVol=10
quietVol=3
BLANKED=0
WPCTL_HDMI_ID=
export DISPLAY=:0 # TODO: check if we still need these
unset WAYLAND_DISPLAY          # force mpv into X11/EGL mode so overlay-add works
export XDG_RUNTIME_DIR=/run/user/1000
cd /home/upl/upl_scroller

if [[ -z "$WPCTL_HDMI_ID" ]]; then
    WPCTL_HDMI_ID=$(wpctl status | grep -i 'hdmi' | grep -oE '\b[0-9]+\.' | tr -d '.' | head -1)
    if [[ -n "$WPCTL_HDMI_ID" ]]; then
        wpctl set-default $WPCTL_HDMI_ID
    else
        echo "Warning: HDMI audio sink not found, audio may not route correctly"
    fi
fi

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
    wpctl set-mute $WPCTL_HDMI_ID 1
    BLANKED=1
}

# Function to unblank the screen and unmute audio
screen_unblank() {
    vcgencmd display_power 1
    wpctl set-mute $WPCTL_HDMI_ID 0
    wpctl set-volume $WPCTL_HDMI_ID ${defaultVol}%
    BLANKED=0
}

# Initialize: unblank screen and unmute audio
screen_unblank

# Start audio loop
taskset -c 2 mpv --loop --no-video bing.mp3 &

# Start MPV fullscreen with IPC socket for overlay-add support
rm -f /tmp/mpvsock
taskset -c 2 nice -n 19 mpv --loop --fullscreen --no-terminal \
    --hwdec=auto \
    --geometry=800x600 \
    --keepaspect=no \
    --input-ipc-server=/tmp/mpvsock \
    /media/upl/W/train.mp4 \
    >/tmp/mpv.log 2>&1 &


# Start the overlay app (connects to MPV socket, draws via overlay-add)
sleep 200
taskset -c 3 /home/upl/upl_scroller/upl_scroller &

# Testing: ramp down to quiet, hold 2 minutes, ramp back to normal
for vol in $(seq $defaultVol -1 $quietVol); do
    wpctl set-volume $WPCTL_HDMI_ID ${vol}%
    sleep 10
done

sleep 10

for vol in $(seq $quietVol 1 $defaultVol); do
    wpctl set-volume $WPCTL_HDMI_ID ${vol}%
    sleep 10
done

# # Main scheduling loop
# while true; do
#     if is_active_hours; then
#         # Currently active hours
#         if [[ $BLANKED -eq 1 ]]; then
#             screen_unblank
#         fi
#         # Volume ramp logic
#         inc=1
#         loopCount=$defaultVol
#         while true; do
#             if [[ "$inc" -eq 1 && "$loopCount" -lt 60 ]]; then
#                 sleep 10
#                 loopCount=$(($loopCount + 1))
#                 wpctl set-volume $WPCTL_HDMI_ID ${loopCount}%
#             elif [[ "$inc" -eq 0 && "$loopCount" -gt $defaultVol ]]; then
#                 sleep 10
#                 loopCount=$(($loopCount - 1))
#                 wpctl set-volume $WPCTL_HDMI_ID ${loopCount}%
#             elif [ "$loopCount" -ge 60 ]; then
#                 inc=0
#             else
#                 # Volume back to minimum — restart overlay app for next cycle
#                 inc=1
#                 taskset -c 3 /home/upl/notcursesTesting/upl_scroller &
#                 sleep 1200
#             fi
#         done
#     else
#         # Currently blank hours
#         if [[ $BLANKED -eq 0 ]]; then
#             screen_blank
#         fi
#     fi
#     sleep 60
# done
