/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cu/dyn.h>
#include <cu/hash.h>
#include <cu/algo.h>
#include <cudyn/algo.h>

typedef struct kat_s *kat_t;
struct kat_s { CU_HCOBJ
    int i;
    void *sub0;
    void *sub1;
};

cudyn_stdtype_t kat_type_ptr;
#define kat_type() kat_type_ptr

cu_clop_def(kat_eq, cu_bool_t, void *s0, void *s1)
{
#define s0 ((kat_t)s0)
#define s1 ((kat_t)s1)
    return s0->i == s1->i && s0->sub0 == s1->sub0 && s0->sub1 == s1->sub1;
#undef s0
#undef s1
}

cu_clop_def(kat_hash, cu_hash_t, void *s0)
{
#define s0 ((kat_t)s0)
    cu_hash_t hash = cu_hash_mix3(s0->i);
    hash = cu_hash_mix3(hash + (uintptr_t)s0->sub0);
    hash = cu_hash_mix3(hash + (uintptr_t)s0->sub1);
    return hash;
#undef s0
}

cu_clop_def(kat_conj, cu_bool_t, void *k, cu_clop(fn, cu_bool_t, void *))
{
#define k ((kat_t)k)
    return (!k->sub0 || cu_call(fn, k->sub0))
	&& (!k->sub1 || cu_call(fn, k->sub1));
#undef k
}

kat_t kat_new(int i, void *sub0, void *sub1)
{
    struct kat_s data = { CU_HCOBJ_INIT i, sub0, sub1 };
    return cudyn_hnew(kat, &data);
}

cu_clos_def(ff, cu_prot(cu_bool_t, void *k),
    (int level;))
{
    cu_clos_self(ff);
    if (cudyn_stdtype_from_meta(cuex_meta(k)) == kat_type_ptr)
#define k ((kat_t)k)
	printf("[%d] %d %p %p\n", self->level, k->i, k->sub0, k->sub1);
#undef k
    else
	printf("[%d] %p\n", self->level, k);
    ++self->level;
    cudyn_conj(k, ff_ref(self));
    --self->level;
    return cu_true;
}

void
submain()
{
    kat_t k1, k2, k3, k4;
    ff_t ff_c;
    kat_type_ptr = cudyn_stdtype_new_hcs(sizeof(struct kat_s) - CU_HCOBJ_SHIFT);
    kat_type_ptr->conj = kat_conj;
    k1 = kat_new(1, NULL, NULL);
    k2 = kat_new(2, NULL, NULL);
    assert(k2 == kat_new(2, NULL, NULL));
    k3 = kat_new(3, k1, k2);
    assert(k3 == kat_new(3, k1, k2));
    k4 = kat_new(4, k1, k3);
    ff_c.level = 0;
    cudyn_conj(k4, ff_prep(&ff_c));
}

int main()
{
    cu_init();
    submain();
    return 0;
}
