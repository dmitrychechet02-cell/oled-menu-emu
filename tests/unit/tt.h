/*
 * tt — минимальный тестовый раннер без зависимостей.
 * TT_TEST(name) регистрирует тест; запуск: build/unit_tests [фильтр-подстрока].
 */
#ifndef TT_H
#define TT_H

#include <stdio.h>

typedef void (*tt_fn)(void);

void tt_register(const char *name, tt_fn fn);
void tt_fail(const char *file, int line, const char *expr);

#define TT_TEST(name)                                                                              \
    static void name(void);                                                                        \
    __attribute__((constructor)) static void tt_reg_##name(void)                                   \
    {                                                                                              \
        tt_register(#name, name);                                                                  \
    }                                                                                              \
    static void name(void)

#define TT_ASSERT(expr)                                                                            \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            tt_fail(__FILE__, __LINE__, #expr);                                                    \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define TT_ASSERT_EQ(a, b)                                                                         \
    do {                                                                                           \
        long long tt_a_ = (long long)(a), tt_b_ = (long long)(b);                                  \
        if (tt_a_ != tt_b_) {                                                                      \
            char tt_msg_[160];                                                                     \
            snprintf(tt_msg_, sizeof tt_msg_, "%s == %s (%lld != %lld)", #a, #b, tt_a_, tt_b_);    \
            tt_fail(__FILE__, __LINE__, tt_msg_);                                                  \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define TT_ASSERT_STR(a, b)                                                                        \
    do {                                                                                           \
        const char *tt_a_ = (a), *tt_b_ = (b);                                                     \
        if (strcmp(tt_a_, tt_b_) != 0) {                                                           \
            char tt_msg_[200];                                                                     \
            snprintf(tt_msg_, sizeof tt_msg_, "%s == %s (\"%s\" != \"%s\")", #a, #b, tt_a_,        \
                     tt_b_);                                                                       \
            tt_fail(__FILE__, __LINE__, tt_msg_);                                                  \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#endif /* TT_H */
