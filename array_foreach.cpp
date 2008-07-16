/*
 *  Copyright 2008 James Peach
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>

/* This is a very basic implementation of a BOOST_FOREACH-like macro. It only
 * works for array literals, and it's not clever enough to automatically
 * derive the type of the array elements. It's not const correct.
 *
 * On the plus side, it is a smaller and simpler than BOOST_FOREACH and can
 * be used standalone.
 */

template <typename T>
struct array_traits
{
    typedef T * iterator;
};

template <typename T, int N>
inline typename array_traits<T>::iterator
array_begin( T (&array_literal)[N])
{
    return array_literal;
}

template <typename T, int N>
inline typename array_traits<T>::iterator
array_end( T (&array_literal)[N])
{
    return array_literal + N;
}

const int numbers[] = { 1, 2, 3, 4, 5};

#define array_foreach(TYPE, VAR, ARRAY_LITERAL) \
    if (array_traits<TYPE>::iterator i = array_begin(ARRAY_LITERAL)) \
	for (bool __continue = true; \
		__continue && i != array_end(numbers); ++i) \
	    if (__continue = false) {} \
	    else for (TYPE& VAR = *i; !__continue; __continue = true)

void manual_enumeration(void)
{
    const int * end = numbers + (sizeof(numbers) / sizeof(numbers[0]));

    for (const int * n = numbers; n != end; ++n) {
	printf("%d ", *n);
    }
}

void basic_enumeration(void)
{
    for (const int * n = array_begin(numbers); n != array_end(numbers); ++n) {
	printf("%d ", *n);
    }
}

void foreach_enumeration(void)
{
    array_foreach(const int, n, numbers) {
	printf("%d ", n);
    }
}

int main(void)
{
    printf("manual: ");
    manual_enumeration();
    printf("\n");

    printf("basic: ");
    basic_enumeration();
    printf("\n");

    printf("foreach: ");
    foreach_enumeration();
    printf("\n");
}
