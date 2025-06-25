#include <reactive/Var.h>
#include <reactive/Deferrer.h>
#include <reactive/Computation.h>

#include <cassert>

namespace Reactive::Detail
{
  VarBase::VarBase() = default;

  VarBase::~VarBase()
  {
    assert(!m_computationsLocked);
    for(auto c : m_computations)
      if(c.second == Alive)
        c.first->unregisterVar(this);
  }

  void VarBase::unregisterComputation(Computation *c) const
  {
    if(m_computationsLocked)
    {
      m_computations[c] = Doomed;
    }
    else
    {
      m_computations.erase(c);
    }
  }

  void VarBase::onReadAccess() const
  {
    if(auto c = Computation::getCurrentComputation())
    {
      assert(!m_computationsLocked);
      if(!m_computations.insert({ c, Alive }).second)
        m_computations[c] = Alive;
      c->registerVar(const_cast<VarBase *>(this));
    }
  }

  void VarBase::onWriteAccess() const
  {
    Deferrer deferrer;
    auto current = Computation::getCurrentComputation();

    m_computationsLocked = true;

    for(auto c : m_computations)
      if(c.first != current && c.second == Alive)
        c.first->invalidate();

    erase_if(m_computations, [](const auto &c) { return c.second == Doomed; });

    m_computationsLocked = false;
  }

}
