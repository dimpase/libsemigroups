//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2018 James D. Mitchell
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// The purpose of this file is to test the ToddCoxeter classes.

#include <utility>

#include "libsemigroups.tests.hpp"

#include "internal/report.h"

#include "bmat8.h"
#include "element-helper.h"
#include "element.h"
#include "semigroup.h"
#include "tce.h"
#include "todd-coxeter.h"

namespace libsemigroups {
  namespace congruence_todd_coxeter {

    template <class TElementType>
    void delete_gens(std::vector<TElementType>& gens) {
      for (auto x : gens) {
        delete x;
      }
    }

    using ToddCoxeter = congruence::ToddCoxeter;

    constexpr bool REPORT = false;

    congruence_type constexpr TWOSIDED = congruence_type::TWOSIDED;
    congruence_type constexpr LEFT     = congruence_type::LEFT;
    congruence_type constexpr RIGHT    = congruence_type::RIGHT;

    TEST_CASE("Todd-Coxeter 01: small fp semigroup",
              "[quick][todd-coxeter][01]") {
      REPORTER.set_report(REPORT);

      std::vector<relation_type> rels;
      rels.push_back(relation_type({0, 0, 0}, {0}));  // (a^3, a)
      rels.push_back(relation_type({0}, {1, 1}));     // (a, b^2)

      ToddCoxeter tc(TWOSIDED, 2, rels);

      REQUIRE(!tc.finished());

      REQUIRE(tc.nr_classes() == 5);
      REQUIRE(tc.finished());

      REQUIRE(tc.word_to_class_index({0, 0, 1})
              == tc.word_to_class_index({0, 0, 0, 0, 1}));
      REQUIRE(tc.word_to_class_index({0, 1, 1, 0, 0, 1})
              == tc.word_to_class_index({0, 0, 0, 0, 1}));
      REQUIRE(tc.word_to_class_index({0, 0, 0}) != tc.word_to_class_index({1}));
    }

    TEST_CASE("Todd-Coxeter 02: Example 6.6 in Sims (see also RWS 13)",
              "[standard][todd-coxeter][02]") {
      // TODO double check if this is standard or not
      REPORTER.set_report(REPORT);

      std::vector<relation_type> rels
          = {relation_type({0, 0}, {0}),
             relation_type({1, 0}, {1}),
             relation_type({0, 1}, {1}),
             relation_type({2, 0}, {2}),
             relation_type({0, 2}, {2}),
             relation_type({3, 0}, {3}),
             relation_type({0, 3}, {3}),
             relation_type({1, 1}, {0}),
             relation_type({2, 3}, {0}),
             relation_type({2, 2, 2}, {0}),
             relation_type({1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2}, {0}),
             relation_type({1, 2, 1, 3, 1, 2, 1, 3, 1, 2, 1, 3, 1, 2, 1, 3,
                            1, 2, 1, 3, 1, 2, 1, 3, 1, 2, 1, 3, 1, 2, 1, 3},
                           {0})};
      ToddCoxeter tc(TWOSIDED, 4, rels, {});
      tc.run_for(std::chrono::milliseconds(200));
      REQUIRE(tc.nr_classes() == 10752);

      SemigroupBase* S = tc.quotient_semigroup();
      REQUIRE(S->size() == 10752);
      REQUIRE(S->nridempotents() == 1);
      // RWS rws(S);
      // REQUIRE(rws.confluent());  // This is slow
    }

    TEST_CASE("Todd-Coxeter 03: ToddCoxeter constructed with Semigroup",
              "[quick][todd-coxeter][03]") {
      REPORTER.set_report(REPORT);

      Semigroup<BMat8> S(
          {BMat8({{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}),
           BMat8({{0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}}),
           BMat8({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}})});

      ToddCoxeter tc(TWOSIDED, S, ToddCoxeter::policy::use_relations);

      tc.add_pair({0}, {1});
      REQUIRE(tc.nr_classes() == 3);
    }

    TEST_CASE(
        "Todd-Coxeter 05: non-trivial two-sided constructed with Semigroup",
        "[quick][todd-coxeter][05]") {
      REPORTER.set_report(REPORT);

      using Transf = Transf<5>::type;
      Semigroup<Transf> S({Transf({1, 3, 4, 2, 3}), Transf({3, 2, 1, 3, 3})});

      REQUIRE(S.size() == 88);

      ToddCoxeter tc(TWOSIDED, S, ToddCoxeter::policy::use_cayley_graph);
      tc.add_pair(S.factorisation(Transf({3, 4, 4, 4, 4})),
                  S.factorisation(Transf({3, 1, 3, 3, 3})));

      REQUIRE(tc.nr_classes() == 21);
    }

    TEST_CASE("Todd-Coxeter 06: non-trivial two-sided constructed by relations",
              "[quick][todd-coxeter][06]") {
      REPORTER.set_report(REPORT);

      std::vector<relation_type> relations = {relation_type({0, 1}, {1, 0}),
                                              relation_type({0, 2}, {2, 2}),
                                              relation_type({0, 2}, {0}),
                                              relation_type({0, 2}, {0}),
                                              relation_type({2, 2}, {0}),
                                              relation_type({1, 2}, {1, 2}),
                                              relation_type({1, 2}, {2, 2}),
                                              relation_type({1, 2, 2}, {1}),
                                              relation_type({1, 2}, {1}),
                                              relation_type({2, 2}, {1})};
      std::vector<relation_type> extra     = {relation_type({0}, {1})};

      ToddCoxeter tc1(TWOSIDED, 3, relations, extra);
      REQUIRE(tc1.nr_classes() == 2);

      ToddCoxeter tc2(TWOSIDED, 3, relations, {});
      REQUIRE(tc2.nr_classes() == 2);
    }

    TEST_CASE("Todd-Coxeter 09: Small right congruence on free semigroup",
              "[quick][todd-coxeter][09]") {
      REPORTER.set_report(REPORT);

      std::vector<relation_type> rels;
      rels.push_back(relation_type({0, 0, 0}, {0}));  // (a^3, a)
      rels.push_back(relation_type({0}, {1, 1}));     // (a, b^2)
      std::vector<relation_type> extra;

      ToddCoxeter tc(RIGHT, 2, rels, extra);
      REQUIRE(tc.nr_classes() == 5);
      REQUIRE(tc.finished());
    }

    TEST_CASE("Todd-Coxeter 10: word_to_class_index for left "
              "congruence on free semigroup",
              "[quick][todd-coxeter][10]") {
      REPORTER.set_report(REPORT);

      std::vector<relation_type> rels;
      rels.push_back(relation_type({0, 0, 0}, {0}));  // (a^3, a)
      rels.push_back(relation_type({0}, {1, 1}));     // (a, b^2)
      std::vector<relation_type> extra;

      ToddCoxeter tc(LEFT, 2, rels, extra);
      REQUIRE(tc.word_to_class_index({0, 0, 1})
              == tc.word_to_class_index({0, 0, 0, 0, 1}));
      REQUIRE(tc.word_to_class_index({0, 1, 1, 0, 0, 1})
              == tc.word_to_class_index({0, 0, 0, 0, 1}));
      REQUIRE(tc.word_to_class_index({1})
              != tc.word_to_class_index({0, 0, 0, 0}));
      REQUIRE(tc.word_to_class_index({0, 0, 0})
              != tc.word_to_class_index({0, 0, 0, 0}));
    }

    TEST_CASE("Todd-Coxeter 11: word_to_class_index for small fp semigroup",
              "[quick][todd-coxeter][11]") {
      REPORTER.set_report(REPORT);
      std::vector<relation_type> rels;
      rels.push_back(relation_type({0, 0, 0}, {0}));  // (a^3, a)
      rels.push_back(relation_type({0}, {1, 1}));     // (a, b^2)
      std::vector<relation_type> extra;

      ToddCoxeter tc1(TWOSIDED, 2, rels, extra);
      REQUIRE(tc1.word_to_class_index({0, 0, 1})
              == tc1.word_to_class_index({0, 0, 0, 0, 1}));
      REQUIRE(tc1.word_to_class_index({0, 1, 1, 0, 0, 1})
              == tc1.word_to_class_index({0, 0, 0, 0, 1}));
      REQUIRE(tc1.word_to_class_index({0, 0, 0})
              != tc1.word_to_class_index({1}));

      ToddCoxeter tc2(TWOSIDED, 2, rels, extra);

      REQUIRE(tc2.word_to_class_index({0, 0, 0, 0}) < tc2.nr_classes());
    }

    TEST_CASE("Todd-Coxeter 12: 2-sided cong. trans. semigroup",
              "[quick][todd-coxeter][12]") {
      REPORTER.set_report(REPORT);
      auto S = Semigroup<Transformation<u_int16_t>>(
          {Transformation<u_int16_t>({1, 3, 4, 2, 3}),
           Transformation<u_int16_t>({3, 2, 1, 3, 3})});

      REQUIRE(S.size() == 88);
      REQUIRE(S.nrrules() == 18);

      ToddCoxeter tc(TWOSIDED, S, ToddCoxeter::policy::use_relations);
      tc.add_pair(S.factorisation(Transformation<u_int16_t>({3, 4, 4, 4, 4})),
                  S.factorisation(Transformation<u_int16_t>({3, 1, 3, 3, 3})));

      REQUIRE(tc.nr_classes() == 21);
      REQUIRE(tc.nr_classes() == 21);

      REQUIRE(tc.word_to_class_index(
                  S.factorisation(Transformation<u_int16_t>({1, 3, 1, 3, 3})))
              == tc.word_to_class_index(S.factorisation(
                     Transformation<u_int16_t>({4, 2, 4, 4, 2}))));

      REQUIRE(tc.nr_non_trivial_classes() == 1);
      REQUIRE(tc.cbegin_ntc()->size() == 68);
    }

    TEST_CASE("Todd-Coxeter 13: left cong. trans. semigroup",
              "[quick][todd-coxeter][13]") {
      REPORTER.set_report(REPORT);
      auto S = Semigroup<Transformation<u_int16_t>>(
          {Transformation<u_int16_t>({1, 3, 4, 2, 3}),
           Transformation<u_int16_t>({3, 2, 1, 3, 3})});

      REQUIRE(S.size() == 88);
      REQUIRE(S.nrrules() == 18);

      ToddCoxeter tc(LEFT, S, ToddCoxeter::policy::use_relations);
      tc.add_pair(S.factorisation(Transformation<u_int16_t>({3, 4, 4, 4, 4})),
                  S.factorisation(Transformation<u_int16_t>({3, 1, 3, 3, 3})));

      REQUIRE(tc.nr_classes() == 69);
      REQUIRE(tc.nr_classes() == 69);

      REQUIRE(tc.word_to_class_index(
                  S.factorisation(Transformation<u_int16_t>({1, 3, 1, 3, 3})))
              != tc.word_to_class_index(S.factorisation(
                     Transformation<u_int16_t>({4, 2, 4, 4, 2}))));

      REQUIRE(tc.nr_non_trivial_classes() == 1);
      REQUIRE(tc.cbegin_ntc()->size() == 20);
    }

    TEST_CASE("Todd-Coxeter 14: right cong. trans. semigroup",
              "[quick][todd-coxeter][13]") {
      REPORTER.set_report(REPORT);
      auto S = Semigroup<Transformation<u_int16_t>>(
          {Transformation<u_int16_t>({1, 3, 4, 2, 3}),
           Transformation<u_int16_t>({3, 2, 1, 3, 3})});

      REQUIRE(S.size() == 88);
      REQUIRE(S.nrrules() == 18);

      ToddCoxeter tc(RIGHT, S, ToddCoxeter::policy::use_relations);
      tc.add_pair(S.factorisation(Transformation<u_int16_t>({3, 4, 4, 4, 4})),
                  S.factorisation(Transformation<u_int16_t>({3, 1, 3, 3, 3})));

      REQUIRE(tc.nr_classes() == 72);
      REQUIRE(tc.nr_classes() == 72);

      REQUIRE(tc.word_to_class_index(
                  S.factorisation(Transformation<u_int16_t>({1, 3, 1, 3, 3})))
              != tc.word_to_class_index(S.factorisation(
                     Transformation<u_int16_t>({4, 2, 4, 4, 2}))));

      REQUIRE(tc.word_to_class_index(
                  S.factorisation(Transformation<u_int16_t>({1, 3, 3, 3, 3})))
              != tc.word_to_class_index(S.factorisation(
                     Transformation<u_int16_t>({4, 2, 4, 4, 2}))));
      REQUIRE(tc.word_to_class_index(
                  S.factorisation(Transformation<u_int16_t>({2, 4, 2, 2, 2})))
              == tc.word_to_class_index(S.factorisation(
                     Transformation<u_int16_t>({2, 3, 3, 3, 3}))));
      REQUIRE(tc.word_to_class_index(
                  S.factorisation(Transformation<u_int16_t>({1, 3, 3, 3, 3})))
              != tc.word_to_class_index(S.factorisation(
                     Transformation<u_int16_t>({2, 3, 3, 3, 3}))));

      REQUIRE(tc.nr_non_trivial_classes() == 4);

      std::vector<size_t> v(tc.nr_non_trivial_classes(), 0);
      std::transform(tc.cbegin_ntc(),
                     tc.cend_ntc(),
                     v.begin(),
                     std::mem_fn(&std::vector<word_type>::size));
      REQUIRE(std::count(v.cbegin(), v.cend(), 3) == 1);
      REQUIRE(std::count(v.cbegin(), v.cend(), 5) == 2);
      REQUIRE(std::count(v.cbegin(), v.cend(), 7) == 1);
    }

    TEST_CASE("Todd-Coxeter 15: transformation semigroup size 88",
              "[quick][todd-coxeter][15]") {
      REPORTER.set_report(REPORT);

      std::vector<Element*> gens
          = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
             new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
      Semigroup<> S = Semigroup<>(gens);
      delete_gens(gens);

      REQUIRE(S.size() == 88);
      REQUIRE(S.nrrules() == 18);
      REQUIRE(S.degree() == 5);

      ToddCoxeter tc(TWOSIDED, S, ToddCoxeter::policy::use_cayley_graph);

      Element*  t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
      Element*  t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
      word_type w1, w2;
      S.factorisation(w1, S.position(t1));
      S.factorisation(w2, S.position(t2));

      tc.add_pair(w1, w2);

      REQUIRE(tc.nr_classes() == 21);
      REQUIRE(tc.nr_classes() == 21);
      Element*  t3 = new Transformation<u_int16_t>({1, 3, 1, 3, 3});
      Element*  t4 = new Transformation<u_int16_t>({4, 2, 4, 4, 2});
      word_type w3, w4;
      S.factorisation(w3, S.position(t3));
      S.factorisation(w4, S.position(t4));
      REQUIRE(tc.word_to_class_index(w3) == tc.word_to_class_index(w4));

      delete t1;
      delete t2;
      delete t3;
      delete t4;
    }

    TEST_CASE(
        "Todd-Coxeter 16: left congruence on transformation semigroup size 88 ",
        "[quick][todd-coxeter][16]") {
      std::vector<Element*> gens
          = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
             new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
      Semigroup<> S = Semigroup<>(gens);
      REPORTER.set_report(REPORT);
      delete_gens(gens);

      REQUIRE(S.size() == 88);
      REQUIRE(S.degree() == 5);
      Element*  t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
      Element*  t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
      word_type w1, w2;
      S.factorisation(w1, S.position(t1));
      S.factorisation(w2, S.position(t2));
      ToddCoxeter tc(LEFT, &S, ToddCoxeter::policy::use_relations);
      tc.add_pair(w1, w2);

      REPORTER.set_report(REPORT);

      REQUIRE(tc.nr_classes() == 69);
      REQUIRE(tc.nr_classes() == 69);
      Element*  t3 = new Transformation<u_int16_t>({1, 3, 1, 3, 3});
      Element*  t4 = new Transformation<u_int16_t>({4, 2, 4, 4, 2});
      word_type w3, w4;
      S.factorisation(w3, S.position(t3));
      S.factorisation(w4, S.position(t4));

      delete t1;
      delete t2;
      delete t3;
      delete t4;
    }

    TEST_CASE("Todd-Coxeter 17: right congruence on transformation semigroup "
              "size 88 ",
              "[quick][todd-coxeter][17]") {
      std::vector<Element*> gens
          = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
             new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
      Semigroup<> S = Semigroup<>(gens);
      REPORTER.set_report(REPORT);
      delete_gens(gens);

      REQUIRE(S.size() == 88);
      REQUIRE(S.nrrules() == 18);
      REQUIRE(S.degree() == 5);
      Element*  t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
      Element*  t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
      word_type w1, w2;
      S.factorisation(w1, S.position(t1));
      S.factorisation(w2, S.position(t2));
      ToddCoxeter tc(RIGHT, S, ToddCoxeter::policy::use_relations);
      tc.add_pair(w1, w2);

      REPORTER.set_report(REPORT);

      REQUIRE(tc.nr_classes() == 72);
      REQUIRE(tc.nr_classes() == 72);
      Element*  t3 = new Transformation<u_int16_t>({1, 3, 3, 3, 3});
      Element*  t4 = new Transformation<u_int16_t>({4, 2, 4, 4, 2});
      Element*  t5 = new Transformation<u_int16_t>({2, 4, 2, 2, 2});
      Element*  t6 = new Transformation<u_int16_t>({2, 3, 3, 3, 3});
      word_type w3, w4, w5, w6;
      S.factorisation(w3, S.position(t3));
      S.factorisation(w4, S.position(t4));
      S.factorisation(w5, S.position(t5));
      S.factorisation(w6, S.position(t6));
      REQUIRE(tc.word_to_class_index(w3) != tc.word_to_class_index(w4));
      REQUIRE(tc.word_to_class_index(w5) == tc.word_to_class_index(w6));
      REQUIRE(tc.word_to_class_index(w3) != tc.word_to_class_index(w6));

      delete t1;
      delete t2;
      delete t3;
      delete t4;
      delete t5;
      delete t6;
    }

    TEST_CASE("Todd-Coxeter 18: finite fp-semigroup, dihedral group of order 6",
              "[quick][todd-coxeter][18]") {
      REPORTER.set_report(REPORT);
      std::vector<relation_type> rels  = {relation_type({0, 0}, {0}),
                                         relation_type({0, 1}, {1}),
                                         relation_type({1, 0}, {1}),
                                         relation_type({0, 2}, {2}),
                                         relation_type({2, 0}, {2}),
                                         relation_type({0, 3}, {3}),
                                         relation_type({3, 0}, {3}),
                                         relation_type({0, 4}, {4}),
                                         relation_type({4, 0}, {4}),
                                         relation_type({1, 2}, {0}),
                                         relation_type({2, 1}, {0}),
                                         relation_type({3, 4}, {0}),
                                         relation_type({4, 3}, {0}),
                                         relation_type({2, 2}, {0}),
                                         relation_type({1, 4, 2, 3, 3}, {0}),
                                         relation_type({4, 4, 4}, {0})};
      std::vector<relation_type> extra = {};

      ToddCoxeter tc(TWOSIDED, 5, rels, extra);
      REQUIRE(tc.nr_classes() == 6);
      REQUIRE(tc.word_to_class_index({1}) == tc.word_to_class_index({2}));
    }

    TEST_CASE("Todd-Coxeter 19: finite fp-semigroup, size 16",
              "[quick][todd-coxeter][19]") {
      REPORTER.set_report(REPORT);
      std::vector<relation_type> rels
          = {relation_type({3}, {2}),
             relation_type({0, 3}, {0, 2}),
             relation_type({1, 1}, {1}),
             relation_type({1, 3}, {1, 2}),
             relation_type({2, 1}, {2}),
             relation_type({2, 2}, {2}),
             relation_type({2, 3}, {2}),
             relation_type({0, 0, 0}, {0}),
             relation_type({0, 0, 1}, {1}),
             relation_type({0, 0, 2}, {2}),
             relation_type({0, 1, 2}, {1, 2}),
             relation_type({1, 0, 0}, {1}),
             relation_type({1, 0, 2}, {0, 2}),
             relation_type({2, 0, 0}, {2}),
             relation_type({0, 1, 0, 1}, {1, 0, 1}),
             relation_type({0, 2, 0, 2}, {2, 0, 2}),
             relation_type({1, 0, 1, 0}, {1, 0, 1}),
             relation_type({1, 2, 0, 1}, {1, 0, 1}),
             relation_type({1, 2, 0, 2}, {2, 0, 2}),
             relation_type({2, 0, 1, 0}, {2, 0, 1}),
             relation_type({2, 0, 2, 0}, {2, 0, 2})};
      std::vector<relation_type> extra = {};
      ToddCoxeter                tc(TWOSIDED, 4, rels, extra);

      REQUIRE(tc.nr_classes() == 16);
      REQUIRE(tc.word_to_class_index({2}) == tc.word_to_class_index({3}));
    }

    TEST_CASE("Todd-Coxeter 20: finite fp-semigroup, size 16",
              "[quick][todd-coxeter][20]") {
      REPORTER.set_report(REPORT);
      std::vector<relation_type> rels
          = {relation_type({2}, {1}),
             relation_type({4}, {3}),
             relation_type({5}, {0}),
             relation_type({6}, {3}),
             relation_type({7}, {1}),
             relation_type({8}, {3}),
             relation_type({9}, {3}),
             relation_type({10}, {0}),
             relation_type({0, 2}, {0, 1}),
             relation_type({0, 4}, {0, 3}),
             relation_type({0, 5}, {0, 0}),
             relation_type({0, 6}, {0, 3}),
             relation_type({0, 7}, {0, 1}),
             relation_type({0, 8}, {0, 3}),
             relation_type({0, 9}, {0, 3}),
             relation_type({0, 10}, {0, 0}),
             relation_type({1, 1}, {1}),
             relation_type({1, 2}, {1}),
             relation_type({1, 4}, {1, 3}),
             relation_type({1, 5}, {1, 0}),
             relation_type({1, 6}, {1, 3}),
             relation_type({1, 7}, {1}),
             relation_type({1, 8}, {1, 3}),
             relation_type({1, 9}, {1, 3}),
             relation_type({1, 10}, {1, 0}),
             relation_type({3, 1}, {3}),
             relation_type({3, 2}, {3}),
             relation_type({3, 3}, {3}),
             relation_type({3, 4}, {3}),
             relation_type({3, 5}, {3, 0}),
             relation_type({3, 6}, {3}),
             relation_type({3, 7}, {3}),
             relation_type({3, 8}, {3}),
             relation_type({3, 9}, {3}),
             relation_type({3, 10}, {3, 0}),
             relation_type({0, 0, 0}, {0}),
             relation_type({0, 0, 1}, {1}),
             relation_type({0, 0, 3}, {3}),
             relation_type({0, 1, 3}, {1, 3}),
             relation_type({1, 0, 0}, {1}),
             relation_type({1, 0, 3}, {0, 3}),
             relation_type({3, 0, 0}, {3}),
             relation_type({0, 1, 0, 1}, {1, 0, 1}),
             relation_type({0, 3, 0, 3}, {3, 0, 3}),
             relation_type({1, 0, 1, 0}, {1, 0, 1}),
             relation_type({1, 3, 0, 1}, {1, 0, 1}),
             relation_type({1, 3, 0, 3}, {3, 0, 3}),
             relation_type({3, 0, 1, 0}, {3, 0, 1}),
             relation_type({3, 0, 3, 0}, {3, 0, 3})};
      std::vector<relation_type> extra = {};

      ToddCoxeter tc(TWOSIDED, 11, rels, extra);

      REQUIRE(tc.nr_classes() == 16);
      REQUIRE(tc.word_to_class_index({0}) == tc.word_to_class_index({5}));
      REQUIRE(tc.word_to_class_index({0}) == tc.word_to_class_index({10}));
      REQUIRE(tc.word_to_class_index({1}) == tc.word_to_class_index({2}));
      REQUIRE(tc.word_to_class_index({1}) == tc.word_to_class_index({7}));
      REQUIRE(tc.word_to_class_index({3}) == tc.word_to_class_index({4}));
      REQUIRE(tc.word_to_class_index({3}) == tc.word_to_class_index({6}));
      REQUIRE(tc.word_to_class_index({3}) == tc.word_to_class_index({8}));
      REQUIRE(tc.word_to_class_index({3}) == tc.word_to_class_index({9}));
    }

    TEST_CASE("Todd-Coxeter 21: test prefilling of the table manually",
              "[quick][todd-coxeter][21]") {
      REPORTER.set_report(REPORT);
      std::vector<Element*> gens
          = {new Transformation<u_int16_t>({7, 3, 5, 3, 4, 2, 7, 7}),
             new Transformation<u_int16_t>({3, 6, 3, 4, 0, 6, 0, 7})};
      Semigroup<> S = Semigroup<>(gens);
      delete_gens(gens);

      // Copy the right Cayley graph of S for prefilling
      Semigroup<>::cayley_graph_type const* right = S.right_cayley_graph_copy();
      RecVec<size_t>                        table(S.nrgens(), 1, UNDEFINED);
      table.append(*right);
      delete right;
      // TODO move this stuff into prefill
      size_t j = 1;
      std::for_each(table.begin(),
                    table.begin() + table.nr_cols(),
                    [&j](size_t& i) { i = j++; });
      std::for_each(
          table.begin() + table.nr_cols(), table.end(), [](size_t& i) { ++i; });

      ToddCoxeter tc(TWOSIDED, 2, {}, {});
      REQUIRE(tc.get_policy() == ToddCoxeter::policy::none);
      tc.prefill(table);
      REQUIRE(!tc.is_quotient_obviously_infinite());
      REQUIRE(tc.nr_classes() == S.size());
    }

    TEST_CASE("Todd-Coxeter 22: test packing phase",
              "[quick][todd-coxeter][22]") {
      REPORTER.set_report(REPORT);
      std::vector<relation_type> rels
          = {relation_type({0, 0, 0}, {0}),
             relation_type({1, 0, 0}, {1, 0}),
             relation_type({1, 0, 1, 1, 1}, {1, 0}),
             relation_type({1, 1, 1, 1, 1}, {1, 1}),
             relation_type({1, 1, 0, 1, 1, 0}, {1, 0, 1, 0, 1, 1}),
             relation_type({0, 0, 1, 0, 1, 1, 0}, {0, 1, 0, 1, 1, 0}),
             relation_type({0, 0, 1, 1, 0, 1, 0}, {0, 1, 1, 0, 1, 0}),
             relation_type({0, 1, 0, 1, 0, 1, 0}, {1, 0, 1, 0, 1, 0}),
             relation_type({1, 0, 1, 0, 1, 0, 1}, {1, 0, 1, 0, 1, 0}),
             relation_type({1, 0, 1, 0, 1, 1, 0}, {1, 0, 1, 0, 1, 1}),
             relation_type({1, 0, 1, 1, 0, 1, 0}, {1, 0, 1, 1, 0, 1}),
             relation_type({1, 1, 0, 1, 0, 1, 0}, {1, 0, 1, 0, 1, 0}),
             relation_type({1, 1, 1, 1, 0, 1, 0}, {1, 0, 1, 0}),
             relation_type({0, 0, 1, 1, 1, 0, 1, 0}, {1, 1, 1, 0, 1, 0})};

      ToddCoxeter tc1(TWOSIDED, 2, rels, std::vector<relation_type>());
      tc1.set_pack(10);
      REQUIRE(tc1.nr_classes() == 78);

      ToddCoxeter tc2(LEFT, 2, rels, std::vector<relation_type>());
      tc2.set_pack(10);
      REQUIRE(tc2.nr_classes() == 78);
    }

    TEST_CASE("Todd-Coxeter 24: non-trivial left congruence constructed with "
              "semigroup",
              "[quick][todd-coxeter][24]") {
      REPORTER.set_report(REPORT);

      std::vector<Element*> gens
          = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
             new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
      Semigroup<> S = Semigroup<>(gens);
      delete_gens(gens);

      REQUIRE(S.size() == 88);
      REQUIRE(S.degree() == 5);

      Element*  t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
      Element*  t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
      word_type w1, w2;
      S.factorisation(w1, S.position(t1));
      S.factorisation(w2, S.position(t2));
      delete t1;
      delete t2;

      ToddCoxeter tc(LEFT, S, ToddCoxeter::policy::use_cayley_graph);
      tc.add_pair(w1, w2);
      REQUIRE(tc.nr_classes() == 69);
    }

    TEST_CASE("Todd-Coxeter 25: 2-sided congruence on free semigroup",
              "[quick][todd-coxeter][25]") {
      REPORTER.set_report(REPORT);
      ToddCoxeter tc(TWOSIDED, 1, {});
      REQUIRE(tc.contains({0, 0}, {0, 0}));
      REQUIRE(!tc.contains({0, 0}, {0}));
    }

    TEST_CASE("Todd-Coxeter 27: calling run when obviously infinite",
              "[quick][todd-coxeter][27]") {
      ToddCoxeter tc(TWOSIDED, 5, {});
      REQUIRE_THROWS_AS(tc.run(), LibsemigroupsException);
    }

    LIBSEMIGROUPS_TEST_CASE("Todd-Coxeter 030",
                            "Stellar S3",
                            "[quick][hivert]") {
      REPORTER.set_report(REPORT);

      congruence::ToddCoxeter tc(TWOSIDED);
      tc.set_nr_generators(4);
      tc.add_pair({3, 3}, {3});
      tc.add_pair({0, 3}, {0});
      tc.add_pair({3, 0}, {0});
      tc.add_pair({1, 3}, {1});
      tc.add_pair({3, 1}, {1});
      tc.add_pair({2, 3}, {2});
      tc.add_pair({3, 2}, {2});
      tc.add_pair({0, 0}, {0});
      tc.add_pair({1, 1}, {1});
      tc.add_pair({2, 2}, {2});
      tc.add_pair({0, 2}, {2, 0});
      tc.add_pair({2, 0}, {0, 2});
      tc.add_pair({1, 2, 1}, {2, 1, 2});
      tc.add_pair({1, 0, 1, 0}, {0, 1, 0, 1});
      tc.add_pair({1, 0, 1, 0}, {0, 1, 0});

      REQUIRE(tc.nr_classes() == 34);
      REQUIRE(tc.quotient_semigroup()->size() == 34);

      auto S = static_cast<Semigroup<TCE>*>(tc.quotient_semigroup());
      S->enumerate();
      std::vector<TCE> v(S->cbegin(), S->cend());
      std::sort(v.begin(), v.end());
      REQUIRE(v
              == std::vector<TCE>(
                     {TCE(tc, 1),  TCE(tc, 2),  TCE(tc, 3),  TCE(tc, 4),
                      TCE(tc, 5),  TCE(tc, 6),  TCE(tc, 7),  TCE(tc, 8),
                      TCE(tc, 9),  TCE(tc, 10), TCE(tc, 11), TCE(tc, 12),
                      TCE(tc, 13), TCE(tc, 14), TCE(tc, 15), TCE(tc, 16),
                      TCE(tc, 17), TCE(tc, 18), TCE(tc, 19), TCE(tc, 20),
                      TCE(tc, 21), TCE(tc, 22), TCE(tc, 23), TCE(tc, 24),
                      TCE(tc, 25), TCE(tc, 26), TCE(tc, 27), TCE(tc, 28),
                      TCE(tc, 29), TCE(tc, 30), TCE(tc, 31), TCE(tc, 32),
                      TCE(tc, 33), TCE(tc, 34)}));
    }

    TEST_CASE("Todd-Coxeter 31: finite semigroup (size 5)",
              "[todd-coxeter][quick][31]") {
      congruence::ToddCoxeter tc(LEFT);
      tc.set_nr_generators(2);
      tc.add_pair({0, 0, 0}, {0});  // (a^3, a)
      tc.add_pair({0}, {1, 1});     // (a, b^2)
      REQUIRE(tc.nr_classes() == 5);
    }
  }  // namespace congruence_todd_coxeter

  namespace fpsemigroup_todd_coxeter {
    using ToddCoxeter     = fpsemigroup::ToddCoxeter;
    constexpr bool REPORT = false;

    TEST_CASE("Todd-Coxeter 04: add_rule ", "[quick][todd-coxeter][04]") {
      REPORTER.set_report(REPORT);
      ToddCoxeter tc;
      tc.set_alphabet("ab");
      tc.add_rule("aaa", "a");
      tc.add_rule("a", "bb");

      REQUIRE(tc.size() == 5);
    }

    // RWS methods fail for this one
    TEST_CASE("Todd-Coxeter 07: (from kbmag/standalone/kb_data/s4) (RWS 49)",
              "[quick][todd-coxeter][07]") {
      REPORTER.set_report(REPORT);

      ToddCoxeter tc;
      tc.set_alphabet("abcd");
      tc.add_rule("bb", "c");
      tc.add_rule("caca", "abab");
      tc.add_rule("bc", "d");
      tc.add_rule("cb", "d");
      tc.add_rule("aa", "d");
      tc.add_rule("ad", "a");
      tc.add_rule("da", "a");
      tc.add_rule("bd", "b");
      tc.add_rule("db", "b");
      tc.add_rule("cd", "c");
      tc.add_rule("dc", "c");
      REQUIRE(tc.size() == 24);
      REQUIRE(tc.isomorphic_non_fp_semigroup()->size() == 24);
      REQUIRE(tc.normal_form("aaaaaaaaaaaaaaaaaaa") == "a");
      // TODO uncomment the next line
      //  REQUIRE(RWS(tc.isomorphic_non_fp_semigroup()).confluent());
    }

    // Second of BHN's series of increasingly complicated presentations
    // of 1. Doesn't terminate
    TEST_CASE("Todd-Coxeter 08: (from kbmag/standalone/kb_data/degen4b) "
              "(RWS 65)",
              "[extreme][todd-coxeter][kbmag][shortlex][08]") {
      REPORTER.set_report(REPORT);

      ToddCoxeter tc;
      tc.set_alphabet("abcdefg");
      // Inverses . . .
      tc.add_rule("ad", "g");
      tc.add_rule("da", "g");
      tc.add_rule("be", "g");
      tc.add_rule("eb", "g");
      tc.add_rule("cf", "g");
      tc.add_rule("fc", "g");

      // Identity . . .
      tc.add_rule("ag", "a");
      tc.add_rule("bg", "b");
      tc.add_rule("cg", "c");
      tc.add_rule("dg", "d");
      tc.add_rule("eg", "e");
      tc.add_rule("fg", "f");
      tc.add_rule("ga", "a");
      tc.add_rule("gb", "b");
      tc.add_rule("gc", "c");
      tc.add_rule("gd", "d");
      tc.add_rule("ge", "e");
      tc.add_rule("gf", "f");
      tc.add_rule("gg", "g");

      tc.add_rule("bbdeaecbffdbaeeccefbccefb", "g");
      tc.add_rule("ccefbfacddecbffaafdcaafdc", "g");
      tc.add_rule("aafdcdbaeefacddbbdeabbdea", "g");

      REQUIRE(tc.size() == 1);
      REQUIRE(tc.isomorphic_non_fp_semigroup()->size() == 1);
      // TODO uncomment the next line
      // REQUIRE(RWS(tc.isomorphic_non_fp_semigroup()).confluent());
    }

    TEST_CASE("Todd-Coxeter 23: test validate", "[quick][todd-coxeter][23]") {
      REPORTER.set_report(REPORT);

      ToddCoxeter tc;
      tc.set_alphabet("ab");
      tc.add_rule("a", "b");
      tc.add_rule("bb", "b");

      REQUIRE_THROWS_AS(tc.add_rule("b", "c"), LibsemigroupsException);
    }

    TEST_CASE("Todd-Coxeter 26: add_rules after constructed from semigroup",
              "[quick][todd-coxeter][26]") {
      REPORTER.set_report(REPORT);

      using Transf = Transf<5>::type;

      Semigroup<Transf> S({Transf({1, 3, 4, 2, 3}), Transf({3, 2, 1, 3, 3})});
      REQUIRE(S.size() == 88);
      REQUIRE(S.nrrules() == 18);

      word_type w1, w2, w3, w4;
      S.factorisation(w1, S.position(Transf({3, 4, 4, 4, 4})));
      S.factorisation(w2, S.position(Transf({3, 1, 3, 3, 3})));
      S.factorisation(w3, S.position(Transf({1, 3, 1, 3, 3})));
      S.factorisation(w4, S.position(Transf({4, 2, 4, 4, 2})));

      ToddCoxeter tc1(S);
      tc1.add_rule(w1, w2);

      REQUIRE(tc1.size() == 21);
      REQUIRE(tc1.size() == tc1.isomorphic_non_fp_semigroup()->size());
      REQUIRE(tc1.equal_to(w3, w4));
      REQUIRE(tc1.normal_form(w3) == tc1.normal_form(w4));

      ToddCoxeter tc2(S);
      tc2.add_rule(w1, w2);

      REQUIRE(tc2.size() == 21);
      REQUIRE(tc2.size() == tc2.isomorphic_non_fp_semigroup()->size());
      REQUIRE(tc2.equal_to(w3, w4));
      REQUIRE(tc2.normal_form(w3) == tc2.normal_form(w4));
    }

    TEST_CASE("Todd-Coxeter 28: Sym(5) from Chapter 3, Proposition 1.1 in NR",
              "[todd-coxeter][quick][28]") {
      REPORTER.set_report(REPORT);

      ToddCoxeter tc("ABabe");
      tc.set_identity("e");
      tc.add_rule("aa", "e");
      tc.add_rule("bbbbb", "e");
      tc.add_rule("babababa", "e");
      tc.add_rule("bB", "e");
      tc.add_rule("Bb", "e");
      tc.add_rule("BabBab", "e");
      tc.add_rule("aBBabbaBBabb", "e");
      tc.add_rule("aBBBabbbaBBBabbb", "e");
      tc.add_rule("aA", "e");
      tc.add_rule("Aa", "e");
      REQUIRE(tc.size() == 120);
    }

    TEST_CASE("Todd-Coxeter 29: Chapter 7, Theorem 3.6 in NR (size 243)",
              "[todd-coxeter][quick][29]") {
      REPORTER.set_report(REPORT);
      ToddCoxeter tc("ab");
      tc.add_rule("aaa", "a");
      tc.add_rule("bbbb", "b");
      tc.add_rule("ababababab", "aa");

      REQUIRE(tc.size() == 243);
    }

    TEST_CASE("Todd-Coxeter 29: finite semigroup (size 99)",
              "[todd-coxeter][quick][29]") {
      REPORTER.set_report(REPORT);
      ToddCoxeter tc("ab");
      tc.add_rule("aaa", "a");
      tc.add_rule("bbbb", "b");
      tc.add_rule("abababab", "aa");

      REQUIRE(tc.size() == 99);
    }

  }  // namespace fpsemigroup_todd_coxeter
}  // namespace libsemigroups*/