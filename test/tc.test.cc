//
// Semigroups++ - C/C++ library for computing with semigroups and monoids
// Copyright (C) 2016 Michael Torpey
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

// The purpose of this file is to test the Congruence::TC class, this is
// achieved by calling cong->tc() before calculating anything about the
// congruence.

#include <utility>

#include "../cong.h"
#include "catch.hpp"

#define TC_REPORT false

using namespace libsemigroups;

template <typename T> static inline void really_delete_cont(T cont) {
  for (Element* x : cont) {
    x->really_delete();
    delete x;
  }
}

static inline void really_delete_partition(Congruence::partition_t part) {
  for (auto& cont : part) {
    for (Element const* x : cont) {
      const_cast<Element*>(x)->really_delete();
      delete x;
    }
  }
}

TEST_CASE(
    "TC 01: Small fp semigroup",
    "[quick][congruence][quick][congruence][tc][noprefill][fpsemigroup]") {
  std::vector<relation_t> rels;
  rels.push_back(relation_t({0, 0, 0}, {0}));  // (a^3, a)
  rels.push_back(relation_t({0}, {1, 1}));     // (a, b^2)
  std::vector<relation_t> extra;

  Congruence* cong = new Congruence("twosided", 2, rels, extra);
  cong->force_tc();
  cong->set_report(TC_REPORT);

  REQUIRE(!cong->is_done());

  REQUIRE(cong->nr_classes() == 5);
  REQUIRE(cong->is_done());

  REQUIRE(cong->word_to_class_index({0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 0, 0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 1, 1, 0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 0, 0}) == 1);
  REQUIRE(cong->word_to_class_index({1}) == 3);

  delete cong;
}

TEST_CASE("TC 02: Small left congruence on free semigroup",
          "[quick][congruence][tc][noprefill][fpsemigroup]") {
  std::vector<relation_t> rels;
  rels.push_back(relation_t({0, 0, 0}, {0}));  // (a^3, a)
  rels.push_back(relation_t({0}, {1, 1}));     // (a, b^2)
  std::vector<relation_t> extra;

  Congruence* cong = new Congruence("left", 2, rels, extra);
  cong->force_tc();
  cong->set_report(TC_REPORT);
  REQUIRE(cong->nr_classes() == 5);
  delete cong;
}

TEST_CASE("TC 03: Small right congruence on free semigroup",
          "[quick][congruence][tc][noprefill][fpsemigroup]") {
  std::vector<relation_t> rels;
  rels.push_back(relation_t({0, 0, 0}, {0}));  // (a^3, a)
  rels.push_back(relation_t({0}, {1, 1}));     // (a, b^2)
  std::vector<relation_t> extra;

  Congruence* cong = new Congruence("right", 2, rels, extra);
  cong->force_tc();
  cong->set_report(TC_REPORT);
  REQUIRE(cong->nr_classes() == 5);
  REQUIRE(cong->is_done());
  delete cong;
}

TEST_CASE("TC 04: word_to_class_index for left congruence on free "
          "semigroup",
          "[quick][congruence][tc][noprefill][fpsemigroup]") {
  std::vector<relation_t> rels;
  rels.push_back(relation_t({0, 0, 0}, {0}));  // (a^3, a)
  rels.push_back(relation_t({0}, {1, 1}));     // (a, b^2)
  std::vector<relation_t> extra;

  Congruence* cong = new Congruence("left", 2, rels, extra);
  cong->force_tc();
  cong->set_report(TC_REPORT);
  REQUIRE(cong->word_to_class_index({0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 0, 0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 1, 1, 0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 0, 0}) == 1);
  REQUIRE(cong->word_to_class_index({1}) == 3);
  REQUIRE(cong->word_to_class_index({0, 0, 0, 0}) == 2);
  delete cong;
}

TEST_CASE("TC 05: word_to_class_index for small fp semigroup",
          "[quick][congruence][tc][noprefill][fpsemigroup]") {
  std::vector<relation_t> rels;
  rels.push_back(relation_t({0, 0, 0}, {0}));  // (a^3, a)
  rels.push_back(relation_t({0}, {1, 1}));     // (a, b^2)
  std::vector<relation_t> extra;

  Congruence* cong = new Congruence("twosided", 2, rels, extra);
  cong->force_tc();
  cong->set_report(TC_REPORT);
  REQUIRE(cong->word_to_class_index({0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 0, 0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 1, 1, 0, 0, 1}) == 5);
  REQUIRE(cong->word_to_class_index({0, 0, 0}) == 1);
  REQUIRE(cong->word_to_class_index({1}) == 3);
  delete cong;

  cong = new Congruence("twosided", 2, rels, extra);
  cong->force_tc();
  cong->set_report(TC_REPORT);
  REQUIRE(cong->word_to_class_index({0, 0, 0, 0}) == 2);
  delete cong;
}

TEST_CASE("TC 06: transformation semigroup size 88",
          "[quick][congruence][tc][noprefill][finite]") {
  std::vector<Element*> vec = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
                               new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
  Semigroup S = Semigroup(vec);
  S.set_report(TC_REPORT);

  REQUIRE(S.size() == 88);
  REQUIRE(S.nrrules() == 18);
  REQUIRE(S.degree() == 5);

  vec.push_back(new Transformation<u_int16_t>({3, 4, 4, 4, 4}));
  word_t w1;
  S.factorisation(w1, S.position(vec.back()));

  vec.push_back(new Transformation<u_int16_t>({3, 1, 3, 3, 3}));
  word_t w2;
  S.factorisation(w2, S.position(vec.back()));

  std::vector<relation_t> extra({relation_t(w1, w2)});
  Congruence              cong("twosided", &S, extra);
  cong.force_tc();
  cong.set_report(TC_REPORT);

  REQUIRE(cong.nr_classes() == 21);
  REQUIRE(cong.nr_classes() == 21);

  vec.push_back(new Transformation<u_int16_t>({1, 3, 1, 3, 3}));
  S.factorisation(w1, S.position(vec.back()));

  vec.push_back(new Transformation<u_int16_t>({4, 2, 4, 4, 2}));
  S.factorisation(w2, S.position(vec.back()));

  REQUIRE(cong.word_to_class_index(w1) == cong.word_to_class_index(w2));

  Congruence::partition_t nontrivial_classes = cong.nontrivial_classes();
  REQUIRE(nontrivial_classes.size() == 1);
  REQUIRE(nontrivial_classes[0].size() == 68);
  really_delete_partition(nontrivial_classes);

  really_delete_cont(vec);
}

TEST_CASE("TC 07: left congruence on transformation semigroup size 88",
          "[quick][congruence][tc][noprefill][finite]") {
  std::vector<Element*> vec = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
                               new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
  Semigroup S = Semigroup(vec);
  S.set_report(TC_REPORT);

  REQUIRE(S.size() == 88);
  REQUIRE(S.degree() == 5);

  vec.push_back(new Transformation<u_int16_t>({3, 4, 4, 4, 4}));
  word_t w1;
  S.factorisation(w1, S.position(vec.back()));
  vec.push_back(new Transformation<u_int16_t>({3, 1, 3, 3, 3}));
  word_t w2;
  S.factorisation(w2, S.position(vec.at(3)));
  std::vector<relation_t> extra({relation_t(w1, w2)});
  Congruence              cong("left", &S, extra);
  cong.force_tc();
  cong.set_report(TC_REPORT);

  REQUIRE(cong.nr_classes() == 69);
  REQUIRE(cong.nr_classes() == 69);

  Congruence::partition_t nontrivial_classes = cong.nontrivial_classes();
  REQUIRE(nontrivial_classes.size() == 1);
  REQUIRE(nontrivial_classes[0].size() == 20);
  really_delete_partition(nontrivial_classes);

  really_delete_cont(vec);
}

TEST_CASE("TC 08: right congruence on transformation semigroup size 88",
          "[quick][congruence][tc][noprefill][finite]") {
  std::vector<Element*> vec = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
                               new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
  Semigroup S = Semigroup(vec);
  S.set_report(TC_REPORT);

  REQUIRE(S.size() == 88);
  REQUIRE(S.nrrules() == 18);
  REQUIRE(S.degree() == 5);

  word_t w1, w2;
  vec.push_back(new Transformation<u_int16_t>({3, 4, 4, 4, 4}));
  S.factorisation(w1, S.position(vec.back()));
  vec.push_back(new Transformation<u_int16_t>({3, 1, 3, 3, 3}));
  S.factorisation(w2, S.position(vec.back()));

  Congruence cong("right", &S, {relation_t(w1, w2)});
  cong.force_tc();
  cong.set_report(TC_REPORT);

  REQUIRE(cong.nr_classes() == 72);
  REQUIRE(cong.nr_classes() == 72);

  word_t w3, w4, w5, w6;
  vec.push_back(new Transformation<u_int16_t>({1, 3, 3, 3, 3}));
  S.factorisation(w3, S.position(vec.back()));
  vec.push_back(new Transformation<u_int16_t>({4, 2, 4, 4, 2}));
  S.factorisation(w4, S.position(vec.back()));
  vec.push_back(new Transformation<u_int16_t>({2, 4, 2, 2, 2}));
  S.factorisation(w5, S.position(vec.back()));
  vec.push_back(new Transformation<u_int16_t>({2, 3, 3, 3, 3}));
  S.factorisation(w6, S.position(vec.back()));

  REQUIRE(cong.word_to_class_index(w3) != cong.word_to_class_index(w4));
  REQUIRE(cong.word_to_class_index(w5) == cong.word_to_class_index(w6));
  REQUIRE(cong.word_to_class_index(w3) != cong.word_to_class_index(w6));

  Congruence::partition_t nontrivial_classes = cong.nontrivial_classes();
  REQUIRE(nontrivial_classes.size() == 4);
  REQUIRE(nontrivial_classes[0].size() == 3);
  REQUIRE(nontrivial_classes[1].size() == 5);
  REQUIRE(nontrivial_classes[2].size() == 5);
  REQUIRE(nontrivial_classes[3].size() == 7);
  really_delete_partition(nontrivial_classes);

  really_delete_cont(vec);
}

// Testing prefilled TC

TEST_CASE("TC 09: transformation semigroup size 88",
          "[quick][congruence][tc][prefill][finite]") {
  std::vector<Element*> gens = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
                                new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
  Semigroup S = Semigroup(gens);
  S.set_report(TC_REPORT);
  really_delete_cont(gens);

  REQUIRE(S.size() == 88);
  REQUIRE(S.nrrules() == 18);
  REQUIRE(S.degree() == 5);

  Element* t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
  Element* t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
  word_t   w1, w2;
  S.factorisation(w1, S.position(t1));
  S.factorisation(w2, S.position(t2));
  std::vector<relation_t> extra({relation_t(w1, w2)});
  Congruence*             cong = new Congruence("twosided", &S, extra);
  cong->force_tc_prefill();
  cong->set_report(TC_REPORT);

  REQUIRE(cong->nr_classes() == 21);
  REQUIRE(cong->nr_classes() == 21);
  Element* t3 = new Transformation<u_int16_t>({1, 3, 1, 3, 3});
  Element* t4 = new Transformation<u_int16_t>({4, 2, 4, 4, 2});
  word_t   w3, w4;
  S.factorisation(w3, S.position(t3));
  S.factorisation(w4, S.position(t4));
  REQUIRE(cong->word_to_class_index(w3) == cong->word_to_class_index(w4));

  t1->really_delete();
  t2->really_delete();
  t3->really_delete();
  t4->really_delete();
  delete t1;
  delete t2;
  delete t3;
  delete t4;
  delete cong;
}

TEST_CASE("TC 10: left congruence on transformation semigroup size 88",
          "[quick][congruence][tc][prefill][finite]") {
  std::vector<Element*> gens = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
                                new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
  Semigroup S = Semigroup(gens);
  S.set_report(TC_REPORT);
  really_delete_cont(gens);

  REQUIRE(S.size() == 88);
  REQUIRE(S.degree() == 5);
  Element* t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
  Element* t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
  word_t   w1, w2;
  S.factorisation(w1, S.position(t1));
  S.factorisation(w2, S.position(t2));
  std::vector<relation_t> extra({relation_t(w1, w2)});
  Congruence*             cong = new Congruence("left", &S, extra);
  cong->force_tc_prefill();
  cong->set_report(TC_REPORT);

  REQUIRE(cong->nr_classes() == 69);
  REQUIRE(cong->nr_classes() == 69);
  Element* t3 = new Transformation<u_int16_t>({1, 3, 1, 3, 3});
  Element* t4 = new Transformation<u_int16_t>({4, 2, 4, 4, 2});
  word_t   w3, w4;
  S.factorisation(w3, S.position(t3));
  S.factorisation(w4, S.position(t4));

  t1->really_delete();
  t2->really_delete();
  t3->really_delete();
  t4->really_delete();
  delete t1;
  delete t2;
  delete t3;
  delete t4;
  delete cong;
}

TEST_CASE("TC 11: right congruence on transformation semigroup size 88",
          "[quick][congruence][tc][prefill][finite]") {
  std::vector<Element*> gens = {new Transformation<u_int16_t>({1, 3, 4, 2, 3}),
                                new Transformation<u_int16_t>({3, 2, 1, 3, 3})};
  Semigroup S = Semigroup(gens);
  S.set_report(TC_REPORT);
  really_delete_cont(gens);

  REQUIRE(S.size() == 88);
  REQUIRE(S.nrrules() == 18);
  REQUIRE(S.degree() == 5);
  Element* t1 = new Transformation<u_int16_t>({3, 4, 4, 4, 4});
  Element* t2 = new Transformation<u_int16_t>({3, 1, 3, 3, 3});
  word_t   w1, w2;
  S.factorisation(w1, S.position(t1));
  S.factorisation(w2, S.position(t2));
  std::vector<relation_t> extra({relation_t(w1, w2)});
  Congruence*             cong = new Congruence("right", &S, extra);
  cong->force_tc_prefill();
  cong->set_report(TC_REPORT);

  REQUIRE(cong->nr_classes() == 72);
  REQUIRE(cong->nr_classes() == 72);
  Element* t3 = new Transformation<u_int16_t>({1, 3, 3, 3, 3});
  Element* t4 = new Transformation<u_int16_t>({4, 2, 4, 4, 2});
  Element* t5 = new Transformation<u_int16_t>({2, 4, 2, 2, 2});
  Element* t6 = new Transformation<u_int16_t>({2, 3, 3, 3, 3});
  word_t   w3, w4, w5, w6;
  S.factorisation(w3, S.position(t3));
  S.factorisation(w4, S.position(t4));
  S.factorisation(w5, S.position(t5));
  S.factorisation(w6, S.position(t6));
  REQUIRE(cong->word_to_class_index(w3) != cong->word_to_class_index(w4));
  //  REQUIRE(cong->word_to_class_index(w5) == cong->word_to_class_index(w6));
  REQUIRE(cong->word_to_class_index(w3) != cong->word_to_class_index(w6));

  t1->really_delete();
  t2->really_delete();
  t3->really_delete();
  t4->really_delete();
  t5->really_delete();
  t6->really_delete();
  delete t1;
  delete t2;
  delete t3;
  delete t4;
  delete t5;
  delete t6;
  delete cong;
}
