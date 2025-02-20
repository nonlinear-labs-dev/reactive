#include <catch2/catch_all.hpp>
#include <reactive/Computations.h>
#include <reactive/Deferrer.h>
#include <reactive/Latch.h>
#include <reactive/Var.h>

using namespace Reactive;

namespace Reactive
{
  struct DeferrerTester
  {
    static bool areElementsInPendingUnique(Deferrer &p)
    {
      for(auto &c : p.m_pending)
        if(std::count_if(p.m_pending.begin(), p.m_pending.end(), [&](auto &a) { return a.lock() == c.lock(); }) > 1)
          return false;

      return true;
    }
  };
}

TEST_CASE("reactive")
{
  WHEN("Variable is destroyed with pending computation")
  {
    auto shortLife = std::make_unique<Var<int>>(2);
    auto longLife = std::make_unique<Var<int>>(2);
    Computations computations;
    int longLifeValue = 0;

    computations.add(
        [&]
        {
          longLifeValue = longLife->get();

          if(shortLife)
            shortLife->get();
        });

    shortLife.reset();

    THEN("Computation can still be executed")
    {
      *longLife = 9;
      CHECK(longLifeValue == 9);
    }
  }

  WHEN("Variable is first written to and then read from in the same computation")
  {
    Var var(2);
    Computations computations;
    int readValue = 0;

    computations.add(
        [&]
        {
          var = 9;
          readValue = var.get();
        });

    THEN("value can be read without crashing")
    {
      CHECK(readValue == 9);
    }
  }

  WHEN("Variable is first read from and then written to in the same computation")
  {
    Var var(2);
    Computations computations;
    int readValue = 0;

    computations.add(
        [&]
        {
          readValue = var.get();
          var = 9;
        });

    THEN("value can be read without crashing")
    {
      CHECK(readValue == 2);
      CHECK(var == 9);
    }
  }

  WHEN("variable exists")
  {
    Var v = 1;

    AND_WHEN("Computation depends on variable")
    {
      int numCalls = 0;
      auto c = std::make_unique<Computations>();
      c->add([&] { numCalls++, v.get(); });

      REQUIRE(numCalls == 1);

      AND_WHEN("ComputationOwner still exists")
      {
        THEN("re-computation is done on var change")
        {
          v = 2;
          CHECK(numCalls == 2);
        }
      }

      AND_WHEN("ComputationOwner is already gone")
      {
        c.reset();

        THEN("changing var does not hurt")
        {
          v = 2;
          CHECK(numCalls == 1);
        }
      }
    }
  }

  WHEN("two vars exist")
  {
    Var<int> a;
    Var<int> b;

    AND_WHEN("one computation depends on both vars")
    {
      int computationCalled = 0;

      Computations c;
      c.add([&] { computationCalled++, a.get(), b.get(); });

      THEN("re-computation is done for changing either")
      {
        REQUIRE(computationCalled == 1);
        a = 2;
        CHECK(computationCalled == 2);
        b = 2;
        CHECK(computationCalled == 3);
      }

      AND_WHEN("both vars are changed in one step, with a deferrer")
      {
        {
          Reactive::Deferrer deferrer;
          a = 3, b = 3;
        }

        THEN("re-computation is done only once")
        {
          CHECK(computationCalled == 2);
        }
      }
    }
  }

  WHEN("ComputationsImpl contains two computations")
  {
    Deferrer deferrer;
    Computations computations;
    Var v { 2 };
    computations.add([&] { v.get(); });
    computations.add([&] { v.get(); });

    AND_WHEN("Both computations are invalidated")
    {
      v = 3;

      THEN("pending computations in Deferrer are unique")
      {
        CHECK(Reactive::DeferrerTester::areElementsInPendingUnique(deferrer));
      }
    }
  }

  WHEN("We have a nested structure")
  {
    Computations computations;

    Latch<int> latch0;
    Latch<int> latch1;
    Var<int> var { 0 };
    int result = 0;

    int depth0 = 0;
    int depth1 = 0;
    int depth2 = 0;
    int depth3 = 0;

    computations.add(
        [&]
        {
          depth0++;

          result = latch0.doLatch(
              [&]
              {
                depth1++;
                return latch1.doLatch(
                    [&]
                    {
                      depth2++;
                      return var.get() / 2;
                    });
              });
        });

    {
      Deferrer deferrer;
      var = 2;
    }

    CHECK(result == 1);
    CHECK(depth0 == 2);
    CHECK(depth1 == 2);
    CHECK(depth2 == 2);

    {
      Deferrer deferrer;
      var = 3;  // calculation result should not change
    }

    CHECK(result == 1);
    CHECK(depth0 == 2);
    CHECK(depth1 == 2);
    CHECK(depth2 == 3);

    {
      Deferrer deferrer;
      var = 4;
    }

    CHECK(result == 2);
    CHECK(depth0 == 3);
    CHECK(depth1 == 3);
    CHECK(depth2 == 4);
  }
}
