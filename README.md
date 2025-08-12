# gddraw - script-based drawing tool

I got tired of trying to get a Windows Perl environment that included libgd, so I wrote a command-line program that reads a script file to invoke GD primitives.  Available script statements:

- image(w,h); - Initializes the image. Do this first, or the rest of the script will not work.
- pensize(n); - Sets the pen size for subsequent lines.  Default: 1.
- linecolor('black'|'white'|hex); Sets the pen color for subsequent lines. Default: 'white'.
- fillcolor('black'|'white'|hex); Sets the fill color for subsequent rectangles and polygons.  Default: 'black'.
- backgroundfill; - Fills the image with the fillcolor starting from the image center.  Use to color image before other operations.
- polyclear; - Clears the polygon array of points, to prepare for constructing a new polygon.
- polypt(x,y); - Adds a point to the polygon array.
- polyoffset(x,y); - Translates the polygon array to the specified coordinate.  This modifies the points in the polygon array.
- polygon; - Renders the polygon array as a line polygon using the linecolor.
- filledpoly; - Renders the polygon array as a fill area using the fillcolor.
- filledlinepoly; - Renders the polygon array as a fill area using the fill color, with an outline using the linecolor.
- line(x1,y1,x2,y2); - Renders a line in the linecolor from x1,y1 to x2,y2.
- rectangle(x1,y1,x2,y2); - Renders a rectangle as an outline in the linecolor from corner x1,y1 to corner x2,y2.
- filledrectangle(x1,y1,x2,y2); - Renders a rectangle as a filled area in the fillcolor from corner x1,y1 to corner x2,y2.
- filledlinerectangle(x1,y1,x2,y2); - Renders a rectangle as a filled area in the fillcolor and an outline in the line color from corner x1,y1 to corner x2,y2.
- flipvertical; - Flips the image vertically;
- fliphorizontal; - Flips the image horizontally;

You can define variables, they start with a '$': $foo=21;

You can use basic math expressions in both variable assignments and statement parameters: image($w*2, ($h+20)*2);

## Dependencies

gddraw depends on libgd, an OS package is the easiest way to go.  For Debian/Ubuntu:

    $ apt get install libgd-dev

for MSYS2:

    $ pacman -S mingw-w64-x86_64-libgd 
    or
    $ pacman -S mingw-w64-ucrt-x86_64-libgd

## Building

    $ git clone https://github.com/butcherg/gddraw.git
    $ cd gddraw
    $ mkdir build && cd build
    $ cmake ..
    $ make|ninja|whatever

## Running

    $ ./gddraw scriptfile.txt image.png

gddraw only makes .png images at this time.
