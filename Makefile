all:
	gcc ezview.c -lGLESv2 -lglfw -o ezview
run:
	./ezview input.ppm
clean:
	rm ezview
	clear
