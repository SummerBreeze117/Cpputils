//
// Created by co2ma on 2023/6/14.
//

#ifndef LIKELY_H
#define LIKELY_H


#if defined(__GNUC__)
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif


#endif //LIKELY_H
