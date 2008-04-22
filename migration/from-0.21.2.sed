s/#include <cutext\/ucs4ctype\.h>/#include <cutext\/wccat.h>/g
s/#include <cutext\/ucs4\.h>/#include <cutext\/conv.h>/

s/\<cutext_ucs4ctype\>/cutext_wchar_wccat/g
s/\<cutext_ucs4ctype_t\>/cutext_wccat_t/g
s/\<cutext_ucs4ctype_none\>/CUTEXT_WCCAT_NONE/g
s/\<cutext_ucs4ctype_\(\w\w\)\>/CUTEXT_WCCAT_\U\1/g
s/\<cutext_ucs4ctype_is_printable\>/cutext_wccat_is_print/g
s/\<cutext_ucs4ctype_is_otherucscat\>/cutext_wccat_is_other/g
s/\<cutext_ucs4ctype_/cutext_wccat_/g

s/\<cutext_ucs4char_t\>/cu_wchar_t/g
s/\<cutext_ucs4char_is_printable\>/cutext_wchar_is_print/g
s/\<cutext_ucs4char_is_otherucscat\>/cutext_wchar_is_other/g
s/\<cutext_ucs4char_/cutext_wchar_/g

s/\<cutext_chenc_t\>/cutext_encoding_t/g
s/\<cutext_chenc_name\>/cutext_encoding_name/g
s/\<cutext_chenc_to_cstr\>/cutextP_encoding_name/g
s/\<cutext_chenc_from_cstr\>/cutext_encoding_by_name/g
s/\<cutext_chenc_\(\w\+\)\>/CUTEXT_ENCODING_\U\1/g

s/\<cutext_ucs4arr_to_charr\>/cutext_iconv_wchar_to_char/g
s/\<cutext_charr_to_ucs4arr\>/cutext_iconv_char_to_wchar/g

s/\<cu_dlink_cct_singular\>/cu_dlink_init_singleton/g
s/\<cu_scratch_cct\>/cu_scratch_init/g
s/\<cu_sref_cct\>/cu_sref_init/g
s/\<cu_sref_cct_range\>/cu_sref_init_range/g
s/\<cu_sref_cct_sref_range\>/cu_sref_init_span/g
s/\<cu_sref_cct_sref_first\>/cu_sref_init_first/g
s/\<cu_sref_cct_sref_last\>/cu_sref_init_last/g
s/\<cu_sref_cct_copy\>/cu_sref_init_copy/g
s/\<cu_str_cct\>/cu_str_init/g
s/\<cu_str_cct_cstr_static\>/cu_str_init_static_cstr/g
s/\<cu_str_new_cstr_static\>/cu_str_new_static_cstr/g
s/\<cu_str_onew_cstr_static\>/cu_str_onew_static_cstr/g
s/\<cu_str_cct_uninit\>/cu_str_init_uninit/g
s/\<cu_str_cct_copy\>/cu_str_init_copy/g
s/\<cu_weakptr_cct\>/cu_weakptr_init/g
s/\<cu_weakptr_cct_null\>/cu_weakptr_init_null/g
s/\<cucon_bitvect_cct_uninit\>/cucon_bitvect_init_uninit/g
s/\<cucon_bitvect_cct_fill\>/cucon_bitvect_init_fill/g
s/\<cucon_bitvect_cct_copy\>/cucon_bitvect_init_copy/g
s/\<cucon_hmap_cct\>/cucon_hmap_init/g
s/\<cucon_hset_cct\>/cucon_hset_init/g
s/\<cucon_list_cct\>/cucon_list_init/g
s/\<cucon_list_cct_copy_ptr\>/cucon_list_init_copy_ptr/g
s/\<cucon_list_insert_node_cct\>/cucon_list_insert_init_node/g
s/\<cucon_list_prepend_node_cct\>/cucon_list_prepend_init_node/g
s/\<cucon_list_append_node_cct\>/cucon_list_append_init_node/g
s/\<cucon_pmap_cct\>/cucon_pmap_init/g
s/\<cucon_pmap_cct_copy_void\>/cucon_pmap_init_copy_void/g
s/\<cucon_pmap_cct_copy_mem\>/cucon_pmap_init_copy_mem/g
s/\<cucon_pmap_cct_copy_mem_ctor\>/cucon_pmap_init_copy_mem_ctor/g
s/\<cucon_pmap_cct_copy_node\>/cucon_pmap_init_copy_node/g
s/\<cucon_priq_cct\>/cucon_priq_init/g
s/\<cucon_priq_cct_copy\>/cucon_priq_init_copy/g
s/\<cucon_pritree_cct\>/cucon_pritree_init/g
s/\<cucon_pritree_insert_cct\>/cucon_pritree_insert_init/g
s/\<cucon_pset_cct\>/cucon_pset_init/g
s/\<cucon_pset_cct_copy\>/cucon_pset_init_copy/g
s/\<cucon_rpmap_cct\>/cucon_rpmap_init/g
s/\<cucon_rumap_cct\>/cucon_rumap_init/g
s/\<cucon_stack_cct\>/cucon_stack_init/g
s/\<cucon_stack_cct_copy\>/cucon_stack_init_copy/g
s/\<cucon_umap_cct\>/cucon_umap_init/g
s/\<cucon_umap_cct_copy_void\>/cucon_umap_init_copy_void/g
s/\<cucon_umap_cct_copy_mem\>/cucon_umap_init_copy_mem/g
s/\<cucon_umap_cct_copy_mem_ctor\>/cucon_umap_init_copy_mem_ctor/g
s/\<cucon_umap_cct_copy_node\>/cucon_umap_init_copy_node/g
s/\<cucon_uset_cct\>/cucon_uset_init/g
s/\<cucon_uset_cct_copy\>/cucon_uset_init_copy/g
s/\<cugra_graph_cct\>/cugra_graph_init/g
