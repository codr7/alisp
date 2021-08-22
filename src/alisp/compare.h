#ifndef ALISP_COMPARE_H
#define ALISP_COMPARE_H

enum a_order {A_LT = -1, A_EQ, A_GT};

typedef enum a_order (*a_compare)(const void *x, const void *y);

#endif
