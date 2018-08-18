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

// This file contains a method for enumerating a congruence by attempting to
// find all pairs of related elements using brute force. This does not work
// very well in most cases, due to the high complexity of the approach. Note
// that currently it is only used with FroidurePin<KBE>, and so doesn't
// strictly have to be a class template.

#ifndef LIBSEMIGROUPS_INCLUDE_CONG_PAIR_HPP_
#define LIBSEMIGROUPS_INCLUDE_CONG_PAIR_HPP_

#include <queue>

#include "internal/stl.hpp"
#include "internal/uf.hpp"

#include "cong-base.hpp"
#include "fpsemi-base.hpp"
#include "froidure-pin.hpp"
#include "kbe.hpp"
#include "wrap.hpp"

namespace libsemigroups {
  namespace congruence {
    // TODO: the type of the congruence defined by class P should be a template
    // parameter
    template <typename TElementType  = Element const*,
              typename TElementHash  = hash<TElementType>,
              typename TElementEqual = equal_to<TElementType>,
              class TTraits
              = TraitsHashEqual<TElementType, TElementHash, TElementEqual>>
    class P : public CongBase, protected TTraits {
     public:
      ////////////////////////////////////////////////////////////////////////
      // P - typedefs - public
      ////////////////////////////////////////////////////////////////////////
      // TODO check if the following are all actually used
      using element_type       = typename TTraits::element_type;
      using const_element_type = typename TTraits::const_element_type;
      using reference          = typename TTraits::reference;
      using const_reference    = typename TTraits::const_reference;

      ////////////////////////////////////////////////////////////////////////
      // P - typedefs - private
      ////////////////////////////////////////////////////////////////////////

      using internal_element_type = typename TTraits::internal_element_type;
      using internal_const_element_type =
          typename TTraits::internal_const_element_type;
      using internal_reference = typename TTraits::internal_reference;
      using internal_const_reference =
          typename TTraits::internal_const_reference;

      using internal_equal_to = typename TTraits::internal_equal_to;
      using internal_hash     = typename TTraits::internal_hash;

      using product = ::libsemigroups::product<internal_element_type>;

     protected:
      ////////////////////////////////////////////////////////////////////////
      // P - constructor - protected
      ////////////////////////////////////////////////////////////////////////
      // This is protected because it is not possible to create a P object
      // without it being defined over a parent semigroup.

      // TODO should be this and not TTraits that is used as a template
      // parameter, maybe?
      using semigroup_type
          = FroidurePin<TElementType, TElementHash, TElementEqual, TTraits>;

      explicit P(congruence_type type)
          : CongBase(type),
            _class_lookup(),
            _found_pairs(),
            _init_done(false),
            _lookup(0),
            _map(),
            _map_next(0),
            _next_class(0),
            _nr_non_trivial_classes(UNDEFINED),
            _nr_non_trivial_elemnts(UNDEFINED),
            _pairs_to_mult(),
            _reverse_map(),
            _tmp1(),
            _tmp2() {}

     public:
      ////////////////////////////////////////////////////////////////////////
      // P - constructor + destructor - public
      ////////////////////////////////////////////////////////////////////////

      P(congruence_type type, FroidurePinBase* S) : P(type) {
        LIBSEMIGROUPS_ASSERT(S != nullptr);
        set_nr_generators(S->nr_generators());
        set_parent(S);
      }

      P(congruence_type type, FroidurePinBase& S) : P(type, &S) {}

      ~P() {
        delete_tmp_storage();
        this->internal_free(_tmp1);
        this->internal_free(_tmp2);
        for (auto& x : _map) {
          this->internal_free(const_cast<internal_element_type>(x.first));
        }
      }

      ////////////////////////////////////////////////////////////////////////
      // Runner - overridden pure virtual methods - public
      ////////////////////////////////////////////////////////////////////////

      void run() override {
        if (finished() || dead()) {
          return;
        }
        Timer t;
        init();

        size_t tid = REPORTER.thread_id(std::this_thread::get_id());
        while (!_pairs_to_mult.empty() && !dead() && !timed_out()) {
          // Get the next pair
          auto& current_pair = _pairs_to_mult.front();
          _pairs_to_mult.pop();
          // TODO can the previous be auto&, or does the pop make this UB?

          auto prnt = static_cast<semigroup_type*>(get_parent());
          // Add its left and/or right multiples
          for (size_t i = 0; i < prnt->nr_generators(); i++) {
            const_reference gen = prnt->generator(i);
            if (type() == congruence_type::LEFT
                || type() == congruence_type::TWOSIDED) {
              product()(
                  _tmp1, this->to_internal_const(gen), current_pair.first, tid);
              product()(_tmp2,
                        this->to_internal_const(gen),
                        current_pair.second,
                        tid);
              internal_add_pair(_tmp1, _tmp2);
            }
            if (type() == congruence_type::RIGHT
                || type() == congruence_type::TWOSIDED) {
              product()(
                  _tmp1, current_pair.first, this->to_internal_const(gen), tid);
              product()(_tmp2,
                        current_pair.second,
                        this->to_internal_const(gen),
                        tid);
              internal_add_pair(_tmp1, _tmp2);
            }
          }
          if (report()) {
            REPORT("found ",
                   _found_pairs.size(),
                   " pairs: ",
                   _map_next,
                   " elements in ",
                   _lookup.nr_blocks(),
                   " classes, ",
                   _pairs_to_mult.size(),
                   " pairs on the stack");
          }
        }

        if (!dead() && !timed_out()) {
          // Make a normalised class lookup (class numbers {0, .., n-1}, in
          // order)
          if (_lookup.get_size() > 0) {
            _class_lookup.reserve(_lookup.get_size());
            _next_class = 1;
            size_t max  = 0;
            LIBSEMIGROUPS_ASSERT(_lookup.find(0) == 0);
            _class_lookup.push_back(0);
            for (size_t i = 1; i < _lookup.get_size(); i++) {
              size_t nr = _lookup.find(i);
              if (nr > max) {
                _class_lookup.push_back(_next_class++);
                max = nr;
              } else {
                _class_lookup.push_back(_class_lookup[nr]);
              }
            }
          }

          // Record information about non-trivial classes
          _nr_non_trivial_classes = _next_class;
          _nr_non_trivial_elemnts = _map_next;
        }

        REPORT("stopping with ",
               _found_pairs.size(),
               " pairs: ",
               _map_next,
               " elements in ",
               _lookup.nr_blocks(),
               " classes");
        REPORT("elapsed time = ", t);
        report_why_we_stopped();
        if (!dead() && !timed_out()) {
          set_finished(true);
          delete_tmp_storage();
        }
      }

      ////////////////////////////////////////////////////////////////////////
      // CongBase - overridden pure virtual methods - public
      ////////////////////////////////////////////////////////////////////////

      void add_pair(word_type const& l, word_type const& r) override {
        if (!has_parent()) {
          throw LIBSEMIGROUPS_EXCEPTION("cannot add generating pairs before "
                                        "the parent semigroup is defined");
        }
        auto x = static_cast<semigroup_type*>(get_parent())->word_to_element(l);
        auto y = static_cast<semigroup_type*>(get_parent())->word_to_element(r);
        internal_add_pair(this->to_internal(x), this->to_internal(y));
        this->external_free(x);
        this->external_free(y);
        set_finished(false);
      }

      word_type class_index_to_word(class_index_type) override {
        // TODO implement
        throw LIBSEMIGROUPS_EXCEPTION("not yet implemented");
      }

      FroidurePinBase* quotient_semigroup() override {
        // FIXME actually implement this
        throw LIBSEMIGROUPS_EXCEPTION("not yet implemented");
      }

      size_t nr_classes() override {
        run();
        return get_parent()->size() - _class_lookup.size() + _next_class;
      }

      class_index_type word_to_class_index(word_type const& w) final {
        run();
        LIBSEMIGROUPS_ASSERT(finished());
        return const_word_to_class_index(w);
      }

     protected:
      ////////////////////////////////////////////////////////////////////////
      // CongBase - overridden non-pure virtual methods - protected
      ////////////////////////////////////////////////////////////////////////

      class_index_type
      const_word_to_class_index(word_type const& w) const override {
        if (!finished()) {
          return UNDEFINED;
        }
        auto x = static_cast<semigroup_type*>(get_parent())->word_to_element(w);
        size_t ind_x = get_index(this->to_internal_const(x));
        this->external_free(x);
        LIBSEMIGROUPS_ASSERT(ind_x < _class_lookup.size());
        LIBSEMIGROUPS_ASSERT(_class_lookup.size() == _map.size());
        return _class_lookup[ind_x];
      }

      void init_non_trivial_classes() override {
        run();
        LIBSEMIGROUPS_ASSERT(_reverse_map.size() >= _nr_non_trivial_elemnts);
        LIBSEMIGROUPS_ASSERT(_class_lookup.size() >= _nr_non_trivial_elemnts);
        LIBSEMIGROUPS_ASSERT(has_parent());

        _non_trivial_classes.assign(_nr_non_trivial_classes,
                                    std::vector<word_type>());
        for (size_t ind = 0; ind < _nr_non_trivial_elemnts; ++ind) {
          word_type word
              = static_cast<semigroup_type*>(get_parent())
                    ->factorisation(this->to_external(_reverse_map[ind]));
          _non_trivial_classes[_class_lookup[ind]].push_back(word);
        }
      }

      ////////////////////////////////////////////////////////////////////////
      // P - methods - protected
      ////////////////////////////////////////////////////////////////////////

      void internal_add_pair(internal_const_element_type x,
                             internal_const_element_type y) {
        if (!internal_equal_to()(x, y)) {
          internal_element_type xx, yy;
          bool                  xx_new = false, yy_new = false;
          size_t                i, j;

          auto it_x = _map.find(x);
          if (it_x == _map.end()) {
            xx_new = true;
            xx     = this->internal_copy(x);
            i      = add_index(xx);
          } else {
            i = it_x->second;
          }

          auto it_y = _map.find(y);
          if (it_y == _map.end()) {
            yy_new = true;
            yy     = this->internal_copy(y);
            j      = add_index(yy);
          } else {
            j = it_y->second;
          }

          LIBSEMIGROUPS_ASSERT(i != j);
          std::pair<internal_element_type, internal_element_type> pair;
          if (xx_new || yy_new) {  // it's a new pair
            xx   = internal_element_type(xx_new ? xx : it_x->first);
            yy   = internal_element_type(yy_new ? yy : it_y->first);
            pair = (i < j ? std::make_pair(xx, yy) : std::make_pair(yy, xx));
          } else {
            pair = (i < j ? std::make_pair(internal_element_type(it_x->first),
                                           internal_element_type(it_y->first))
                          : std::make_pair(internal_element_type(it_y->first),
                                           internal_element_type(it_x->first)));
            if (_found_pairs.find(pair) != _found_pairs.end()) {
              return;
            }
          }
          _found_pairs.insert(pair);
          _pairs_to_mult.push(pair);
          _lookup.unite(i, j);
        }
      }

      ////////////////////////////////////////////////////////////////////////
      // P - methods - private
      ////////////////////////////////////////////////////////////////////////
     private:
      size_t add_index(internal_element_type x) const {
        LIBSEMIGROUPS_ASSERT(_reverse_map.size() == _map_next);
        LIBSEMIGROUPS_ASSERT(_map.size() == _map_next);
        _map.emplace(x, _map_next);
        _reverse_map.push_back(x);
        _lookup.add_entry();
        if (finished()) {
          _class_lookup.push_back(_next_class++);
        }
        return _map_next++;
      }

      void delete_tmp_storage() {
        std::unordered_set<
            std::pair<internal_element_type, internal_element_type>,
            PHash,
            PEqual>()
            .swap(_found_pairs);
        std::queue<std::pair<internal_element_type, internal_element_type>>()
            .swap(_pairs_to_mult);
      }

      size_t get_index(internal_const_element_type x) const {
        auto it = _map.find(x);
        if (it == _map.end()) {
          return add_index(this->internal_copy(x));
        }
        return it->second;
      }

      void init() {
        if (!_init_done) {
          LIBSEMIGROUPS_ASSERT(has_parent());
          LIBSEMIGROUPS_ASSERT(get_parent()->nr_generators() > 0);
          _tmp1      = this->internal_copy(this->to_internal_const(
              static_cast<semigroup_type*>(get_parent())->generator(0)));
          _tmp2      = this->internal_copy(_tmp1);
          _init_done = true;
        }
      }

      ////////////////////////////////////////////////////////////////////////
      // P - inner structs - private
      ////////////////////////////////////////////////////////////////////////

      struct PHash {
       public:
        size_t
        operator()(std::pair<internal_const_element_type,
                             internal_const_element_type> const& pair) const {
          return internal_hash()(pair.first)
                 + 17 * internal_hash()(pair.second);
        }
      };

      struct PEqual {
        size_t operator()(std::pair<internal_const_element_type,
                                    internal_const_element_type> pair1,
                          std::pair<internal_const_element_type,
                                    internal_const_element_type> pair2) const {
          return internal_equal_to()(pair1.first, pair2.first)
                 && internal_equal_to()(pair1.second, pair2.second);
        }
      };

      ////////////////////////////////////////////////////////////////////////
      // P - data - private
      ////////////////////////////////////////////////////////////////////////

      mutable std::vector<class_index_type> _class_lookup;
      std::unordered_set<
          std::pair<internal_element_type, internal_element_type>,
          PHash,
          PEqual>
                 _found_pairs;
      bool       _init_done;
      mutable UF _lookup;
      mutable std::unordered_map<internal_const_element_type,
                                 size_t,
                                 internal_hash,
                                 internal_equal_to>
                               _map;
      mutable size_t           _map_next;
      mutable class_index_type _next_class;
      size_t                   _nr_non_trivial_classes;
      size_t                   _nr_non_trivial_elemnts;
      std::queue<std::pair<internal_element_type, internal_element_type>>
                                                 _pairs_to_mult;
      mutable std::vector<internal_element_type> _reverse_map;
      internal_element_type                      _tmp1;
      internal_element_type                      _tmp2;
    };

    //////////////////////////////////////////////////////////////////////////
    // The next class is for calculating congruences on a finitely presented
    // semigroup using KnuthBendix on the fp semigroup and then the pairs
    // algorithm to compute the congruence.
    //////////////////////////////////////////////////////////////////////////
    // TODO move the implementation to a cong-p.cpp file
    class KBP : public P<KBE,
                         hash<KBE>,
                         equal_to<KBE>,
                         TraitsHashEqual<KBE, hash<KBE>, equal_to<KBE>>> {
      ////////////////////////////////////////////////////////////////////////
      // KBP - typedefs - private
      ////////////////////////////////////////////////////////////////////////

      using p_type = P<KBE,
                       hash<KBE>,
                       equal_to<KBE>,
                       TraitsHashEqual<KBE, hash<KBE>, equal_to<KBE>>>;

     public:
      ////////////////////////////////////////////////////////////////////////
      // KBP - constructors - public
      ////////////////////////////////////////////////////////////////////////

      KBP(congruence_type type, fpsemigroup::KnuthBendix* kb)
          : p_type(type), _kb(kb) {
        set_nr_generators(_kb->alphabet().size());
      }

      // For testing purposes only really
      KBP(congruence_type type, fpsemigroup::KnuthBendix& kb)
          : KBP(type, &kb){};

      ////////////////////////////////////////////////////////////////////////
      // P - overridden virtual methods - public
      ////////////////////////////////////////////////////////////////////////

      void run() override {
        if (stopped()) {
          return;
        }
        _kb->run_until([this]() -> bool { return dead() || timed_out(); });
        if (!stopped()) {
          set_parent(_kb->isomorphic_non_fp_semigroup());
          p_type::run();
        }
        report_why_we_stopped();
      }

      // Override the method for the class P to avoid having to know the parent
      // semigroup (found as part of KBP::run) to add a pair.
      // TODO this copies KBE(_kb, l) and KBE(_kb, r) twice.
      void add_pair(word_type const& l, word_type const& r) override {
        internal_element_type x = new element_type(_kb, l);
        internal_element_type y = new element_type(_kb, r);
        internal_add_pair(x, y);
        this->internal_free(x);
        this->internal_free(y);
        set_finished(false);
      }

     protected:
      ////////////////////////////////////////////////////////////////////////
      // CongBase - overridden non-pure virtual methods - private
      ////////////////////////////////////////////////////////////////////////
      using p_type::init_non_trivial_classes;

      ////////////////////////////////////////////////////////////////////////
      // KBP - data - private
      ////////////////////////////////////////////////////////////////////////
      // TODO use shared_ptr

      fpsemigroup::KnuthBendix* _kb;
    };

  }  // namespace congruence

  namespace fpsemigroup {
    template <typename TElementType  = Element const*,
              typename TElementHash  = hash<TElementType>,
              typename TElementEqual = equal_to<TElementType>,
              class TTraits
              = TraitsHashEqual<TElementType, TElementHash, TElementEqual>>
    using P = WrappedCong<
        congruence::P<TElementType, TElementHash, TElementEqual, TTraits>,
        false>;
    // The false in the template of the previous line, is so that we do not add
    // the rules from any underlying semigroup to the P.
  }  // namespace fpsemigroup
}  // namespace libsemigroups
#endif  // LIBSEMIGROUPS_INCLUDE_CONG_PAIR_HPP_