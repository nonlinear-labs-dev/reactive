#pragma once

#include <memory>
#include <queue>
#include <vector>

namespace Reactive
{
  class Deferrable;

  class Deferrer
  {
   public:
    Deferrer();
    ~Deferrer();

    static void add(const std::shared_ptr<Deferrable>& pending);

   private:
    using tDepth = int;
    using tPendingEntry = std::pair<tDepth, std::weak_ptr<Deferrable>>;

    struct Comperator
    {
      bool operator()(const tPendingEntry& lhs, const tPendingEntry& rhs) const
      {
        return lhs.first > rhs.first;
      }
    };

    std::priority_queue<tPendingEntry, std::vector<tPendingEntry>, Comperator> m_pending;

    friend struct DeferrerTester;
  };
}
