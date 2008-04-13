#! /bin/sh

etype_doc()
{
    name="$1"
    type_t="$2"
    cat <<END
/*!The type descriptor for dynamically typed \\c $type_t objects. */
cudyn_type_t cudyn_${name}_type(void);

/*!Returns a hashconsed dynamically typed representation of \\c x. */
cuex_t cudyn_${name}($type_t x);

/*!True iff \\a e was created with cudyn_${name}. */
cu_bool_t cudyn_is_${name}(cuex_t e);

/*!If \\a e was created with <tt>cudyn_${name}(\e x)</tt>, for some
 * \e x, returns \e x, else undefied bohaviour. */
$type_t cudyn_to_${name}(cuex_t e);

END
}

arrtype_doc()
{
    name="$1"
    type_t="$2"
    cat <<END
/*!Type type descriptor for dynamically typed \\c $type_t[\a cnt] arrays. */
cudyn_type_t cudyn_${name}arr_type(size_t cnt);

/*!Returns a hashconsed dynamically typed array of \\c cnt elements
 *initialised from \\c arr. */
cuex_t cudyn_${name}arr(size_t cnt, $type_t *arr);

/*!If \\a e is a dynamically typed \\c $type_t[\\c cnt] array for some
 * \\c cnt, return \\c cnt, else return \\c cudyn_condsize_none. */
size_t cudyn_${name}arr_condsize(cuex_t e);

/*!Return the array size of \a e without checking type.
 * \pre e is an array. */
size_t cudyn_${name}arr_size(cuex_t e);

/*!If \\a e is a dynamically typed \\c $type_t[\\c cnt] array for some
 * \\c cnt, return a pointer to the elements, else undefined behaviour. */
$type_t *cudyn_${name}arr_arr(cuex_t e);

/*!Assuming \\a e is a dynamically typed \\c $type_t[\\c cnt] array for
 * some \\c cnt > \\a i, return a pointer to element \\a i. */
$type_t *cudyn_${name}arr_at(cuex_t e, size_t i);
END
}

etypearr_doc()
{
    etype_doc "$@"
    arrtype_doc "$@"
}

bool_doc()
{
    etype_doc bool cu_bool_t
    cat <<END
/*!\\def cudyn_true
 * The dynamic boolean true value. */
/*!\\def cudyn_false
 * The dynamic boolean false value. */
END
}

wrapmod()
{
    echo "/*!\\name Dynamic $3"
    echo " * @{\\ingroup cudyn_misc_h */"
    "$@"
    echo "/*!@}*/"
}

cat <<END
/*!\\defgroup cudyn_misc_h cudyn/misc.h: Various Dynamic Types
 * @{ \\ingroup cudyn_mod
 * This submodule implements dynamically typed integers, floats, chars,
 * and cu_bool_t.  For integers, there are two schemes, one based
 * on the \c stdint.h definitions for \c int8_t, \c int16_t, \c int32_t,
 * \c int64_t, and their unsigned variants, and one based on the
 * ordinary C integers.  The latter scheme uses aliases to the former,
 * so their types may not be distinct.
 */
END
for prec in 8 16 32 64; do
    wrapmod etypearr_doc int${prec} int${prec}_t
    wrapmod etypearr_doc uint${prec} uint${prec}_t
done
wrapmod bool_doc bool cu_bool_t
wrapmod etypearr_doc float float
wrapmod etypearr_doc double double
for t in char short int long; do
    wrapmod etypearr_doc $t $t
    wrapmod etypearr_doc u$t "unsigned $t"
done
wrapmod arrtype_doc cuex cuex_t
cat <<END
/*!@}*/
END
