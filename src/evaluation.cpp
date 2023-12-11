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
using std::vector, std::string;

Value Let::eval(Assoc &env)
{
	Assoc e = env;

	for (auto &[x, y]:bind) {
		Assoc ee = env;
		e = extend(x, y->eval(ee), e);
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
		throw RuntimeError("parameter error, expected a procedure");
	Closure *clsr = dynamic_cast <Closure *> (val.get());
	if (clsr->parameters.size() != rand.size())
		throw RuntimeError("the number of paramters is illgeal");
	Assoc env = clsr->env;
	int sz = clsr->parameters.size();
	for (int i = 0; i < sz; i++) {
		string ss = clsr->parameters[i];
		Assoc ee = e;
		Value v = rand[i]->eval(ee);
		env = extend(ss, v, env);
	}
	return clsr->e->eval(env);
}                              // for function calling

Value Letrec::eval(Assoc &env)
{
	Assoc ee = env;

	for (auto &[x, y]:bind)
		ee = extend(x, Value(nullptr), ee);
	for (auto &[x, y]:bind)
		modify(x, y->eval(ee), ee);
	return body->eval(ee);
}                                 // letrec expression

Value Var::eval(Assoc &e)
{
	Value val = find(x, e);

	if (val.get() == nullptr)
		throw RuntimeError("varible not found");
	return val;
}                            // evaluation of variable

Value Fixnum::eval(Assoc &e)
{
	return Value(new Integer(n));
}                               // evaluation of a fixnum

Value If::eval(Assoc &e)
{
	Assoc ee = e;
	Value res = cond->eval(e);
	Boolean *Res = dynamic_cast <Boolean *> (res.get());

	if (Res->b == true)
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

Value Quote::eval(Assoc &e)
{
}                              // quote expression

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

	return Value(evalRator(val1, val2));
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
		throw RuntimeError("parameter error!");
	Integer *val1 = dynamic_cast <Integer *> (rand1.get());
	Integer *val2 = dynamic_cast <Integer *> (rand2.get());
	return Value(new Boolean((val1->n) >= (val2->n)));
}                                                                     // >=

Value Greater::evalRator(const Value &rand1, const Value &rand2)
{
	if (rand1->v_type != V_INT || rand2->v_type != V_INT)
		throw RuntimeError("parameter error!");
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
	Boolean *val = dynamic_cast <Boolean *> (rand.get());

	return Value(new Boolean(!val->b));
}                                          // not

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
