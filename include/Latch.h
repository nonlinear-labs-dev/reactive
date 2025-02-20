#pragma once

#include "Invalidateable.h"
#include "Var.h"
#include "Computation.h"
#include "Deferrable.h"
#include "Deferrer.h"

namespace Reactive
{
  template <typename T> class Latch : public Invalidateable
  {
    struct _Deferrable : Deferrable
    {
      Latch& self;

      _Deferrable(Latch& _latch)
          : self(_latch)
      {
      }

      void doDeferred() override
      {
        self.execute();
      }
    };

   public:
    Latch()
        : m_deferrable(std::make_shared<_Deferrable>(*this))
    {
    }

    ~Latch()
    {
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
      m_dirty = true;
      Deferrer::add(m_deferrable);
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

    std::shared_ptr<Deferrable> m_deferrable;
    std::unique_ptr<Computation> m_computation;

    struct LatchVar : Var<T>
    {
      Latch& m_latch;
      LatchVar(Latch& latch)
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