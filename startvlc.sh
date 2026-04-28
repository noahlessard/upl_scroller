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
    if [[ $current_hour -ge 19 ]] || [[ $current_hour -lt 3 ]]; then
        return 0  # true - active hours
    else
        return 1  # false - blank hours
    fi
}

screen_blank() {
    vcgencmd display_power 0
    BLANKED=1
}

screen_unblank() {
    vcgencmd display_power 1
    BLANKED=0
}

screen_unblank
wpctl set-volume $WPCTL_HDMI_ID ${quietVol}%


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


# Main scheduling loop
while true; do

    if is_active_hours; then

        # Active hours — ensure screen is on
        if [[ $BLANKED -eq 1 ]]; then
            screen_unblank
        fi

        # Volume ramp cycle: quietVol -> defaultVol -> quietVol
        direction=1  # 1 = ramping up, 0 = ramping down
        vol=$quietVol

        while true; do
            wpctl set-volume $WPCTL_HDMI_ID ${vol}%
            sleep 30
            if [[ $direction -eq 1 ]]; then
                vol=$((vol + 1))
                if [[ $vol -ge $defaultVol ]]; then
                    direction=0
                fi
            else
                vol=$((vol - 1))
                if [[ $vol -le $quietVol ]]; then
                    break
                fi
            fi
        done

        # Full ramp cycle complete — sleep random 12-24 hours
        sleep_seconds=$(( (RANDOM % 43200) + 43200 ))
        sleep $sleep_seconds

    else
        if [[ $BLANKED -eq 0 ]]; then
            screen_blank
        fi
    fi

    # Check every 60 seconds before checking again
    sleep 60
done
