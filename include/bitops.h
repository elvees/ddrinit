// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _BITOPS_H
#define _BITOPS_H

#define BIT(n) (1 << (n))
#define GENMASK(hi, lo) (((1 << ((hi) - (lo) + 1)) - 1) << (lo))

#define __bf_shf(x) (__builtin_ffs(x) - 1)

#define FIELD_PREP(_mask, _val) ({ ((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask); })
#define FIELD_GET(_mask, _reg) ({ (typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask)); })

#endif /* _BITOPS_H */
