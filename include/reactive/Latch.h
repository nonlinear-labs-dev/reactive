#pragma once

#include "Invalidateable.h"
#include "Var.h"
#include "Computation.h"
#include "Deferrable.h"
#include "Deferrer.h"

#include <memory>

namespace Reactive
{
  template <typename T> class Latch : public Invalidateable
  {
    struct LatchLifetime
    {
      bool alive = true;
    };

    struct _Deferrable : Deferrable
    {
      std::shared_ptr<LatchLifetime> m_lifetime;
      Latch& self;

      _Deferrable(std::shared_ptr<LatchLifetime> lifetime, Latch& latch)
          : m_lifetime(std::move(lifetime))
          , self(latch)
      {
      }

      void doDeferred(Computation*) override
      {
        if(m_lifetime->alive)
          self.execute();
      }

      Computation* getLowest(Computation* lowestSoFar) const override
      {
        if(m_lifetime->alive)
        {
          if(!self.m_dirty)
            self.resolveDirtynessDownstream();

          if(self.m_dirty)
          {
            auto my = self.m_computation.get();

            if(lowestSoFar)
              return my->getDepth() < lowestSoFar->getDepth() ? my : lowestSoFar;

            return my;
          }

          return lowestSoFar;
        }

        return nullptr;
      }
    };

   public:
    Latch()
        : m_lifetime(std::make_shared<LatchLifetime>())
        , m_deferrable(std::make_shared<_Deferrable>(m_lifetime, *this))
    {
    }

    ~Latch() override
    {
      m_lifetime->alive = false;
      m_computation.reset();
    }

    template <typename Producer> const T& doLatch(const Producer& producer)
    {
      if(!m_computation)
        m_computation = std::make_unique<Computation>(*this, [this, producer] { m_cache = producer(); });

      execute();
      return m_cache.get();
    }

    void invalidate(Computation* comp) override
    {
      if(m_lifetime->alive)
      {
        m_dirty = true;
        Deferrer::add(m_deferrable);
      }
    }

    void resolveDirtynessDownstream() override
    {
      if(m_computation)
        m_computation->resolveDirtynessDownstream();
    }

   private:
    void execute()
    {
      if(!m_dirty)
        resolveDirtynessDownstream();

      if(std::exchange(m_dirty, false))
      {
        if(m_computation)
        {
          m_computation = std::make_unique<Computation>(*this, m_computation->expropriateCallback());
          m_computation->execute();
        }
      }
    }

    std::shared_ptr<LatchLifetime> m_lifetime;
    std::shared_ptr<Deferrable> m_deferrable;
    std::unique_ptr<Computation> m_computation;

    struct LatchVar : Var<T>
    {
      Latch& m_latch;
      explicit LatchVar(Latch& latch)
          : m_latch(latch)
      {
      }

      void resolveDirtynessDownstream() override
      {
        m_latch.execute();
      }

      using Var<T>::operator=;

    } m_cache { *this };

    bool m_dirty = true;
  };
}
