s/#include <cuex\/opr\.h>/#include <cuex\/oprdefs.h>/g
s/\<cuex_o\([0-9]\)(\(\w\+\),\s*/cuex_o\1_\2(/g
s/\<cuex_opr_null_0\>/CUEX_O0_NULL/g
s/\<cuex_opr_unknown_0\>/CUEX_O0_UNKNOWN/g
s/\<cuex_opr_ident_1\>/CUEX_O1_IDENT/g
s/\<cuex_opr_lambda_2\>/CUEX_O2_LAMBDA/g
s/\<cuex_opr_lambda_ta_3\>/CUEX_O3_LAMBDA_TA/g
s/\<cuex_opr_forall_2\>/CUEX_O2_FORALL/g
s/\<cuex_opr_forall_3\>/CUEX_O3_FORALL/g
s/\<cuex_opr_exists_2\>/CUEX_O2_EXISTS/g
s/\<cuex_opr_exists_3\>/CUEX_O3_EXISTS/g
s/\<cuex_opr_let_3\>/CUEX_O3_LET/g
s/\<cuex_opr_subst_block_1\>/CUEX_O1_SUBST_BLOCK/g
s/\<cuex_opr_apply_2\>/CUEX_O2_APPLY/g
s/\<cuex_opr_tuple_2\>/CUEX_O2M_TUPLE/g
s/\<cuex_opr_tuple_r\>/CUEX_OR_TUPLE/g
s/\<cuex_opr_rbind_2\>/CUEX_O2_RBIND/g
s/\<cuex_opr_if_3\>/CUEX_O3_IF/g
s/\<cuex_opr_gprod_2\>/CUEX_O2_GPROD/g
s/\<cuex_opr_gexpt_2\>/CUEX_O2_GEXPT/g
s/\<cuex_opr_farrow_2\>/CUEX_O2_FARROW/g
s/\<cuex_opr_farrow_native_2\>/CUEX_O2_FARROW_NATIVE/g
s/\<cuex_opr_ptr_to_1\>/CUEX_O1_PTR_TO/g
s/\<cuex_opr_prod_2aci1\>/CUEX_O3ACI_PROD/g
s/\<cuex_opr_sigprod_2aci2\>/CUEX_O4ACI_SIGPROD/g
s/\<cuex_opr_sigbase_2\>/CUEX_O2_SIGBASE/g
s/\<cuex_opr_singleton_1\>/CUEX_O1_SINGLETON/g
s/\<cuex_opr_vtable_slots_1\>/CUEX_O1_VTABLE_SLOTS/g
s/\<cuex_opr_structjoin_2aci2\>/CUEX_O4ACI_STRUCTJOIN/g
s/\<cuex_opr_dunion_2aci2\>/CUEX_O4ACI_DUNION/g
s/\<cuex_opr_setjoin_2aci1\>/CUEX_O3ACI_SETJOIN/g
s/\<cuex_opr_ident_ta_2\>/CUEX_O2_IDENT_TA/g
s/\<cuex_opr_load_ta_3\>/CUEX_O3_LOAD_TA/g
s/\<cuex_opr_call_ta_r\>/CUEX_OR_CALL_TA/g

s/\<cuex_opr_is_scoping\>/cuex_og_scoping_contains/g
