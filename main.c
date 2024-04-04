#include <stdio.h>
#include <stdlib.h>

// -------- Macros --------

#define RESULT(T, E) struct { union { T value; E error; }; bool is_ok; }
#define OK(F, V) (typeof(F)) { .is_ok = true, .value = V }
#define ERR(F, E) (typeof(F)) { .is_ok = false, .error = E }

void panic(const char *msg, const char *file, int line) {
    fprintf(stderr, "%s:%d Error: %s\n", file, line, msg);
    exit(EXIT_FAILURE);
}

#define UNWRAP(R) ({ \
    auto _r = (R); \
    _r.is_ok ? _r.value : (panic(_r.error, __FILE__, __LINE__), _r.value); \
})

#define UNWRAP_ERR(R) ({ \
    auto _r = (R); \
    (!_r.is_ok) ? _r.error : (panic("unwrap_err got an ok value", __FILE__, __LINE__), _r.error); \
})

#define TRY(T, R) ({ \
    auto _r = (R); \
    if (!_r.is_ok) return ERR(T, _r.error); \
    _r.value; \
})

// -------- Usage --------

#define T checked_add(0, 0)
RESULT(int, char *) checked_add(int a, int b) {
    if (a > 0 && b > INT_MAX - a) return ERR(T, "Int overflow");
    if (a < 0 && b < INT_MIN - a) return ERR(T, "Int underflow");
    return OK(T, a + b);
}
#undef T

#define T checked_multiply(0, 0)
RESULT(int, char *) checked_multiply(int a, int b) {
    int c = a;
    for (int i = 1; i < b; i++) c = TRY(T, checked_add(c, a));
    return OK(T, c);
}
#undef T

int main(void) {
    int a = 34, b = 35;
    int c = UNWRAP(checked_add(a, b));
    fprintf(stderr, "%d + %d = %d\n", a, b, c);

    a = 42, b = 10;
    c = UNWRAP(checked_multiply(a, b));
    fprintf(stderr, "%d * %d = %d\n", a, b, c);

    a = 42, b = INT_MAX;
    auto err = UNWRAP_ERR(checked_add(a, b));
    fprintf(stderr, "Error: %s\n", err);

    a = 42, b = INT_MAX;
    err = UNWRAP_ERR(checked_multiply(a, b));
    fprintf(stderr, "Error: %s\n", err);

    return 0;
}
