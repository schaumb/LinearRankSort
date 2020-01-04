#pragma once

#include <utility>
#include <iterator>
#include <limits>
#include <cmath>
#include <algorithm>

namespace bxlx {
    /* sorting different integral elements. 'It' need to be random access iterator for the best performance
        n = number of elements
        r = the range of numbers
        Time:
            best = average = worst = O(n)
        Swaps:
            best = 0
            average = ?
            worst = O(n)
        Memory:
            best = average = worst = O(n*log_2(r)). 
            Memory usage will be only a number. 
        Stable:
            The conditions prohibits duplicated elements
        n << r:
            Yes/No. See Note.
            
        If 'It' not a random access iterator, these complexities changes:
        Time:
            average = ?
            worst = O(n^2)
            
        Note:
            There is a fix limit for the parameters (n, r):
                n * (ceil(log_2(r))+1) <= bits(uintmax_t) 
            
            if bits(uintmax_t) == 64 and r == n, n maximum can be only 12
            if bits(uintmax_t) == 64:
                    n == 2,  r max: 2^31
                    n == 3,  r max: 2^20
                    n == 4,  r max: 2^15
                    n == 5,  r max: 2^11
                    n == 6,  r max: 2^ 9
                    n == 7,  r max: 2^ 8
                    n == 8,  r max: 2^ 7
                    n == 9,  r max: 2^ 6
                    n == 10, r max: 2^ 5
                    n == 11, r max: 2^ 4
                    n == 12, r max: 2^ 4
            
            If you need more bits, you can use your own bignum type, which has operators: 
                unary: ~ 
                binary: +, -, *, %, >>, <<, ^, &, ^=, &=
                
        Note2: the "different integer" limitation can be eliminated with the "rank_" functions somehow
    */
    template<typename DataStorage = std::uintmax_t, bool calculateMinMax = true, typename It>
    void linearRankSort(It begin, It end);

    namespace detail {
    constexpr auto identity = [](auto&& data) -> decltype(data) { return data; };
        template<typename T>
        struct ConstTimeRankVec {
            constexpr static T numberOfBits(T x) {
                return x < 2 ? x : 1 + numberOfBits(x >> 1);
            }
            constexpr static T usableBits = numberOfBits(std::numeric_limits<T>::max());
            
            T b, mul, bMod;
            std::size_t k;
            T data;
            
            ConstTimeRankVec(std::size_t k, std::size_t other)
                : b (static_cast<T>(std::ceil(std::log2(k + other))))
                , mul{}
                , bMod (static_cast<T>(1 << (b+1)) - 1)
                , k {k}
                , data{} {
                if (k * (b+1) > usableBits)
                    throw std::overflow_error("overflow in const time ranking vector");
            
                for (std::size_t i = 0; i < k; ++i)
                    set(i, 1);
                std::swap(data, mul);
            }

            template<typename It, typename ToElem = decltype((identity))>
            ConstTimeRankVec(It begin, It end, std::size_t other, ToElem&& toE = identity)
                : ConstTimeRankVec(std::distance(begin, end), other) {
                assign(begin, end, std::forward<ToElem>(toE));
            }
            
            template<typename It, typename ToElem = decltype((identity))>
            void assign(It begin, It end, ToElem&& toE = identity) {
                for (auto&& [index, cur] = std::pair{T{}, begin}; cur != end; ++index, ++cur)
                    set(index, toE(*cur));
            }
            
            void set(std::size_t index, T elem) {
                data ^= elem << (b+1) * index;
            }
            
            T get(std::size_t index) const {
                return data >> (b+1) * index & bMod;
            }
            
            std::size_t size() const {
                return k;
            }
            
            void swap(std::size_t lhs, std::size_t rhs) {
                auto tmp = get(lhs) ^ get(rhs);
                set(lhs, tmp);
                set(rhs, tmp);
            }
            
            void erase(std::size_t index) {
                data &= ~(bMod << (b+1) * index);
            }
            
            std::size_t rank(std::size_t index) const {
                return ( ((get(index) + (1 << b)) * mul - data) >> b & mul) % bMod - 1;
            }

            std::size_t rank_bigger(std::size_t index) const {
                return ( ((get(index) + (1 << b)) * mul - data) >> b >> index * (b+1) & mul) % bMod - 1;
            }

            std::size_t rank_lower(std::size_t index) const {
                return ( ((get(index) + (1 << b)) * mul - data) >> b & mul >> index * (b+1)) % bMod - 1;
            }
        };
    }
    
    template<typename Type, bool calculateMinMax, typename It>
    void linearRankSort(It begin, It end) {
        if (begin == end) return;

        const std::size_t n = std::distance(begin, end);

        std::size_t r = n;
        typename std::iterator_traits<It>::value_type minE{};
        if constexpr (calculateMinMax) {
            auto&& [min, max] = std::minmax_element(begin, end);
            r = *max - (minE = *min) + 1;
        }
        detail::ConstTimeRankVec<Type> rankVec(begin, end, r-n, [&minE](auto&& elem) { return elem - minE; });
        
        It it = begin;
        for (std::size_t i{}; i < n;)
            if (auto rank = rankVec.rank(i); rank != i) {
                std::iter_swap(it, std::next(it, rank - i));
                rankVec.swap(i, rank);
            } else
                ++i, ++it;
    }
}
