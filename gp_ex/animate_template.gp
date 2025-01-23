# plot histogram as an animation
reset
set term gif animate
set output "<filename>"

# n frames in the animation
max_frames = <number_frames>
i = 0

load "animate_helper.gp"
set output
