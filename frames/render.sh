ffmpeg -framerate 60 -i %d.png -y -r 60 -pix_fmt yuv420p out.mp4
rm *.png
