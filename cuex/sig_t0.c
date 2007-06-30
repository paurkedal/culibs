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

#include <cu/diag.h>
#include <cuex/sig.h>
#include <cuex/type.h>
#include <cudyn/misc.h>

cu_clos_def(check_sigcomp,
	    cu_prot(cu_bool_t, cu_idr_t label,
			       cuex_t type, cu_offset_t bitoff),
    ( cuex_t sig;
      cu_word_t *bitmap;
      size_t bitmap_size;
      cu_offset_t bitoff; ))
{
    cu_clos_self(check_sigcomp);
    cu_offset_t i;
    cu_offset_t bitsize = cuex_type_bitsize(type);
    cu_offset_t bitalign = cuex_type_bitalign(type);
    cuex_t type_ck;
    cu_offset_t bitoff_ck;
    if (!cuex_sig_find(self->sig, label, &type_ck, &bitoff_ck))
	cu_debug_unreachable();
    cu_debug_assert(type == type_ck);
    cu_debug_assert(bitoff == bitoff_ck);
    cu_debug_assert(bitoff % bitalign == 0);
    bitoff += self->bitoff;
    if (cuex_is_sig(type)) {
	check_sigcomp_t ck;
	ck.bitmap_size = self->bitmap_size;
	ck.bitmap = self->bitmap;
	ck.sig = type;
	ck.bitoff = bitoff;
	return cuex_sig_conj(type, check_sigcomp_prep(&ck));
    }
    else {
	for (i = 0; i < bitsize; ++i) {
	    cu_offset_t iw = bitoff / (sizeof(cu_word_t)*8);
	    cu_offset_t ib = bitoff % (sizeof(cu_word_t)*8);
	    cu_debug_assert(iw < self->bitmap_size);
	    cu_debug_assert((self->bitmap[iw] & (1UL << ib)) == 0);
	    self->bitmap[iw] |= 1UL << ib;
	    ++bitoff;
	}
	return cu_true;
    }
}

void
check_sig(cuex_t sig)
{
    cu_offset_t bitsize = cuex_type_bitsize(sig);
    check_sigcomp_t ck;
    int i;
    int filled = 0;
    ck.bitmap_size = (bitsize+sizeof(cu_word_t)*8-1)/(sizeof(cu_word_t)*8);
    ck.bitmap = cu_galloc(ck.bitmap_size*sizeof(cu_word_t));
    ck.sig = sig;
    ck.bitoff = 0;
    memset(ck.bitmap, 0, ck.bitmap_size*sizeof(cu_word_t));
    cuex_sig_conj(sig, check_sigcomp_prep(&ck));
    for (i = 0; i < bitsize; ++i) {
	cu_offset_t iw = i / (sizeof(cu_word_t)*8);
	cu_offset_t ib = i % (sizeof(cu_word_t)*8);
	int v = (ck.bitmap[iw] >> ib) & 1;
	filled += v;
    }
    if (filled != bitsize)
	printf("Filled %6d of %6d bits, unfilled frac. %10.3lg\n",
	       filled, bitsize, (bitsize - filled)/(double)bitsize);
}

cuex_t
random_sig(int base_cnt_max, int comp_cnt_max)
{
    static int label_cnt = 0;
    int i;
    int n = lrand48() % (comp_cnt_max + 1);
    cuex_t sig = cuex_sig_identity();
    for (i = 0; i < n; ++i) {
	cuex_t type;
	char label_buffer[2+sizeof(int)*3];
	cuex_t label;
	sprintf(label_buffer, "l%d", label_cnt++);
	label = cu_idr_by_cstr(label_buffer);
	switch (lrand48() % 6) {
	    case 0: type = cudyn_bool_type(); break;
	    case 1: type = cudyn_int8_type(); break;
	    case 2: type = cudyn_int16_type(); break;
	    case 3: type = cudyn_int32_type(); break;
	    case 4: type = cudyn_int64_type(); break;
	    default: type = random_sig(base_cnt_max/n,
				       comp_cnt_max/n);
	}
	sig = cuex_sigprod_generator(sig, label, type);
    }
    n = lrand48() % (base_cnt_max + 1);
    for (i = 0; i < n; ++i) {
	sig = cuex_o2_sigbase(
		      random_sig(base_cnt_max/n, comp_cnt_max/n),
		      sig);
    }
    return sig;
}

void
stress_test()
{
    int i;
    for (i = 0; i < 100; ++i) {
	cuex_t sig = random_sig(32, 512);
	check_sig(sig);
    }
}

cu_clos_def(show_sigcomp,
	    cu_prot(cu_bool_t, cu_idr_t label,
			       cuex_t type, cu_offset_t bitoff),
    ( cu_offset_t bitsize_sum;
      cu_offset_t bound; ))
{
    cu_clos_self(show_sigcomp);
    cu_offset_t bitsize;
    bitsize = cuex_type_bitsize(type);
    cu_fprintf(stdout, "%! ∈ %!; %d bits at %d\n", label, type, bitsize, bitoff);
    self->bitsize_sum += bitsize;
    if (bitoff + bitsize > self->bound)
	self->bound = bitoff + bitsize;
    return cu_true;
}

void
sig_show_comp(cuex_opn_t sig, char const *idr_cstr)
{
    cu_idr_t idr = cu_idr_by_cstr(idr_cstr);
    cuex_t type;
    cu_offset_t bitoffset;
    if (!cuex_sig_find(sig, idr, &type, &bitoffset))
	cu_bugf("could not find %!", idr);
    cu_fprintf(stdout, "%! ∈ %! @ %d\n", idr, type, bitoffset);
}

void
check_size_and_align(int n, ...)
{
    cuex_opn_t sig;
    size_t bitsize = 0;
    size_t bitalign = 1;
    int i;
    va_list va;
    va_start(va, n);
    sig = cuex_sig_va(n, va);
    va_end(va);
    va_start(va, n);
    for (i = 0; i < n; ++i) {
	cuex_t e;
	cu_offset_t s;
	va_arg(va, char const *);
	e = va_arg(va, cuex_t);
	s = cuex_type_bitalign(e);
	if (s > bitalign)
	    bitalign = s;
	bitsize += cuex_type_bitsize(e);
    }
    va_end(va);
    cu_debug_assert(cuex_type_bitalign(sig) == bitalign);
    cu_debug_assert(cuex_type_bitsize(sig)
		    == (bitsize + bitalign - 1)/bitalign*bitalign);
}

void
test_sig()
{
    cuex_opn_t sig;
    show_sigcomp_t cb;
    sig = cuex_sig_v(4,
		     "i8", cudyn_int8_type(),
		     "xdbl", cudyn_double_type(),
		     "i16", cudyn_int16_type(),
		     "i32", cudyn_int32_type());
    cb.bitsize_sum = 0;
    cb.bound = 0;
    cuex_sig_conj(sig, show_sigcomp_prep(&cb));
    sig_show_comp(sig, "i8");
    sig_show_comp(sig, "xdbl");
    sig_show_comp(sig, "i16");
    sig_show_comp(sig, "i32");
    cu_debug_assert(cb.bitsize_sum == cb.bound);
    check_size_and_align(2, "i8", cudyn_int8_type(), "i16", cudyn_int16_type());
    check_size_and_align(3, "i32", cudyn_int32_type(), "x", cudyn_double_type(), "i8", cudyn_int8_type());
    cu_verbf(0, "Checking signature %!", sig);
    check_sig(sig);
}

int
main()
{
    cuex_init();
    test_sig();
    stress_test();
    return 0;
}
