s/\<cu_mutex_cct\>/cu_mutex_init/g
s/\<cuoo_stdtype_t\>/cuoo_type_t/g
s/\<cuoo_stdtype_s\>/cuoo_type_s/g
s/\<cuoo_stdtype_type\>/cuoo_type_type/g
s/\<cuoo_stdtype_new\>/cuoo_type_new_opaque/g
s/\<cuoo_stdtype_new_hcs\>/cuoo_type_new_opaque_hcs/g
s/\<cuoo_stdtype_new_hcv\>/cuoo_type_new_opaque_hcv/g
s/\<cuoo_stdtype_cct\>/cuoo_type_init_opaque/g
s/\<cuoo_stdtype_cct_hcs\>/cuoo_type_init_opaque_hcs/g
s/\<cuoo_stdtype_cct_hcv\>/cuoo_type_init_opaque_hcv/g
s/\<cuoo_stdtype_from_meta\>/cuoo_type_from_meta/g
s/\<cuoo_stdtype_to_meta\>/cuoo_type_to_meta/g
s/\<cuoo_typekind_stdtype\>/cuoo_typekind_opaque/g
s/\<cuoo_typekind_stdtypeoftypes\>/cuoo_typekind_metatype/g
s/\<cuoo_stdtypeoftypes_new\(\|_hc[es]\)\>/cuoo_type_new_metatype\1/g
s/\<cuoo_type_is_stdtype\>/cuoo_type_is_metatype/g

s/\<cuoo_typekind_elmtype_\(\w\+\)/CUOO_SHAPE_SCALAR_\U\1/g
s/\<cuoo_typekind_\(\w\w\+\)/CUOO_SHAPE_\U\1/g
s/\<cuoo_typekind_t\>/cuoo_shape_t/g
s/\<cuoo_type_typekind\>/cuoo_type_shape/g
s/\<cuoo_type_is_typeoftypes\>/cuoo_type_is_metatype/g

s/\<cucon_rbtree_cct\>/cucon_rbtree_init/g
s/\<cucon_rbset_cct\>/cucon_rbset_init/g
s/\<cucon_rbset_cct_str_cmp\>/cucon_rbset_init_str_cmp/g
s/\<cucon_queue_cct\>/cucon_queue_init/g

s/\<cucon_arr_cct_empty\>/cucon_arr_init_empty/g
s/\<cucon_arr_cct_size\>/cucon_arr_init_size/g
s/\<cucon_parr_cct_empty\>/cucon_parr_init_empty/g
s/\<cucon_parr_cct_size\>/cucon_parr_init_size/g
s/\<cucon_parr_cct_fill\>/cucon_parr_init_fill/g

s/\<cuex_subst_cct\>/cuex_subst_init/g
s/\<cuex_subst_cct_nonidem\>/cuex_subst_init_nonidem/g
s/\<cuex_subst_cct_uw\>/cuex_subst_cct_uw/g
s/\<cuex_subst_cct_e\>/cuex_subst_cct_e/g
s/\<cuex_subst_cct_n\>/cuex_subst_cct_n/g
s/\<cuex_otab_cct\>/cuex_otab_init/g

s/\<cuflow_promise_cct\>/cuflow_promise_init/g
s/\<cuflow_gflexq_cct\>/cuflow_gflexq_init/g
s/\<cuflow_workq_cct\>/cuflow_workq_init/g

s/\<custo_fread_str_cct\>/custo_fread_str_init/g

s/\<cutext_src_cct\>/cutext_src_init/g
s/\<cutext_src_cct_move\>/cutext_src_init_grab/g
s/\<cutext_src_new_move\>/cutext_src_new_grab/g
s/\<cutext_ucs4src_cct\>/cutext_ucs4src_init/g
s/\<cutext_ucs4src_cct_detect\>/cutext_ucs4src_init_detect/g

s/\<cu_rarex_cct\>/cu_rarex_init/g

s/\<cuos_argv_cct_strlist\>/cuos_argv_init_strlist/g
