#include <catch2/catch_all.hpp>
#include <reactive/Computations.h>
#include <reactive/Deferrer.h>
#include <reactive/Latch.h>
#include <reactive/Var.h>

#include <memory>
#include <vector>

// These tests pin down the behaviour the Deferrer flush must preserve when a SINGLE
// deferred batch invalidates many computations at once - the rotation scenario - and,
// crucially, when re-running a computation creates NEW invalidations that must be
// processed in the correct (depth) order.

using namespace Reactive;

TEST_CASE("wide batch: every invalidated computation re-runs exactly once with correct value")
{
  constexpr int M = 50;

  std::vector<std::unique_ptr<Var<int>>> vars;
  for(int i = 0; i < M; i++)
    vars.push_back(std::make_unique<Var<int>>(i));

  // One Computations per "widget", exactly like the real UI.
  std::vector<std::unique_ptr<Computations>> widgets;
  std::vector<int> seen(M, -1);
  std::vector<int> runs(M, 0);

  for(int i = 0; i < M; i++)
  {
    auto w = std::make_unique<Computations>();
    w->add(
        [&, i]
        {
          seen[i] = vars[i]->get();
          runs[i]++;
        });
    widgets.push_back(std::move(w));
  }

  for(int i = 0; i < M; i++)
  {
    REQUIRE(seen[i] == i);
    REQUIRE(runs[i] == 1);
  }

  WHEN("all vars are changed inside one deferrer")
  {
    {
      Deferrer deferrer;
      for(int i = 0; i < M; i++)
        *vars[i] = i + 100;
    }

    THEN("each computation re-ran exactly once and holds the new value")
    {
      for(int i = 0; i < M; i++)
      {
        CHECK(seen[i] == i + 100);
        CHECK(runs[i] == 2);
      }
    }
  }
}

TEST_CASE("re-running a computation that writes a var propagates to its dependents")
{
  Var<int> a { 1 };
  Var<int> b { 0 };

  Computations producer;  // reads a, writes b
  Computations consumer;  // reads b

  int bSeen = -1;
  int producerRuns = 0;
  int consumerRuns = 0;

  producer.add(
      [&]
      {
        producerRuns++;
        b = a.get() * 10;
      });

  consumer.add(
      [&]
      {
        consumerRuns++;
        bSeen = b.get();
      });

  REQUIRE(bSeen == 10);
  REQUIRE(producerRuns == 1);
  REQUIRE(consumerRuns == 1);

  WHEN("the upstream var changes in a deferred batch")
  {
    {
      Deferrer deferrer;
      a = 5;
    }

    THEN("the dependent ends up with the freshly produced value")
    {
      CHECK(bSeen == 50);
      CHECK(producerRuns == 2);
      // consumer must run AFTER producer wrote b - so it sees the final value and
      // does not need a second pass. If ordering were wrong it would run twice.
      CHECK(consumerRuns == 2);
    }
  }
}

TEST_CASE("multi-hop propagation in one batch converges to correct final values")
{
  // a -> p1 (writes b) -> p2 (reads b, writes c) -> p3 (reads c)
  Var<int> a { 1 };
  Var<int> b { 0 };
  Var<int> c { 0 };

  Computations p1, p2, p3;
  int cSeen = -1;

  p1.add([&] { b = a.get() + 1; });
  p2.add([&] { c = b.get() + 1; });
  p3.add([&] { cSeen = c.get(); });

  REQUIRE(cSeen == 3);  // a=1 -> b=2 -> c=3

  {
    Deferrer deferrer;
    a = 10;
  }

  CHECK(cSeen == 12);  // a=10 -> b=11 -> c=12
}

TEST_CASE("two independent latch chains of different depth in one batch")
{
  // Two nested-latch structures with different nesting depth, both driven by the same
  // var, invalidated together. Each must recompute to the correct value regardless of
  // the order the Deferrer happens to visit them in.
  Var<int> var { 0 };

  Latch<int> deepInner, deepOuter;
  Latch<int> shallow;
  Computations deep, flat;

  int deepResult = 0;
  int flatResult = 0;

  deep.add(
      [&]
      {
        deepResult = deepOuter.doLatch([&] { return deepInner.doLatch([&] { return var.get() + 1; }); });
      });

  flat.add([&] { flatResult = shallow.doLatch([&] { return var.get() * 2; }); });

  REQUIRE(deepResult == 1);
  REQUIRE(flatResult == 0);

  {
    Deferrer deferrer;
    var = 7;
  }

  CHECK(deepResult == 8);   // 7 + 1
  CHECK(flatResult == 14);  // 7 * 2
}

TEST_CASE("destroying computations before deferrer flush drops pending callbacks")
{
  Var<int> v { 0 };
  int runs = 0;

  {
    Deferrer deferrer;
    {
      Computations widget;
      widget.add(
          [&]
          {
            runs++;
            (void)v.get();
          });
      REQUIRE(runs == 1);
      v = 1;
    }
  }

  CHECK(runs == 1);
}

TEST_CASE("cancelPending drops deferred callbacks without running them")
{
  Var<int> v { 0 };
  int runs = 0;

  auto widget = std::make_unique<Computations>();
  widget->add(
      [&]
      {
        runs++;
        (void)v.get();
      });
  REQUIRE(runs == 1);

  {
    Deferrer deferrer;
    v = 1;
    widget->cancelPending();
  }

  CHECK(runs == 1);
}
