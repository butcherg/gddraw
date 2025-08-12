#ifndef __MATHPARSER_H__
#define __MATHPARSER_H__

#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <cmath>
#include <iostream>

enum ttype {
	T_string,
	T_num,
	T_plus, // +
	T_minus, // -
	T_mult, // *
	T_div,  // /
	T_mod, // %
	T_lparen,  // (
	T_rparen,  // )
	T_unknown, // error
	S_begin,
	S_end
};

std::string tokType(ttype t)
{
	if (t == T_string) return "T_string";
	if (t == T_num) return "T_num";
	if (t == T_plus) return "T_plus";
	if (t == T_minus) return "T_minus";
	if (t == T_mult) return "T_mult";
	if (t == T_div) return "T_div";
	if (t == T_mod) return "T_mod";
	if (t == T_lparen) return "T_lparen";
	if (t == T_rparen) return "T_rparen";
	if (t == S_begin) return "S_begin";
	if (t == S_end) return "S_end";
	return "T_unknown";
}

struct token {
	ttype toktype;
	std::string sval;
	double dval;
};

class MathStack
{
public:
	MathStack() {}

	bool push(double d)
	{
		s.push_back(d);
		return true;
	}

	double pop()
	{
		float r =  s[s.size()-1];
		s.pop_back();
		return r;
	}

	bool doOp(ttype op)
	{
		if (op == T_plus | op == T_minus | op == T_mult | op == T_div | op == T_mod) {
			float rop = pop();
			float lop = pop();
		
			if (op == T_plus) 
				push(lop + rop);
			else if (op == T_minus) 
				push(lop - rop);
			else if (op == T_mult) 
				push(lop * rop);
			else if (op == T_div) 
				push(lop / rop);
			else if (op == T_mod) 
				push((int) lop % (int) rop);
			return true;
		}
		else {
			return false;
		}
	}

	void dumpStack()
	{
		for (std::vector<float>::iterator it = s.begin(); it != s.end(); ++it)
			std::cout << *it << std::endl;
	}

private:
	std::vector<float> s;
};


class Lexer
{
public:
	Lexer()
	{
		s = "";
		l = 0;
		n = 0;
	}

	Lexer(std::string expression)
	{
		s = expression;
		l = s.size();
		n = 0;
	}

	void setExpression(std::string expression)
	{
		s = expression;
		l = s.size();
		n = 0;
	}
	
	token nextToken(bool advance=true)
	{
		token t;

		if (n >= l) return (token) {S_end, "", 0.0};

		unsigned m=n;

		if (isspace(s[m])) {
			while (isspace(s[n]))
				m++;
		}

		if (s[m] == '+') {
			t.sval = "+";
			t.toktype = T_plus;
			m++;
		}
		else if (s[m] == '-') {
			t.sval = "-";
			t.toktype = T_minus;
			m++;
		}
		else if (s[m] == '*') {
			t.sval = "*";
			t.toktype = T_mult;
			m++;
		}
		else if (s[m] == '/') {
			t.sval = "/";
			t.toktype = T_div;
			m++;
		}
		else if (s[m] == '%') {
			t.sval = "%";
			t.toktype = T_mod;
			m++;
		}
		else if (s[m] == '(') {
			t.sval = "(";
			t.toktype = T_lparen;
			m++;
		}
		else if (s[m] == ')') {
			t.sval = ")";
			t.toktype = T_rparen;
			m++;
		}
		
		else if (isalpha(s[m])) {
			while (isalpha(s[m])) {
				t.sval.push_back(s[m]);
				m++;
			}
			t.toktype = T_string;
		}
		else if (isdigit(s[m])) {
			while (isdigit(s[m]) | s[m] == '.') {
				t.sval.push_back(s[m]);
				m++;
			}
			t.dval = atof(t.sval.c_str());
			t.toktype = T_num;
		}
		else {
			t.toktype = T_unknown;
			t.sval.push_back(s[m]);
			n++;
		}

		if (advance) n = m;

		lastlength = t.sval.size();

		return t;
	}
	
	unsigned getPos()
	{
		return n-lastlength;
	}

	token peekToken()
	{
		return nextToken(false);
	}

private:
	std::string s;
	unsigned n, l, lastlength;
};


/* grammar:
E  –> T Ep 
Ep –> + T Ep | epsilon 
T  –> F Tp 
Tp –> * F Tp | epsilon
F  –> ( E ) | U
U  -> '+' num | '-' num | num 
*/

class Parser
{
public:
	Parser() { }

	bool parse(std::string expression, float &answer)
	{
		expr = expression;
		l.setExpression(expression);
		if (E("")) {
			answer = s.pop();
			return true;
		}
		else 
			return false;
	}

	unsigned getPos()
	{
		return l.getPos();
	}

	bool err(std::string emsg)
	{
		//std::cout << "Error: " << emsg << std::endl;
		e = "Error: "+emsg+"\n";
		return false;
	}

	std::string getError()
	{
		return e;
	}


	//encapsulation of Lexer methods:

	token nextToken(std::string parent)
	{
		token t = l.nextToken();
		std::string name = graphNextPName("tok");
		graphAddNode(NTok, name, t.sval);
		if (parent.size() > 0) graphAddConnector(parent, name);
		
		return t;
	}

	token peekToken()
	{
		return l.peekToken();
	}

	//graph development routines:

	std::string graphNextPName(std::string p)
	{
		if (pnames.find(p) != pnames.end() ) pnames[p] == 0;
		//return p+"<SUB>"+std::to_string(pnames[p]++)+"</SUB>";
		return p+std::to_string(pnames[p]++);
	}

	enum ntypes {
		NProd,
		NTok
	};

	void graphAddNode(ntypes t, std::string name, std::string label)
	{
		if (t == NTok)
			pnodes.push_back("\t"+name+" [shape=box,label=\""+label+"\"];\n");
		else
			pnodes.push_back("\t"+name+"[label=\""+label+"\"];\n");
	}

	void graphAddConnector(std::string from, std::string to)
	{
		pconnectors.push_back("\t"+from+" -> "+to+";\n");
	}

	std::string graphWriteGraph()
	{
		std::string g = "digraph G {\n";
		
		g.append("\tlabel = \""+expr+"\"\n\tlabelloc  =  t\n\tfontsize = 20");
		
		for (std::vector<std::string>::iterator it = pnodes.begin(); it != pnodes.end(); ++it)
			g.append(*it);
		
		for (std::vector<std::string>::iterator it = pconnectors.begin(); it != pconnectors.end(); ++it)
			g.append(*it);
		
		g.append("}\n");
		
		return g;
	}

	//Productions:

	// E –> T Ep 
	bool E(std::string parent)
	{	
		std::string name = graphNextPName("E");
		graphAddNode(NProd, name, "E");
		if (parent.size() > 0) graphAddConnector(parent, name);

		if (T(name)) {
			if (Ep(name)) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	// Ep –> +|- T Ep | e 
	bool Ep(std::string parent)
	{
		std::string name = graphNextPName("Ep");
		graphAddNode(NProd, name, "Ep");
		graphAddConnector(parent, name);
		
		token t = peekToken();
		if (t.toktype == T_unknown) return err("unknown token: "+t.sval);

		if (t.toktype == T_plus | t.toktype == T_minus) {
			nextToken(name);  //eat the peek
			if (T(name)) {
				if (Ep(name)) {
					return s.doOp(t.toktype);
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		return true;  //epsilon
	}

	// T –> F Tp 
	bool T(std::string parent)
	{
		std::string name = graphNextPName("T");
		graphAddNode(NProd, name, "T");
		graphAddConnector(parent, name);
		
		if (F(name)) {
			if (Tp(name)) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	// Tp –> *|/ F Tp | e 
	bool Tp(std::string parent)
	{
		std::string name = graphNextPName("Tp");
		graphAddNode(NProd, name, "Tp");
		graphAddConnector(parent, name);
		
		token t = peekToken();
		if (t.toktype == T_unknown) return err("unknown token: "+t.sval);

		if  (t.toktype == T_mult | t.toktype == T_div | t.toktype == T_mod) {
			nextToken(name);
			if (F(name)) {
				if (Tp(name)) {
					return s.doOp(t.toktype);
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		return true; // epsilon
	}

	// F –> ( E ) | U
	bool F(std::string parent)
	{
		std::string name = graphNextPName("F");
		graphAddNode(NProd, name, "F");
		graphAddConnector(parent, name);
		
		token t = peekToken();

		if (t.toktype == T_lparen) {
			nextToken(name);
			if (E(name)) {
				if ((t = nextToken(name)).toktype == T_rparen) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			if (U(name)) {
				return true;
			}
		}
		
		return false;
	}

	// U -> '+' num | '-' num | num 
	bool U(std::string parent)
	{	
		std::string name = graphNextPName("U");
		graphAddNode(NProd, name, "U");
		graphAddConnector(parent, name);
	
		token t = nextToken(name);

		if (t.toktype == T_plus) {
			if ((t = nextToken(name)).toktype == T_num) {
				return s.push(+t.dval);
			}
			else {
				return err("invalid number: "+t.sval);
			}
		}
		else if (t.toktype == T_minus) {
			if ((t = nextToken(name)).toktype == T_num) {
				return s.push(-t.dval);
			}
			else {
				return err("invalid number: "+t.sval);
			}
		}
		else if (t.toktype == T_num) {
			return s.push(t.dval);
		}
		return err( "invalid number: "+t.sval);
	}

private:
	Lexer l;
	MathStack s;
	std::string expr;  //storage for the expression
	std::string e; //error message, loaded by err()
	
	std::map<std::string, unsigned> pnames;
	std::vector<std::string> pnodes;
	std::vector<std::string> pconnectors;
};


#endif
