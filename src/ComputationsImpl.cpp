#include <reactive/Deferrer.h>
#include <reactive/Computation.h>

#include "ComputationsImpl.h"

namespace Reactive
{
  ComputationsImpl::ComputationsImpl() = default;
  ComputationsImpl::~ComputationsImpl() = default;

  void ComputationsImpl::add(std::function<void()> &&cb)
  {
    auto computation = std::make_unique<Computation>(*this, std::move(cb));
    auto *raw = computation.get();
    m_computations.emplace(raw, std::move(computation));
    raw->execute();
  }

  void ComputationsImpl::invalidate(Computation *c)
  {
    if(const auto it = m_computations.find(c); it != m_computations.end())
    {
      auto node = std::move(it->second);
      auto *raw = node.get();
      m_computations.erase(it);

      const bool wasEmpty = m_pendingOrder.empty();
      m_pending.emplace(raw, std::move(node));
      m_pendingOrder.insert({ raw->getDepth(), raw });

      if(wasEmpty)
        Deferrer::add(shared_from_this());
    }
  }

  void ComputationsImpl::resolveDirtynessDownstream()
  {
  }

  void ComputationsImpl::cancelPending()
  {
    m_pendingOrder.clear();
    m_pending.clear();
  }

  void ComputationsImpl::doDeferred(Computation *c)
  {
    if(const auto it = m_pending.find(c); it != m_pending.end())
    {
      m_pendingOrder.erase({ c->getDepth(), c });
      auto cb = std::move(it->second->expropriateCallback());
      m_pending.erase(it);
      add(std::move(cb));
    }
  }

  Computation *ComputationsImpl::getLowest(Computation *lowestSoFar) const
  {
    if(m_pendingOrder.empty())
      return lowestSoFar;

    auto *my = m_pendingOrder.begin()->second;

    if(!lowestSoFar)
      return my;

    return my->getDepth() < lowestSoFar->getDepth() ? my : lowestSoFar;
  }
}
