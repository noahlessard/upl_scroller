#!/bin/bash
defaultVol=35
quietVol=8
BLANKED=0
export DISPLAY=:0 # TODO: check if we still need these
unset WAYLAND_DISPLAY          # force mpv into X11/EGL mode so overlay-add works
export XDG_RUNTIME_DIR=/run/user/1000
cd /home/upl/upl_scroller

# Get default HDMI ID from Sink ( should be set manually with Pi GUI once at boot )
if [[ -z "$WPCTL_HDMI_ID" ]]; then
    WPCTL_HDMI_ID=$(wpctl status | awk '/Sinks:/' | grep -i 'hdmi' | grep -oE '[0-9]+\.' | tr -d '.' | head -1)
fi

is_active_hours() {
    current_hour=$(date +%H)
    if [[ $current_hour -ge 10 ]] || [[ $current_hour -lt 1 ]]; then
        return 0  # true - active hours
    else
        return 1  # false - blank hours
    fi
}

# Control screen and audio with CEC over HDMI to TV
screen_blank() {
    echo "standby 0" | cec-client -s -d 1
    BLANKED=1
}

screen_unblank() {
    {
        echo "on 0"
        sleep 3
        echo "as"
    } | cec-client -s -d 1
    BLANKED=0
}

vol_down_full() {
    { for i in $(seq 50); do echo "voldown"; done; echo "quit"; } | cec-client -d 1
}

vol_up_full() {
    { for i in $(seq 50); do echo "volup"; done; echo "quit"; } | cec-client -d 1
}

vol_up_slow() {
    { for i in $(seq 50); do echo "volup"; sleep 1; done; echo "quit"; } | cec-client -d 1
}

vol_down_slow() {
    { for i in $(seq 50); do echo "voldown"; sleep 1; done; echo "quit"; } | cec-client -d 1
}

screen_unblank
vol_down_full

# Start audio loop
taskset -c 2 mpv --loop --no-video /media/upl/W/train.m4a &

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


# Main scheduling loop
while true; do

    if is_active_hours; then

        # Active hours — ensure screen is on
        if [[ $BLANKED -eq 1 ]]; then
            screen_unblank
        fi

        # Volume ramp cycle: quietVol -> defaultVol -> quietVol
        vol_up_slow
        sleep 10
        vol_down_slow

        # Full ramp cycle complete — sleep random 1-3 hours, checking each minute
        sleep_seconds=$(( (RANDOM % 7200) + 3600 ))
        while [[ $sleep_seconds -gt 0 ]]; do
            sleep 60
            sleep_seconds=$((sleep_seconds - 60))
            if ! is_active_hours; then
                break
            fi
        done
    else
        if [[ $BLANKED -eq 0 ]]; then
            screen_blank
        fi
    fi
done
