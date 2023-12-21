#include "Def.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>


extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;
using std :: vector;
using std :: string;
using std :: cerr;
using std :: endl;

Value ExprBase::eval(Assoc & env)
{
	throw RuntimeError("illegal expression !");
	// something wrong while parsing
}
Value Let::eval(Assoc &env)
{
	Assoc e = env;

	for (auto &[x, y]:bind) {
		Assoc ee = env;
#ifndef Lazy_tag
		e = extend(x, y->eval(ee), e);
#else
		e = extend(x, IntegerV(0), y, ee, e);
#endif
	}
	return body->eval(e);
}                              // let expression

Value Lambda::eval(Assoc &env)
{
	return Value(new Closure(x, e, env));
}                                 // lambda expression

Value Apply::eval(Assoc &e)
{
	Assoc ee = e;
	Value val = rator->eval(ee);

	if (val->v_type != V_PROC)
		throw RuntimeError("parameter error, expected a procedure ");
	Closure *clsr = dynamic_cast <Closure *> (val.get());
	if (clsr->parameters.size() != rand.size())
		throw RuntimeError("the number of paramters is illgeal ");
	Assoc env = clsr->env;
	int sz = clsr->parameters.size();
	for (int i = 0; i < sz; i++) {
		string ss = clsr->parameters[i];

		Assoc ee = e;
#ifndef Lazy_tag
		Value v = rand[i]->eval(ee);
		env = extend(ss, v, env);
#else
		env = extend(ss, IntegerV(0), rand[i], ee, env);
#endif
	}
	return clsr->e->eval(env);
}                              // for function calling

Value Letrec::eval(Assoc &env)
{
	Assoc ee = env, e1 = env;

	for (auto &[x, y]:bind) {
#ifndef Lazy_tag
		ee = extend(x, Real_VoidV(), ee);
#else
		ee = extend(x, Real_VoidV(), y, e1, ee);
#endif
		Value val = y->eval(ee);
		Real_Void *rv = dynamic_cast <Real_Void *> (val.get());
		if (rv) throw RuntimeError("parameter not defined yet!");
	}
	for (auto &[x, y]:bind)
		modify(x, y->eval(ee), ee);
	return body->eval(ee);
}                                 // letrec expression

Value Var::eval(Assoc &e)
{
	Value val = find(x, e);

	if (val.get() == nullptr)
		throw RuntimeError("varible not found ");
	return val;
}                            // evaluation of variable

Value Fixnum::eval(Assoc &e)
{
	return Value(new Integer(n));
}                               // evaluation of a fixnum
// it is necessary to check the type of the value
// only #f return false
bool is_true(const Value &v)
{
	if (v->v_type != V_BOOL)
		return true;
	Boolean *bl = dynamic_cast <Boolean *> (v.get());
	return bl->b == true;
}
Value If::eval(Assoc &e)
{
	Assoc ee = e, env = e;
	Value res = cond->eval(env);

	if (is_true(res) == true)
		return conseq->eval(ee);
	else return alter->eval(ee);
}                           // if expression

Value True::eval(Assoc &e)
{
	return Value(new Boolean(true));
}                             // evaluation of #t

Value False::eval(Assoc &e)
{
	return Value(new Boolean(false));
}                              // evaluation of #f

Value Begin::eval(Assoc &e)
{
	Value res(nullptr);

	for (auto &u:es) {
		Assoc ee = e;
		res = u->eval(ee);
	}
	return res;
}                              // begin expression
// to eval the quote
// we need to tell list and other types apart
// then to eval the list of it
// we need recursion to solve the list occasion
Value quote_normal(const Syntax &s, Assoc &e);
Value quote_list(const vector <Syntax> &s, int pos, Assoc &e)
{
	if (!s.size()) return Value(new Null());
	if (pos == s.size()) return Value(new Null());
	return Value(new Pair(quote_normal(s[pos], e), quote_list(s, pos + 1, e)));
}                              // quote expression
Value quote_normal(const Syntax &s, Assoc &e)
{
	if (Number *val = dynamic_cast <Number *> (s.get()))
		return Value(new Integer(val->n));
	if (Identifier *val = dynamic_cast <Identifier *> (s.get()))
		return Value(new Symbol(val->s));
	if (TrueSyntax *val = dynamic_cast <TrueSyntax *> (s.get()))
		return Value(new Boolean(true));
	if (FalseSyntax *val = dynamic_cast <FalseSyntax *> (s.get()))
		return Value(new Boolean(false));
	if (List *val = dynamic_cast <List *> (s.get()))
		return quote_list(val->stxs, 0, e);
	throw RuntimeError("wrong quote parameter!");
}
Value Quote::eval(Assoc &e)
{
	Assoc ee = e;

	return quote_normal(s, ee);
}
Value MakeVoid::eval(Assoc &e)
{
	return Value(new Void());
}                                 // (void)

Value Exit::eval(Assoc &e)
{
	return Value(new Terminate());
}                             // (exit)

Value Binary::eval(Assoc &e)
{
	Assoc e1 = e, e2 = e;
	Value val1 = rand1->eval(e1), val2 = rand2->eval(e2);

	return evalRator(val1, val2);
}                               // evaluation of two-operators primitive
Value Unary::eval(Assoc &e)
{
	Assoc ee = e;
	Value val = rand->eval(ee);

	return evalRator(val);
}                                                               // evaluation of single-operator primitive
Value Mult::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Integer((val1->n) * (val2->n)));
}                                                                // *
Value Plus::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Integer((val1->n) + (val2->n)));
}                                                                // +

Value Minus::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Integer((val1->n) - (val2->n)));
}                                                                 // -

Value Less::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Boolean((val1->n) < (val2->n)));
}                                                                // <

Value LessEq::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Boolean((val1->n) <= (val2->n)));
}                                                                  // <=

Value Equal::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Boolean((val1->n) == (val2->n)));
}                                                                 // =

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error !");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	// cerr << val1->n << " " << val2->n << endl;
	return Value(new Boolean((val1->n) >= (val2->n)));
}                                                                     // >=

Value Greater::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error !");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Boolean((val1->n) > (val2->n)));
}                                                                   // >

Value IsEq::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
		Integer *val1 = dynamic_cast <Integer *> (rand1.get());
		Integer *val2 = dynamic_cast <Integer *> (rand2.get());
		return Value(new Boolean(val1->n == val2->n));
	}
	if (rand1->v_type == V_BOOL && rand2->v_type == V_BOOL) {
		Boolean *val1 = dynamic_cast <Boolean *> (rand1.get());
		Boolean *val2 = dynamic_cast <Boolean *> (rand2.get());
		return Value(new Boolean(val1->b == val2->b));
	}
	if (rand1->v_type == V_SYM && rand2->v_type == V_SYM) {
		Symbol *val1 = dynamic_cast <Symbol *> (rand1.get());
		Symbol *val2 = dynamic_cast <Symbol *> (rand2.get());
		return Value(new Boolean(val1->s == val2->s));
	}
	return Value(new Boolean(rand1.get() == rand2.get()));
}                                                                // eq?

Value Cons::evalRator(const Value &rand1, const Value &rand2)
{
	return Value(new Pair(rand1, rand2));
}                                                                // cons

Value IsBoolean::evalRator(const Value &rand)
{
	if (rand->v_type == V_BOOL)
		return Value(new Boolean(true));
	return Value(new Boolean(false));
}                                                // boolean?

Value IsFixnum::evalRator(const Value &rand)
{
	if (rand->v_type == V_INT)
		return Value(new Boolean(true));
	return Value(new Boolean(false));
}                                               // fixnum?

Value IsSymbol::evalRator(const Value &rand)
{
	if (rand->v_type == V_SYM)
		return Value(new Boolean(true));
	return Value(new Boolean(false));
}                                               // symbol?

Value IsNull::evalRator(const Value &rand)
{
	if (rand->v_type == V_NULL)
		return Value(new Boolean(true));
	return Value(new Boolean(false));
}                                             // null?

Value IsPair::evalRator(const Value &rand)
{
	if (rand->v_type == V_PAIR)
		return Value(new Boolean(true));
	return Value(new Boolean(false));
}                                             // pair?

Value IsProcedure::evalRator(const Value &rand)
{
	if (rand->v_type == V_PROC)
		return Value(new Boolean(true));
	return Value(new Boolean(false));
}                                                  // procedure?

Value Not::evalRator(const Value &rand)
{
	return Value(new Boolean(!is_true(rand)));
}                                          // not

#ifndef Lazy_tag

Value Car::evalRator(const Value &rand)
{
	if (rand->v_type != V_PAIR)
		throw RuntimeError("Type Error");
	Pair *val = dynamic_cast <Pair *> (rand.get());
	return val->car;
}                                          // car
Value Cdr::evalRator(const Value &rand)
{
	if (rand->v_type != V_PAIR)
		throw RuntimeError("Type Error");
	Pair *val = dynamic_cast <Pair *> (rand.get());
	return val->cdr;
}                                          // cdr
#else
// we only need to cacluate half of the car and cdr
Value Car::eval(Assoc &e)
{
	Quote *tmp = dynamic_cast <Quote *> (ex.get());
	Assoc ee = e;

	if (tmp) {
		List *lst = dynamic_cast <List *> ((*tmp).s.get());
		if (!lst) throw RuntimeError("wrong type!!");
		return Value(lst->stxs[0]->parse(ee)->eval(ee));
	} else {
		Cons *cos = dynamic_cast <Cons *> (ex.get());
		if (cos) {
			return Value(cos->rand1->eval(ee));
		} else {
			Var *v = dynamic_cast <Var *> (ex.get());
			if (v) {
				Value tv = v->eval(ee);
				Pair *tp = dynamic_cast <Pair *> (tv.get());
				if (!tp) throw RuntimeError("wrong type!!!");
				return tp->car;
			} else {
				Value tv = this->ex->eval(ee);
				Pair *tp = dynamic_cast <Pair *> (tv.get());
				if (!tp) throw RuntimeError("wrong type!!!");
				return tp->car;
			}
		}
	}
}

Value Cdr::eval(Assoc &e)
{
	Quote *tmp = dynamic_cast <Quote *> (ex.get());
	Assoc ee = e;

	if (tmp) {
		List *lst = dynamic_cast <List *> ((*tmp).s.get());
		if (!lst) throw RuntimeError("wrong type!!");
		return quote_list(lst->stxs, 1, ee);
	} else {
		Cons *cos = dynamic_cast <Cons *> (ex.get());
		if (cos) {
			return Value(cos->rand2->eval(ee));
		} else {
			Var *v = dynamic_cast <Var *> (ex.get());
			if (v) {
				Value tv = v->eval(ee);
				Pair *tp = dynamic_cast <Pair *> (tv.get());
				if (!tp) throw RuntimeError("wrong type!!!");
				return tp->cdr;
			} else {
				Value tv = this->ex->eval(ee);
				Pair *tp = dynamic_cast <Pair *> (tv.get());
				if (!tp) throw RuntimeError("wrong type!!!");
				return tp->cdr;
			}
		}
	}
}

#endif
