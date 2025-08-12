#include "gd.h"
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream> 
#include <fstream>
#include <cctype>
#include <algorithm>

#include "mathparser.h"

std::map<std::string, std::string> vars;
static unsigned lineno;
static gdImagePtr im;
static std::vector<gdPoint> polypts;
static int linecolor = 0x00FFFFFF, fillcolor = 0x000000FF;
static int w, h;

void err(std::string msg)
{
	std::cout << msg << std::endl;
	exit(EXIT_FAILURE);
}

std::string tostr(double t)
{ 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 

std::vector<std::string> split(std::string s, std::string delim)
{
	std::vector<std::string> v;
	if (s.find(delim) == std::string::npos) {
		v.push_back(s);
		return v;
	}
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == std::string::npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}

int countchar(std::string s, char c)
{
	int count = 0;
	for (int i=0; i<s.size(); i++) {
		if (s[i] == c) count++;
	}
	return count;
}

std::vector<std::string> bifurcate(std::string strg, char c = ' ', bool fromback=false)
{
	std::vector<std::string> result;
	if (countchar(strg, c) == 0) {
		result.push_back(strg);
	}
	else {
		std::size_t eq;
		if (fromback)
			eq = strg.find_last_of(c);
		else
			eq = strg.find_first_of(c);
		result.push_back(strg.substr(0,eq));
		result.push_back(strg.substr(eq+1));
	}
	return result;
}

std::string string_format(const std::string fmt, ...) 
{
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}

std::string replaceVars(std::string expr)
{
	std::string outstr;
	for (unsigned i=0; i<expr.size(); i++) {
		if (expr[i] == '$') {
			std::string r;
			r.push_back(expr[i]);
			i++;
			while (isalpha(expr[i]) | isdigit(expr[i])) {
				if (i >= expr.size()) err(string_format("Line %d syntax error: unexpected end of expression.", lineno));
				r.push_back(expr[i]);
				i++;
			}
			i--;
			if (vars.find(r) != vars.end())
				outstr.append(vars[r]);
			else
				err(string_format("Line %d Compile error: variable '%s' doesn't exist.", lineno, r.c_str()));
		}
		else
			outstr.push_back(expr[i]);
	}
	return outstr;
}

float evalString(std::string expr)
{
	//std::cout << "evalString: " << expr << std::endl;
	float r;
	std::string e = replaceVars(expr);
	Parser p;
	p.parse(e, r);
	return r;
}

void removechar(std::string &s, char c)
{
	s.erase(remove(s.begin(), s.end(), c), s.end());
}

void  parseLine(std::string line)
{
	//remove comments:
	std::vector<std::string> u = bifurcate(line, '#');
	if (u[0].size() <= 0) return;
	std::string stmt = u[0];
	removechar(stmt, ';');
	removechar(stmt, ' ');
	
	std::vector<std::string> t; //, t1;
	
	if (stmt[0] == '$') //variable assignment, "$i=25;"
	{
		t = split(stmt, "=");
		if (t.size() == 2) 
			vars[t[0]] = tostr(evalString(t[1]));
		else err(string_format("Line %d syntax error: malformed assignment (%s).",lineno, u[0].c_str()));
	}
	else {
		if (stmt[stmt.size()-1] == ')') stmt.erase(stmt.size()-1, 1);
		t = bifurcate(stmt, '(');
		
		//if (t.size() >=2)
		//	printf("%s - %s\n", t[0].c_str(), t[1].c_str());
		//else
		//	printf("%s\n",t[0].c_str());
		
		//cmd - image(w,h); - Initializes the image. Do this first, or the rest of the script will not work.
		if (t[0] == "image") {  //new image, "image(800,600);"
			if (t.size() < 2) err(string_format("Line %d 'image' syntax error: no parameters - %d", lineno, t.size()));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 2) err(string_format("Line %d 'image' syntax error: not enough parameters - %d", lineno, t1.size()));
			im = gdImageCreateTrueColor(int(evalString(t1[0])), int(evalString(t1[1])));
			gdImageSetThickness(im, 1);
		}
		//cmd - pensize(n); - Sets the pen size for subsequent lines.  Default: 1.
		else if (t[0] == "pensize" | t[0] == "penSize") {
			if (!im) err(string_format("Line %d 'pensize' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 1) err(string_format("Line %d 'pensize' syntax error: not enough parameters - %d", lineno, t1.size()));
			gdImageSetThickness(im, int(evalString(t1[0])));
		}
		//cmd - linecolor('black'|'white'|hex); Sets the pen color for subsequent lines. Default: 'white'.
		else if (t[0] == "linecolor") {
			if (!im) err(string_format("Line %d 'linecolor' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 1) err(string_format("Line %d 'linecolor' syntax error: not enough parameters - %d", lineno, t1.size()));
			if (t1[0] == "'black'")
				linecolor = 0x00000000;
			else if (t1[0] == "'white'")
				linecolor = 0x00FFFFFF;
			else
				linecolor = std::stoi(t1[0], nullptr, 0);
		}
		//cmd - fillcolor('black'|'white'|hex); Sets the fill color for subsequent rectangles and polygons.  Default: 'black'.
		else if (t[0] == "fillcolor") {
			if (!im) err(string_format("Line %d 'fillcolor' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 1) err(string_format("Line %d 'fillcolor' syntax error: not enough parameters - %d", lineno, t1.size()));
			if (t1[0] == "'black'")
				fillcolor = 0x00000000;
			else if (t1[0] == "'white'")
				fillcolor = 0x00FFFFFF;
			else
				fillcolor = std::stoi(t1[0], nullptr, 0);
		}
		//cmd - backgroundfill; - Fills the image with the fillcolor starting from the image center.  Use to color image before other operations.
		else if (t[0] == "backgroundfill") {
			if (!im) err(string_format("Line %d 'backgroundfill' error: 'image' not defined.", lineno));
			gdImageFill(im, w/2, h/2, fillcolor);
		}
		//cmd - polyclear; - Clears the polygon array of points, to prepare for constructing a new polygon.
		else if (t[0] == "polyclear") {
			if (!im) err(string_format("Line %d 'clearpoly' error: 'image' not defined.", lineno));
			polypts.clear();
		}
		//cmd - polypt(x,y); - Adds a point to the polygon array.
		else if (t[0] == "polypt") {
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 2) err(string_format("Line %d 'polypt' syntax error: not enough parameters - %d", lineno, t1.size()));
			gdPoint pt; pt.x = int(evalString(t1[0])); pt.y = int(evalString(t1[1])); 
			polypts.push_back(pt);
		}
		//cmd - polyoffset(x,y); - Translates the polygon array to the specified coordinate.  This modifies the points in the polygon array.
		else if (t[0] == "polyoffset") {
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 2) err(string_format("Line %d 'polyoffset' syntax error: not enough parameters - %d", lineno, t1.size()));
			int ox = int(evalString(t1[0])); int oy = int(evalString(t1[1])); 
			for (auto &p : polypts) {
				p.x+= ox;
				p.y+= oy;
			}
		}
		//cmd - polygon; - Renders the polygon array as a line polygon using the linecolor.
		else if (t[0] == "polygon") {
			if (!im) err(string_format("Line %d 'polygon' error: 'image' not defined.", lineno));
			if (polypts.size() < 3) err(string_format("Line %d 'polygon' error: not enough polygon points - %d.", lineno, polypts.size()));
			gdImagePolygon (im, polypts.data(), polypts.size(), linecolor);
		}
		//cmd - filledpoly; - Renders the polygon array as a fill area using the fillcolor.
		else if (t[0] == "filledpoly") {
			if (!im) err(string_format("Line %d 'filledpoly' error: 'image' not defined.", lineno));
			if (polypts.size() < 3) err(string_format("Line %d 'filledpoly' error: not enough polygon points - %d.", lineno, polypts.size()));
			gdImageFilledPolygon (im, polypts.data(), polypts.size(), fillcolor);
		}
		//cmd - filledlinepoly; - Renders the polygon array as a fill area using the fill color, with an outline using the linecolor.
		else if (t[0] == "filledlinepoly") {
			if (!im) err(string_format("Line %d 'filledlinepoly' error: 'image' not defined.", lineno));
			if (polypts.size() < 3) err(string_format("Line %d 'filledlinepoly' error: not enough polygon points - %d.", lineno, polypts.size()));
			gdImageFilledPolygon (im, polypts.data(), polypts.size(), fillcolor);
			gdImagePolygon (im, polypts.data(), polypts.size(), linecolor);
		}
		//cmd - line(x1,y1,x2,y2); - Renders a line in the linecolor from x1,y1 to x2,y2.
		else if (t[0] == "line") {
			if (!im) err(string_format("Line %d 'line' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 4) err(string_format("Line %d 'line' syntax error: not enough parameters - %d", lineno, t1.size()));
			gdImageLine(im, int(evalString(t1[0])), int(evalString(t1[1])), int(evalString(t1[2])), int(evalString(t1[3])), linecolor);
		}
		//cmd - rectangle(x1,y1,x2,y2); - Renders a rectangle as an outline in the linecolor from corner x1,y1 to corner x2,y2.
		else if (t[0] == "rectangle") {
			if (!im) err(string_format("Line %d 'rectangle' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 4) err(string_format("Line %d 'rectangle' syntax error: not enough parameters - %d", lineno, t1.size()));
			gdImageRectangle(im, int(evalString(t1[0])), int(evalString(t1[1])), int(evalString(t1[2])), int(evalString(t1[3])), linecolor);
		}
		//cmd - filledrectangle(x1,y1,x2,y2); - Renders a rectangle as a filled area in the fillcolor from corner x1,y1 to corner x2,y2.
		else if (t[0] == "filledrectangle") {
			if (!im) err(string_format("Line %d 'filledrectangle' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 4) err(string_format("Line %d 'filledrectangle' syntax error: not enough parameters - %d", lineno, t1.size()));
			gdImageFilledRectangle(im, int(evalString(t1[0])), int(evalString(t1[1])), int(evalString(t1[2])), int(evalString(t1[3])), fillcolor);
		}
		//cmd - filledlinerectangle(x1,y1,x2,y2); - Renders a rectangle as a filled area in the fillcolor and an outline in the line color from corner x1,y1 to corner x2,y2.
		else if (t[0] == "filledlinerectangle") {
			if (!im) err(string_format("Line %d 'rectangle' error: 'image' not defined.", lineno));
			std::vector<std::string> t1 = split(t[1], ",");
			if (t1.size() < 4) err(string_format("Line %d 'rectangle' syntax error: not enough parameters - %d", lineno, t1.size()));
			gdImageFilledRectangle(im, int(evalString(t1[0])), int(evalString(t1[1])), int(evalString(t1[2])), int(evalString(t1[3])), fillcolor);
			gdImageRectangle(im, int(evalString(t1[0])), int(evalString(t1[1])), int(evalString(t1[2])), int(evalString(t1[3])), linecolor);
		}
		//cmd - flipvertical; - Flips the image vertically;
		else if (t[0] == "flipvertical") {
			if (!im) err(string_format("Line %d 'flipvertical' syntax error: 'image' not defined.", lineno));
			gdImageFlipVertical(im);
		}
		//cmd - fliphorizontal; - Flips the image horizontally;
		else if (t[0] == "fliphorizontal") {
			if (!im) err(string_format("Line %d 'fliphorizontal' syntax error: 'image' not defined.", lineno));
			gdImageFlipHorizontal(im);
		}
		else if (t[0] == "print") {
			std::string s = replaceVars(t[1]);
			std::cout << s << std::endl;
		}
		else if (t[0] == "print_variables") {
			if (vars.size() > 0)
				for (auto const& m : vars) std::cout << m.first << ": " << m.second << std::endl;
			else
				std::cout << "no variables" << std::endl;
		}
		else {
			err(string_format("Line %d syntax error: unknown keyword: %s.",lineno, t[0].c_str()));
		}
	}
}


int main(int argc, char **argv)
{
	if (argc < 2) err("Error: no script filename.");
	if (argc < 3) err("Error: no image filename.");
	
	std::ifstream qfile(argv[1]);
	std::vector<std::string> lines;
	for (std::string line; std::getline( qfile, line ); /**/ )
		lines.push_back( line );
	
	for (lineno = 1; lineno <= lines.size(); lineno++)
		parseLine(lines[lineno-1]);
	
	std::cout << "Saving " << argv[2] << std::endl;
	FILE *fp = fopen(argv[2], "wb");
	if (!fp) {
		gdImageDestroy(im);
		err("Can't save png image.\n");
	}
	if (im) {
		gdImagePng(im, fp);
		fclose(fp);
		gdImageDestroy(im);
	}
	else err("Image not created for saving.");
	
	return 0;
}
