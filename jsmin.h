#ifndef _JSMIN_H_
#define _JSMIN_H_

class JSMin
{
public:
	JSMin():theLookahead(EOF)
	{}
	void go();

private:
	int theA;
	int theB;
	int theLookahead;
	
	int isAlphanum(int c);
	int get();
	virtual int getChar() = 0;
	virtual void put(int _Ch) = 0;
	int peek();
	int next();
	void action(int d);
};

#endif

