#ifndef PARSER
#define PARSER

// parser of myscheme
#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std :: string;
using std :: vector;
using std :: pair;
using std :: cerr;
using std :: endl;
extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;
Expr holder = Expr(new MakeVoid());
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
	// to solve the problem of some variables of env maybe not defined
	// we let all the varibles be Real_Void at first!
	// it won't result in any error, because it will be overrided later.
	if (find(s, env).get() != nullptr)
		return Expr(new Var(s));
	if (reserved_words.find(s) != reserved_words.end())
		return Expr(new ExprBase(reserved_words[s]));
	if (primitives.find(s) != primitives.end())
		return Expr(new ExprBase(primitives[s]));
#ifdef Lazy_tag
	env = extend(s, Real_VoidV(), holder, env, env);
#else
	env = extend(s, Real_VoidV(), env);
#endif
	return Expr(new Var(s));
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

	// first we use the first Expr of the list to check
	// Acknowledgments: Thank Yuxuan Wang for his approach to tell this two calls apart
	// by changing the definition of Exprbase so that we can return Exprbase type

	Assoc e = env;
	Expr fst = u.front().parse(e);
	// function call
	if (typeid(ExprBase) != typeid(*fst)) {
		vector <Expr> val;
		for (int i = 1; i < u.size(); i++) {
			Assoc ee = env;
			val.push_back(u[i]->parse(ee));
		}
		return Expr(new Apply(fst, val));
	}
	// reserved_words
	switch (fst->e_type) {
	// attention: the environment of the parameters should be seperate!
	case E_LET:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		List *lst = dynamic_cast <List *> (u[1].get());
		if (!lst) throw RuntimeError("invalid format of let!");
		vector <pair<string, Expr> > res;
		Assoc ee = env;
		for (auto &sy : lst->stxs) {
			List *ele = dynamic_cast <List *> (sy.get());
			if (!ele || ele->stxs.size() != 2)
				throw RuntimeError("invalid format of let!");
			Identifier *v = dynamic_cast <Identifier *> (ele->stxs[0].get());
			if (!v) throw RuntimeError("invalid format of let!");
#ifdef Lazy_tag
			ee = extend(v->s, Real_VoidV(), holder, ee, ee);
#else
			ee = extend(v->s, Real_VoidV(), ee);
#endif
			Assoc e2 = env;
			res.push_back({ v->s, ele->stxs[1]->parse(e2) });
		}
		Expr body = u[2]->parse(ee);
		return Expr(new Let(res, body));
		break;
	}
	case E_LAMBDA:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		List *lst = dynamic_cast <List *> (u[1].get());
		if (!lst) throw RuntimeError("invalid format of lambda!");
		vector <string> res;
		Assoc ee = e;
		for (auto & sy:lst->stxs) {
			Identifier *v = dynamic_cast <Identifier *> (sy.get());

			if (!v) throw RuntimeError("invalid format of let!");
#ifdef Lazy_tag
			ee = extend(v->s, Real_VoidV(), holder, ee, ee);
#else
			ee = extend(v->s, Real_VoidV(), ee);
#endif
			res.push_back(v->s);
		}
		Expr body = u[2]->parse(ee);
		return Expr(new Lambda(res, body));
		break;
	}
	case E_LETREC:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		List *lst = dynamic_cast <List *> (u[1].get());
		if (!lst) throw RuntimeError("invalid format of letrec!");
		vector <pair<string, Expr> > res;
		Assoc ee = env;
		for (auto &sy : lst->stxs) {
			List *ele = dynamic_cast <List *> (sy.get());
			if (!ele || ele->stxs.size() != 2)
				throw RuntimeError("invalid format of letrec!");
			Identifier *v = dynamic_cast <Identifier *> (ele->stxs[0].get());
			if (!v) throw RuntimeError("invalid format of let!");
#ifdef Lazy_tag
			ee = extend(v->s, Real_VoidV(), holder, ee, ee);
#else
			ee = extend(v->s, Real_VoidV(), ee);
#endif
		}
		for (auto &sy : lst->stxs) {
			List *ele = dynamic_cast <List *> (sy.get());
			Identifier *v = dynamic_cast <Identifier *> (ele->stxs[0].get());
			Assoc e2 = env;
			res.push_back({ v->s, ele->stxs[1]->parse(e2) });
		}
		Expr body = u[2]->parse(ee);
		return Expr(new Letrec(res, body));
		break;
	}
	case E_IF:
	{
		if (u.size() != 4)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env, e3 = env;
		return Expr(new If(u[1].parse(e1), u[2].parse(e2), u[3].parse(e3)));
		break;
	}
	case E_BEGIN:
	{
		if (u.size() <= 1)
			throw RuntimeError("invalid number of parameters");
		vector <Expr> tmp;
		for (int i = 1; i < u.size(); i++) {
			Assoc e = env;
			tmp.push_back(u[i].parse(e));
		}
		return Expr(new Begin(tmp));
		break;
	}
	case E_QUOTE:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		return Expr(new Quote(u[1]));
		break;
	}
	case E_MUL:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Mult(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_PLUS:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Plus(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_MINUS:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Minus(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_LT:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Less(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_LE:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new LessEq(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_EQ:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Equal(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_GE:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new GreaterEq(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_GT:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Greater(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_EQQ:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new IsEq(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_VOID:
	{
		if (u.size() != 1)
			throw RuntimeError("invalid number of parameters");
		return Expr(new MakeVoid());
	}
	case E_BOOLQ:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new IsBoolean(u[1].parse(e)));
		break;
	}
	case E_INTQ:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new IsFixnum(u[1].parse(e)));
		break;
	}
	case E_NULLQ:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new IsNull(u[1].parse(e)));
		break;
	}
	case E_PAIRQ:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new IsPair(u[1].parse(e)));
		break;
	}
	case E_PROCQ:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new IsProcedure(u[1].parse(e)));
		break;
	}
	case E_SYMBOLQ:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new IsSymbol(u[1].parse(e)));
		break;
	}
	case E_CONS:
	{
		if (u.size() != 3)
			throw RuntimeError("invalid number of parameters");
		Assoc e1 = env, e2 = env;
		return Expr(new Cons(u[1].parse(e1), u[2].parse(e2)));
		break;
	}
	case E_NOT:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new Not(u[1].parse(e)));
		break;
	}
	case E_CAR:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new Car(u[1].parse(e)));
		break;
	}
	case E_CDR:
	{
		if (u.size() != 2)
			throw RuntimeError("invalid number of parameters");
		Assoc e = env;
		return Expr(new Cdr(u[1].parse(e)));
		break;
	}
	case E_EXIT:
	{
		if (u.size() != 1)
			throw RuntimeError("invalid number of parameters");
		return Expr(new Exit());
		break;
	}
	}
}

#endif
