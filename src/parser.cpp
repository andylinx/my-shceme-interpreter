#ifndef PARSER
#define PARSER

// parser of myscheme

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std :: string;
using std :: vector;
using std :: pair;

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Expr Syntax :: parse(Assoc &env)
{
	return (*(this->ptr)).parse(env);
}

Expr Number :: parse(Assoc &env)
{
	return Expr(new Fixnum(this->n));
}

Expr Identifier :: parse(Assoc &env)
{
	return Expr(new Var(this->s));
}

Expr TrueSyntax :: parse(Assoc &env)
{
	return Expr(new True());
}

Expr FalseSyntax :: parse(Assoc &env)
{
	return Expr(new False());
}

Expr List :: parse(Assoc &env)
{
	auto &u = this->stxs;

	if (!u.size()) return Expr(new Quote(new List()));
	// to do !!
	switch (u[0].get_type()) {
	case E_LET:
	{
		break;
	}
	case E_LAMBDA:
	{
		if (u.size() != 4)
			throw RuntimeError("invalid number of values");
		return Expr(new If(u[1].parse(env), u[2].parse(env), u[3].parse(env)));
		break;
	}
	case E_APPLY:
	{
		break;
	}
	case E_LETREC:
	{
		break;
	}
	case E_IF:
	{
		if (u.size() != 4)
			throw RuntimeError("invalid number of values");
		return Expr(new If(u[1].parse(env), u[2].parse(env), u[3].parse(env)));
		break;
	}
	case E_BEGIN:
	{
		vector <Expr> tmp;
		for (int i = 1; i < u.size(); i++)
			tmp.push_back(u[i].parse(env));
		return Expr(new Begin(tmp));
		break;
	}
	case E_QUOTE:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of values");
		return Expr(new Quote(u[1]));
		break;
	}
	case E_MUL:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new Mult(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_PLUS:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new Plus(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_MINUS:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new Minus(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_LT:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new Less(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_LE:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new LessEq(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_EQ:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new Equal(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_GE:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new Greater(u[1].parse(env), u[2].parse(env)));
		break;
	}
	case E_GT:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of values");
		return Expr(new GreaterEq(u[1].parse(env), u[2].parse(env)));
		break;
	}
	}
}

#endif
