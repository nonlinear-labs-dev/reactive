#pragma once

#include <memory>
#include <unordered_map>
#include <utility>

namespace Reactive
{
  class Computation;

  namespace Detail
  {
    class VarBase
    {
     public:
      VarBase();
      virtual ~VarBase();

      VarBase(VarBase &&) = delete;
      VarBase &operator=(VarBase const &) = delete;

      void unregisterComputation(Computation *c) const;

      virtual void resolveDirtynessDownstream() {};

     protected:
      void onReadAccess() const;
      void onWriteAccess() const;

     private:
      enum ComputationLifeCycleState
      {
        Alive,
        Doomed
      };
      using ComputationSet = std::unordered_map<Computation *, ComputationLifeCycleState>;
      mutable ComputationSet m_computations;
      mutable bool m_computationsLocked { false };
    };
  }

  template <typename T> class Var : public Detail::VarBase
  {
   public:
    Var() = default;

    Var(const T &init)
        : m_value(init)
    {
    }

    Var(T &&init)
        : m_value(std::move(init))
    {
    }

    ~Var() override = default;

    Var<T> &operator=(const T &v)
    {
      if(std::exchange(m_value, v) != v)
        onWriteAccess();
      return *this;
    }

    Var<T> &operator=(T &&v)
    {
      bool invalidate = true;
      if constexpr(requires { m_value != v; })
      {
        invalidate = m_value != v;
      }
      m_value = std::move(v);
      if(invalidate)
        onWriteAccess();
      return *this;
    }

    const T &peek() const
    {
      return m_value;
    }

    operator const T &() const
    {
      onReadAccess();
      return m_value;
    }

    const T &get() const
    {
      onReadAccess();
      return m_value;
    }

    bool operator==(const Var<T> &other) const
    {
      return m_value == other.m_value;
    }

    bool operator==(const T &other) const
    {
      return m_value == other;
    }

    template <typename F> void modify(const F &func)
    {
      func(m_value);
      onWriteAccess();
    }

   private:
    T m_value;

    template <typename> friend class Latch;
  };
}
