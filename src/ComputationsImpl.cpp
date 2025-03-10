#include <reactive/Deferrer.h>
#include <reactive/Computation.h>

#include "ComputationsImpl.h"

#include <algorithm>

namespace Reactive
{
  ComputationsImpl::ComputationsImpl() = default;
  ComputationsImpl::~ComputationsImpl() = default;

  void ComputationsImpl::add(std::function<void()> &&cb)
  {
    m_computations.push_back(std::make_unique<Computation>(*this, std::move(cb)));
    m_computations.back()->execute();
  }

  void ComputationsImpl::invalidate(Computation *c)
  {
    auto it = std::find_if(m_computations.begin(), m_computations.end(), [&](auto &m) { return m.get() == c; });

    if(it != m_computations.end())
    {
      m_pending.push_back(std::move(*it));
      m_computations.erase(it);
    }

    if(m_pending.size() == 1)
      Deferrer::add(shared_from_this());
  }

  void ComputationsImpl::resolveDirtynessDownstream()
  {
  }

  void ComputationsImpl::doDeferred(Computation *c)
  {
    auto it = std::find_if(m_pending.begin(), m_pending.end(), [&](auto &m) { return m.get() == c; });

    if(it != m_pending.end())
    {
      auto p = std::move(*it);
      m_pending.erase(it);
      auto cb = std::move(p->expropriateCallback());
      p.reset();
      add(std::move(cb));
    }
  }

  Computation *ComputationsImpl::getLowest(Computation *lowestSoFar) const
  {
    for(auto it = m_pending.begin(); it != m_pending.end(); it++)
      if(!lowestSoFar)
        lowestSoFar = it->get();
      else if((*it)->getDepth() < lowestSoFar->getDepth())
        lowestSoFar = it->get();
    return lowestSoFar;
  }
}
