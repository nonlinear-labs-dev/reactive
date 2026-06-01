#include <reactive/Computations.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include <reactive/Computation.h>
#include <reactive/Deferrable.h>
#include <reactive/Deferrer.h>
#include <reactive/Invalidateable.h>

namespace Reactive
{
  struct Computations::Impl : std::enable_shared_from_this<Impl>, Invalidateable, Deferrable
  {

    void add(std::function<void()> &&cb);
    void invalidate(Computation *c) override;
    void resolveDirtynessDownstream() override;
    void doDeferred(Computation *c) override;
    Computation *getLowest(Computation *lowestSoFar) const override;

    std::vector<std::unique_ptr<Computation>> m_computations;
    std::vector<std::unique_ptr<Computation>> m_pending;
  };

  void Computations::Impl::add(std::function<void()> &&cb)
  {
    m_computations.push_back(std::make_unique<Computation>(*this, std::move(cb)));
    m_computations.back()->execute();
  }

  void Computations::Impl::invalidate(Computation *c)
  {

    if(const auto it = std::ranges::find_if(m_computations, [c](auto &m) { return m.get() == c; });
       it != m_computations.end())
    {
      m_pending.push_back(std::move(*it));
      m_computations.erase(it);
    }

    if(m_pending.size() == 1)
      Deferrer::add(shared_from_this());
  }

  void Computations::Impl::resolveDirtynessDownstream()
  {
  }

  void Computations::Impl::doDeferred(Computation *c)
  {

    if(const auto it = std::ranges::find_if(m_pending, [c](auto &m) { return m.get() == c; }); it != m_pending.end())
    {
      auto p = std::move(*it);
      m_pending.erase(it);
      auto cb = std::move(p->expropriateCallback());
      p.reset();
      add(std::move(cb));
    }
  }

  Computation *Computations::Impl::getLowest(Computation *lowestSoFar) const
  {
    for(const auto & it : m_pending)
      if(!lowestSoFar)
        lowestSoFar = it.get();
      else if(it->getDepth() < lowestSoFar->getDepth())
        lowestSoFar = it.get();
    return lowestSoFar;
  }

  Computations::Computations()
      : m_impl(std::make_shared<Impl>())
  {
  }

  Computations::~Computations() = default;

  void Computations::add(std::function<void()> &&cb) const
  {
    m_impl->add(std::move(cb));
  }
}
