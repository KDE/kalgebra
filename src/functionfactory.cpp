#include "functionfactory.h"

FunctionFactory* FunctionFactory::m_self=0;

bool FunctionFactory::contains(const FunctionFactory::Id& bvars) const
{
	return m_items.contains(bvars.join("|"));
}

FunctionImpl* FunctionFactory::item(const Id& bvars, const Expression& exp, Variables* v) const
{
	return m_items[bvars.join("|")](exp, v);
}

bool FunctionFactory::registerFunction(const FunctionFactory::Id& bvars, registerFunc_fn f)
{
	Q_ASSERT(!contains(bvars));
	m_items[bvars.join("|")]=f;
	return true;
}

FunctionFactory* FunctionFactory::self()
{
	if(!m_self)
		m_self=new FunctionFactory;
	return m_self;
}
