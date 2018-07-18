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

// This file contains an interface for congruence-like classes.

#ifndef LIBSEMIGROUPS_SRC_CONG_INTF_H_
#define LIBSEMIGROUPS_SRC_CONG_INTF_H_

#include "defs.h"
#include "runner.h"
#include "semigroups-base.h"

namespace libsemigroups {
  namespace congruence {
    class Interface : public Runner {
     public:
      //! The different types of congruence.
      enum congruence_t { LEFT = 0, RIGHT = 1, TWOSIDED = 2 };

      //! Type for indices of congruence classes in a Interface
      //! object.
      typedef size_t class_index_t;
      typedef std::vector<std::vector<word_t>>::const_iterator
          non_trivial_class_iterator;

      ////////////////////////////////////////////////////////////////////////////
      // Constructors + Destructors
      ////////////////////////////////////////////////////////////////////////////

      explicit Interface(congruence_t type) : Runner(), _type(type) {}

      //! A default destructor.
      virtual ~Interface() {}

      ////////////////////////////////////////////////////////////////////////////
      // Pure methods
      ////////////////////////////////////////////////////////////////////////////

      //! Returns the index of the congruence class corresponding to \p word.
      //!
      //! The parameter \p word must be a libsemigroups::word_t consisting of
      //! indices of the generators of the semigroup over which \c this is
      //! defined.
      //!
      //! If \c this is defined over a semigroup with generators \f$A\f$, then
      //! Interface::word_to_class_index defines a surjective function
      //! from the set of all words over \f$A\f$ to either
      //! \f$\{0, 1, \ldots, n -  1\}\f$,
      //! where \f$n\f$ is the number of classes, or to the non-negative
      //! integers \f$\{0, 1, \ldots\}\f$ if \c this has infinitely many
      //! classes.
      //!
      //! \warning The method for finding the structure of a congruence may be
      //! non-deterministic, and the return value of this method may vary
      //! between different instances of the same congruence.
      virtual class_index_t word_to_class_index(word_t const& word) = 0;

      // TODO virtual word_t class_index_to_word(class_index_t i) = 0;

      //! Returns the number of congruences classes of \c this.
      //!
      //! This method is non-const because it may fully compute a data structure
      //! for the congruence.
      //!
      //! \warning The problem of determining the number of classes of a
      //! congruence over a finitely presented semigroup is undecidable in
      //! general, and this method may never terminate.
      virtual size_t nr_classes() = 0;

      //! Add a generating pair to the congruence. Should only be done before
      //! anything is computed about the congruence.
      // TODO make the preceeding description more precise, and actually enforce
      // it.
      virtual void add_pair(word_t, word_t) = 0;

      virtual SemigroupBase* quotient_semigroup() = 0;

      //! Returns the non-trivial classes of the congruence.
      //!
      //! The elements in these classes are represented as words in the
      //! generators of the semigroup over which the congruence is defined.
      //!
      //! \warning If \c this has infinitely many non-trivial congruence
      //! classes, then this method will only terminate when it can no longer
      //! allocate memory.
      virtual non_trivial_class_iterator cbegin_non_trivial_classes() = 0;
      virtual non_trivial_class_iterator cend_non_trivial_classes()   = 0;
      virtual size_t                     nr_non_trivial_classes()     = 0;

      //////////////////////////////////////////////////////////////////////////
      // Non-pure virtual methods (default versions using pure methods, where //
      // there might be better ways than the default in some cases).          //
      //////////////////////////////////////////////////////////////////////////

      //! Returns \c true if the words \p w1 and \p w2 belong to the
      //! same congruence class.
      //!
      //! The parameters \p w1  and \p w2 must be libsemigroups::word_t's
      //! consisting of indices of generators of the semigroup over which \c
      //! this is defined.
      //!
      //! \warning The problem of determining the return value of this method is
      //! undecidable in general, and this method may never terminate.
      virtual bool contains(word_t const& w1, word_t const& w2);

      // Same as the above but only uses the so far computed information to
      // answer. In particular, does not call this->run(). This method may
      // return false negatives, but must not return false positives.
      virtual bool const_contains(word_t const& w1, word_t const& w2) const;

      //! Returns \c true if the congruence class of \p w1 is less than
      //! that of \p w2.
      //!
      //! This method returns \c true if the congruence class of \p w1 is less
      //! than the class of \p w2 in a total ordering of congruence classes.
      //!
      //! The parameters \p w1 and \p w2 should be libsemigroups::word_t's
      //! consisting of indices of the generators of the semigroup over which \c
      //! this is defined.
      //!
      //! \warning The method for finding the structure of a congruence is
      //! non-deterministic, and the total order of congruences classes may vary
      //! between different instances of the same congruence.
      //!
      //! \warning The problem of determining the return value of this method is
      //! undecidable in general, and this method may never terminate.
      // Rename less -> ??? Or remove? FIXME
      virtual bool less(word_t const& w1, word_t const& w2);

      virtual bool is_quotient_obviously_infinite() const;

      /////////////////////////////////////////////////////////////////////////
      // Non-pure non-virtual methods (using the pure methods, for syntactic //
      // sugar mostly).                                                      //
      /////////////////////////////////////////////////////////////////////////

      // Pass by value since these must be copied anyway
      void add_pair(std::initializer_list<size_t>,
                    std::initializer_list<size_t>);

      //! Return the type of the congruence, i.e. if it is a left, right, or
      //! two-sided congruence.
      congruence_t type() const;

     private:
      /////////////////////////////////////////////////////////////////////////
      // Private pure virtual methods
      /////////////////////////////////////////////////////////////////////////

      // const_word_to_class_index is private, because the answer returned
      // depends on the state of the object, but word_to_class_index does not
      // (i.e the return value should not change).
      virtual class_index_t
      const_word_to_class_index(word_t const& word) const = 0;

      /////////////////////////////////////////////////////////////////////////
      // Private data
      /////////////////////////////////////////////////////////////////////////
      congruence_t _type;
    };
  }  // namespace congruence
}  // namespace libsemigroups
#endif  // LIBSEMIGROUPS_SRC_CONG_INTF_H_