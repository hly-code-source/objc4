//===- llvm/ADT/DenseSet.h - Dense probed hash table ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DenseSet and SmallDenseSet classes.
//
//===----------------------------------------------------------------------===//

// Taken from clang-1100.247.11.10.9

#ifndef LLVM_ADT_DENSESET_H
#define LLVM_ADT_DENSESET_H

#include "llvm-DenseMap.h"
#include "llvm-DenseMapInfo.h"
#include "llvm-type_traits.h"
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <TargetConditionals.h>

#include "objc-private.h"

namespace objc {

namespace detail {

struct DenseSetEmpty {};
// 稠密集合

// Use the empty base class trick so we can create a DenseMap where the buckets
// contain only a single item.
// 模板类
// 定义了一个键值对，这个键值对只有key，value是空的，
// 因为set只有存储一个关键字的值，也就是key，所以不需要value
template <typename KeyT> class DenseSetPair : public DenseSetEmpty {
  KeyT key;

public:
  KeyT &getFirst() { return key; } // 获取第一个元素 key
  const KeyT &getFirst() const { return key; }
  DenseSetEmpty &getSecond() { return *this; } // 第二个元素就是当前的这个对象
  const DenseSetEmpty &getSecond() const { return *this; }
};

/// Base class for DenseSet and DenseSmallSet.
///
/// MapTy should be either
///
///   DenseMap<ValueT, detail::DenseSetEmpty,
///            DenseMapValueInfo<detail::DenseSetEmpty>,
///            ValueInfoT, detail::DenseSetPair<ValueT>>
///
/// or the equivalent SmallDenseMap type.  ValueInfoT must implement the
/// DenseMapInfo "concept".
template <typename ValueT, typename MapTy, typename ValueInfoT>
class DenseSetImpl {
  static_assert(sizeof(typename MapTy::value_type) == sizeof(ValueT),
                "DenseMap buckets unexpectedly large!");
  MapTy TheMap;

  template <typename T>
  using const_arg_type_t = typename const_pointer_or_const_ref<T>::type;

public:
  using key_type = ValueT; // key的类型
  using value_type = ValueT; // value的类型
  using size_type = unsigned; // 数量的类型，这里设置为了无符号

  explicit DenseSetImpl(unsigned InitialReserve = 0) : TheMap(InitialReserve) {}
//     这个是一个构造函数，默认传进入的值是0

//     初始化构造函数
//     初始化了一个穿进来的列表Elems
  DenseSetImpl(std::initializer_list<ValueT> Elems)
      : DenseSetImpl(PowerOf2Ceil(Elems.size())) {
    insert(Elems.begin(), Elems.end());
//   插入数据： 通过迭代的begin和end
  }

  bool empty() const { return TheMap.empty(); } // 判空
  size_type size() const { return TheMap.size(); } //大小
  size_t getMemorySize() const { return TheMap.getMemorySize(); } //获取内存大小？？？ 这个不知道是干嘛用的？

  /// Grow the DenseSet so that it has at least Size buckets. Will not shrink
  /// the Size of the set.
  void resize(size_t Size) { TheMap.resize(Size); } // 重新设置哈希表的大小

  /// Grow the DenseSet so that it can contain at least \p NumEntries items
  /// before resizing again.
  void reserve(size_t Size) { TheMap.reserve(Size); } //保留的数量
//  上面两个函数是再次变化数据大小的时候的函数； 一个是重新处理的大小，一个是保存的上一次的大小。

  void clear() {
    TheMap.clear(); // 清空
  }

  /// Return 1 if the specified key is in the set, 0 otherwise.
    // 有就返回1，没有就返回0
    // 因为这里是不可重复的
  size_type count(const_arg_type_t<ValueT> V) const {
    return TheMap.count(V);
  }

// 擦除某个值，这里的v应该传入的是值
  bool erase(const ValueT &V) {
    return TheMap.erase(V);
  }

//     交换， 应该是交换两个结婚
  void swap(DenseSetImpl &RHS) { TheMap.swap(RHS.TheMap); }

  // Iterators.
// 实现了迭代器
  class ConstIterator;

  class Iterator {
    typename MapTy::iterator I;
    friend class DenseSetImpl;
    friend class ConstIterator;

  public:
    using difference_type = typename MapTy::iterator::difference_type;
    using value_type = ValueT;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::forward_iterator_tag;

    Iterator() = default;
    Iterator(const typename MapTy::iterator &i) : I(i) {}

    ValueT &operator*() { return I->getFirst(); }
    const ValueT &operator*() const { return I->getFirst(); }
    ValueT *operator->() { return &I->getFirst(); }
    const ValueT *operator->() const { return &I->getFirst(); }

    Iterator& operator++() { ++I; return *this; }
    Iterator operator++(int) { auto T = *this; ++I; return T; }
    bool operator==(const ConstIterator& X) const { return I == X.I; }
    bool operator!=(const ConstIterator& X) const { return I != X.I; }
  };

//     思考一个不同的语言对常量的定义；
//     常量迭代器
  class ConstIterator {
    typename MapTy::const_iterator I; // MapTy中的常来那个迭代器
    friend class DenseSet;
    friend class Iterator;

  public:
    using difference_type = typename MapTy::const_iterator::difference_type;
    using value_type = ValueT;
    using pointer = const value_type *;
    using reference = const value_type &;
    using iterator_category = std::forward_iterator_tag; // 这句话是什么意思、？应该是向前的，类似+1的变量

    ConstIterator() = default;
    ConstIterator(const Iterator &B) : I(B.I) {}
    ConstIterator(const typename MapTy::const_iterator &i) : I(i) {} // 主要的一个构造函数

//       指针和引用的处理
    const ValueT &operator*() const { return I->getFirst(); }
    const ValueT *operator->() const { return &I->getFirst(); }

//    引用++ ++ == != 的定义
    ConstIterator& operator++() { ++I; return *this; }
    ConstIterator operator++(int) { auto T = *this; ++I; return T; }
    bool operator==(const ConstIterator& X) const { return I == X.I; }
    bool operator!=(const ConstIterator& X) const { return I != X.I; }
  };

  using iterator = Iterator;
  using const_iterator = ConstIterator;

//     迭代器的begin和end的处理
  iterator begin() { return Iterator(TheMap.begin()); }
  iterator end() { return Iterator(TheMap.end()); }
  const_iterator begin() const { return ConstIterator(TheMap.begin()); }
  const_iterator end() const { return ConstIterator(TheMap.end()); }

//     查找，一个是一般迭代器，一个是常量迭代器
  iterator find(const_arg_type_t<ValueT> V) { return Iterator(TheMap.find(V)); }
  const_iterator find(const_arg_type_t<ValueT> V) const {
    return ConstIterator(TheMap.find(V));
  }

  /// Alternative version of find() which allows a different, and possibly less
  /// expensive, key type.
  /// The DenseMapInfo is responsible for supplying methods
  /// getHashValue(LookupKeyT) and isEqual(LookupKeyT, KeyT) for each key type
  /// used.
//   查找
  template <class LookupKeyT>
  iterator find_as(const LookupKeyT &Val) {
    return Iterator(TheMap.find_as(Val));
  }
  template <class LookupKeyT>
  const_iterator find_as(const LookupKeyT &Val) const {
    return ConstIterator(TheMap.find_as(Val));
  }
// 删除
  void erase(Iterator I) { return TheMap.erase(I.I); }
  void erase(ConstIterator CI) { return TheMap.erase(CI.I); }

//   插入
  std::pair<iterator, bool> insert(const ValueT &V) {
    detail::DenseSetEmpty Empty;
    return TheMap.try_emplace(V, Empty);
  }

  std::pair<iterator, bool> insert(ValueT &&V) {
    detail::DenseSetEmpty Empty;
    return TheMap.try_emplace(std::move(V), Empty);
  }

  /// Alternative version of insert that uses a different (and possibly less
  /// expensive) key type.
  template <typename LookupKeyT>
  std::pair<iterator, bool> insert_as(const ValueT &V,
                                      const LookupKeyT &LookupKey) {
    return TheMap.insert_as({V, detail::DenseSetEmpty()}, LookupKey);
  }
  template <typename LookupKeyT>
  std::pair<iterator, bool> insert_as(ValueT &&V, const LookupKeyT &LookupKey) {
    return TheMap.insert_as({std::move(V), detail::DenseSetEmpty()}, LookupKey);
  }

  // Range insertion of values.
  template<typename InputIt>
  void insert(InputIt I, InputIt E) {
    for (; I != E; ++I)
      insert(*I);
  }
};

/// Equality comparison for DenseSet.
///
/// Iterates over elements of LHS confirming that each element is also a member
/// of RHS, and that RHS contains no additional values.
/// Equivalent to N calls to RHS.count. Amortized complexity is linear, worst
/// case is O(N^2) (if every hash collides).
/// == 的判断重写
template <typename ValueT, typename MapTy, typename ValueInfoT>
bool operator==(const DenseSetImpl<ValueT, MapTy, ValueInfoT> &LHS,
                const DenseSetImpl<ValueT, MapTy, ValueInfoT> &RHS) {
  if (LHS.size() != RHS.size())
    return false;

  for (auto &E : LHS)
    if (!RHS.count(E))
      return false;

  return true;
}

/// Inequality comparison for DenseSet.
///
/// Equivalent to !(LHS == RHS). See operator== for performance notes.
///  ！= 的判断重写
template <typename ValueT, typename MapTy, typename ValueInfoT>
bool operator!=(const DenseSetImpl<ValueT, MapTy, ValueInfoT> &LHS,
                const DenseSetImpl<ValueT, MapTy, ValueInfoT> &RHS) {
  return !(LHS == RHS);
}

} // end namespace detail

// 大的
/// Implements a dense probed hash-table based set.
///  这里传的是DenseMap 这样的类型，看来set是基于map来实现的
template <typename ValueT, typename ValueInfoT = DenseMapInfo<ValueT>>
class DenseSet : public detail::DenseSetImpl<
                     ValueT, DenseMap<ValueT, detail::DenseSetEmpty,
                                      DenseMapValueInfo<detail::DenseSetEmpty>,
                                      ValueInfoT, detail::DenseSetPair<ValueT>>,
                     ValueInfoT> {
  using BaseT =
      detail::DenseSetImpl<ValueT,
                           DenseMap<ValueT, detail::DenseSetEmpty,
                                    DenseMapValueInfo<detail::DenseSetEmpty>,
                                    ValueInfoT, detail::DenseSetPair<ValueT>>,
                           ValueInfoT>;

public:
  using BaseT::BaseT;
};

// 小的
/// Implements a dense probed hash-table based set with some number of buckets
/// stored inline.
template <typename ValueT, unsigned InlineBuckets = 4,
          typename ValueInfoT = DenseMapInfo<ValueT>>
class SmallDenseSet
    : public detail::DenseSetImpl<
          ValueT, SmallDenseMap<ValueT, detail::DenseSetEmpty, InlineBuckets,
                                DenseMapValueInfo<detail::DenseSetEmpty>,
                                ValueInfoT, detail::DenseSetPair<ValueT>>,
          ValueInfoT> {
  using BaseT = detail::DenseSetImpl<
      ValueT, SmallDenseMap<ValueT, detail::DenseSetEmpty, InlineBuckets,
                            DenseMapValueInfo<detail::DenseSetEmpty>,
                            ValueInfoT, detail::DenseSetPair<ValueT>>,
      ValueInfoT>;

public:
  using BaseT::BaseT;
};

} // end namespace objc

// 这里里面的两个set都是集成了DenseSetImpl 实现， 真实的数据结构都是通过SmallDenseMap/DenseMap来实现的额
// small多了一个初始化的InlineBuckets
#endif // LLVM_ADT_DENSESET_H
