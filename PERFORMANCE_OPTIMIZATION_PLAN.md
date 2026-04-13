# MPV Lag Reduction - Optimization Plan

## Current Problem
MPV overlay shows lag when rendering Cairo-drawn overlays via `overlay-add` IPC calls. The bottleneck appears to be in the shared memory sync and socket communication overhead.

---

## Change #1: Lower MPV's CPU Priority (NICENESS)
**Status: ✅ Implemented, ⏳ Pending Testing**

### What
Start mpv with a high nice value (-19 is most negative/highest priority, +19 is lowest priority). Currently mpv runs at default priority which may be competing with overlay_scroller.

### Implementation
Edit `startvlc.sh` line starting with `taskset -c 2 mpv`:
```bash
taskset -c 2 nice -n 19 mpv --loop --fullscreen --no-terminal \
    --hwdec=auto \
    --geometry=800x600 \
    --keepaspect=no \
    --input-ipc-server=/tmp/mpvsock \
    /media/upl/W/train.mp4 \
    >/tmp/mpv.log 2>&1 &
```

### Expected Effect
- mpv will yield CPU time more readily to overlay_scroller
- May reduce scheduling delays when overlay_scroller sends overlay-add commands
- Low risk, easy to revert

### Testing Notes
- Does overlay responsiveness improve?
- Any visible lag reduction?
- If worse, try `-n 0` (neutral) or remove `nice -n 19`

---

## Change #2: Remove/Reduce msync() Blocking Calls
**Status: ✅ Implemented, ⏳ Pending Testing**

### What
In `mainLoop.cpp`, `present_overlay()` calls `msync(g_shm_data, g_shm_size, MS_SYNC)` which blocks until all writes are flushed. This is a blocking I/O call that can add 10-100ms+ latency.

### Implementation Options (in order of safety):

**Option A - Remove msync entirely:**
```cpp
void present_overlay() {
    cairo_surface_flush(g_surface);
    // REMOVED: msync(g_shm_data, g_shm_size, MS_SYNC);
    // ... rest unchanged
}
```
Risk: Lowest. On modern systems with shared memory (tmpfs), dirty pages are often immediately visible to other processes without explicit sync.

**Option B - Use non-blocking MS_ASYNC:**
```cpp
msync(g_shm_data, g_shm_size, MS_ASYNC);
```
Risk: Very Low. Schedules background writeback, returns immediately.

**Option C - Conditional sync:**
Add a flag to only sync when content actually changes, not every frame.
Risk: Medium. Requires tracking dirty state.

### Expected Effect
- Significant latency reduction (potential 10-100ms per frame)
- Most impactful change

### Testing Notes
- Does overlay appear correctly without msync?
- Any artifacts or delayed rendering?
- If issues, try Option B (MS_ASYNC) as middle ground

---

## Change #3: Optimize MPV Command-Line Options
**Status: ⏳ Planned**

### What
Add GPU rendering flags to reduce CPU load and improve rendering efficiency.

### Implementation
Add these options to mpv command in `startvlc.sh`:
```bash
--vo=gpu              -- Use GPU video output (faster)
--gpu-context=egl     -- Share GPU context with display system
```

### Expected Effect
- Reduced CPU usage for mpv decoding/rendering
- Better performance headroom
- May help with overall frame timing

### Testing Notes
- Verify overlay still renders correctly
- Check for any GPU-related errors in /tmp/mpv.log

---

## Change #4: Reduce Socket Communication Frequency
**Status: ⏳ Planned**

### What
Currently the test loop sends `overlay-add` every 2 seconds. If content doesn't change, mpv will hold the previous frame anyway.

### Implementation
In `mainLoop.cpp`, modify the test loop to only send overlay-add when content actually changes, or add a longer hold between sends.

### Expected Effect
- Less socket write overhead
- Reduced mpv IPC load

### Testing Notes
- Verify overlay persists correctly between sends
- Measure socket write frequency reduction

---

## Testing Procedure

### Build Instructions (on Raspberry Pi)
```bash
cd /home/upl/upl_scroller
cmake -B build
make -C build -j4
```

### Test Procedure
1. **Kill any existing processes:**
   ```bash
   pkill mpv
   pkill upl_scroller
   pkill cvlc
   rm -f /tmp/mpvsock
   ```

2. **Run the script:**
   ```bash
   ./startvlc.sh
   ```

3. **Observe for 5-10 minutes** and note:
   - Is overlay rendering smoother?
   - Any reduction in lag/timing issues?
   - Any new problems?

4. **Report results** → Update plan status → Move to next change

---

## Files to Modify
- `startvlc.sh` - Changes #1, #3
- `src/mainLoop.cpp` - Changes #2, #4

---

## Success Criteria
- No visible lag when scrolling/updating overlay
- Smooth 30fps rendering
- No artifacts or display issues
- System remains responsive
