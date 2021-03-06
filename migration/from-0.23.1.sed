# 2009-11-15 Allocation function names
s/\<cu_galloc_a\>/cu_galloc_atomic/g
s/\<cu_galloc_u\>/cu_ualloc/g
s/\<cu_galloc_au\>/cu_ualloc_atomic/g
s/\<cu_cgalloc\>/cu_gallocz/g
s/\<cu_cgalloc_a\>/cu_gallocz_atomic/g
s/\<cu_cgalloc_u\>/cu_uallocz/g
s/\<cu_gfree_a\>/cu_gfree_atomic/g
s/\<cu_gfree_u\>/cu_ufree/g
s/\<cu_gfree_au\>/cu_ufree_atomic/g
s/\<cu_gnew_a\>/cu_gnew_atomic/g
s/\<cu_gnew_u\>/cu_unew/g
s/\<cu_gnew_au\>/cu_unew_atomic/g
s/\<cu_cgnew\>/cu_gnewz/g
s/\<cu_gnewarr_a\>/cu_gnewarr_atomic/g
s/\<cu_gnewarr_u\>/cu_unewarr/g
s/\<cu_gnewarr_au\>/cu_unewarr_atomic/g

# 2009-11-16 Hash maps and sets
s/\<cucon_hmap_size\>/cucon_hmap_card/g
s/\<cucon_hmap_erase\>/cucon_hmap_pop_mem/g
s/\<cucon_hmap_erase_keep_capacity\>/cucon_hmap_isocap_pop_mem/g
s/\<cucon_hset_size\>/cucon_hset_card/g
s/\<cucon_hset_erase_keep_capacity\>/cucon_isocap_erase/g

# 2009-11-22 Dropping struct name suffix
s/\<cu_buffer_s\>/cu_buffer/g
s/\<cu_dlink_s\>/cu_dlink/g
s/\<cu_dbufsink_s\>/cu_dbufsink/g
s/\<cu_dcountsink_s\>/cu_dcountsink/g
s/\<cu_dsink_s\>/cu_dsink/g
s/\<cu_dsource_s\>/cu_dsource/g
s/\<cu_idr_s\>/cu_idr/g
s/\<cu_log_facility_s\>/cu_log_facility/g
s/\<cu_ptr_array_source_s\>/cu_ptr_array_source/g
s/\<cu_ptr_source_s\>/cu_ptr_source/g
s/\<cu_ptr_sink_s\>/cu_ptr_sink/g
s/\<cu_ptr_junction_s\>/cu_ptr_junction/g
s/\<cu_ptr_sinktor_s\>/cu_ptr_sinktor/g
s/\<cu_ptr_junctor_s\>/cu_ptr_junctor/g
s/\<cu_sref_s\>/cu_sref/g
s/\<cu_str_s\>/cu_str/g
s/\<cu_wstring_s\>/cu_wstring/g
s/\<cu_hook_node_s\>/cu_hook_node/g
s/\<cu_installdir_s\>/cu_installdir/g
s/\<cu_hidden_ptr_s\>/cu_hidden_ptr/g
s/\<cu_scratch_s\>/cu_scratch/g
s/\<cucon_arr_s\>/cucon_arr/g
s/\<cucon_bitvect_s\>/cucon_bitvect/g
s/\<cucon_digraph_s\>/cucon_digraph/g
s/\<cucon_digraph_vertex_s\>/cucon_digraph_vertex/g
s/\<cucon_digraph_edge_s\>/cucon_digraph_edge/g
s/\<cucon_hmap_s\>/cucon_hmap/g
s/\<cucon_hzmap_s\>/cucon_hzmap/g
s/\<cucon_hzmap_itr_s\>/cucon_hzmap_itr/g
s/\<cucon_hzset_s\>/cucon_hzset/g
s/\<cucon_hzset_itr_s\>/cucon_hzset_itr/g
s/\<cucon_hset_s\>/cucon_hset/g
s/\<cucon_list_s\>/cucon_list/g
s/\<cucon_listnode_s\>/cucon_listnode/g
s/\<cucon_pcmap_s\>/cucon_pcmap/g
s/\<cucon_pmap_s\>/cucon_pmap/g
s/\<cucon_parr_s\>/cucon_parr/g
s/\<cucon_poelt_s\>/cucon_poelt/g
s/\<cucon_po_s\>/cucon_po/g
s/\<cucon_priq_s\>/cucon_priq/g
s/\<cucon_pritree_s\>/cucon_pritree/g
s/\<cucon_prinode_s\>/cucon_prinode/g
s/\<cucon_pset_s\>/cucon_pset/g
s/\<cucon_rbmap_s\>/cucon_rbmap/g
s/\<cucon_rbset_s\>/cucon_rbset/g
s/\<cucon_rbnode_s\>/cucon_rbnode/g
s/\<cucon_rbtree_s\>/cucon_rbtree/g
s/\<cucon_rpmap_s\>/cucon_rpmap/g
s/\<cucon_rumap_s\>/cucon_rumap/g
s/\<cucon_slink_s\>/cucon_slink/g
s/\<cucon_stack_s\>/cucon_stack/g
s/\<cucon_stack_itr_s\>/cucon_stack_itr/g
s/\<cucon_umap_s\>/cucon_umap/g
s/\<cucon_uset_s\>/cucon_uset/g
s/\<cucon_ucmap_s\>/cucon_ucmap/g
s/\<cucon_ucset_s\>/cucon_ucset/g
s/\<cucon_ucset_itr_s\>/cucon_ucset_itr/g
s/\<cucon_wmap_s\>/cucon_wmap/g
s/\<cucon_queue_s\>/cucon_queue/g
s/\<cucon_hzmap_node_s\>/cucon_hzmap_node/g
s/\<cucon_hzset_node_s\>/cucon_hzset_node/g
s/\<cucon_pmap_node_s\>/cucon_pmap_node/g
s/\<cucon_hmap_node_s\>/cucon_hmap_node/g
s/\<cucon_queue_node_s\>/cucon_queue_node/g
s/\<cucon_umap_node_s\>/cucon_umap_node/g
s/\<cucon_hmap_itr_s\>/cucon_hmap_itr/g
s/\<cuoo_layout_s\>/cuoo_layout/g
s/\<cuoo_prop_s\>/cuoo_prop/g
s/\<cuoo_type_s\>/cuoo_type/g
s/\<cuoo_libinfo_s\>/cuoo_libinfo/g
s/\<cuoo_intf_serialisable_s\>/cuoo_intf_serialisable/g
s/\<cuos_dirpile_s\>/cuos_dirpile/g
s/\<cuos_pkg_user_dirs_s\>/cuos_pkg_user_dirs/g
s/\<cutext_sink_s\>/cutext_sink/g
s/\<cutext_countsink_s\>/cutext_countsink/g
s/\<cutext_buffersink_s\>/cutext_buffersink/g
s/\<cutext_src_s\>/cutext_src/g
s/\<cutext_ucs4src_s\>/cutext_ucs4src/g
s/\<cutext_sink_descriptor_s\>/cutext_sink_descriptor/g
s/\<cufo_convinfo_s\>/cufo_convinfo/g
s/\<cufo_prispec_s\>/cufo_prispec/g
s/\<cufo_stream_s\>/cufo_stream/g
s/\<cufo_tag_s\>/cufo_tag/g
s/\<cufo_namespace_s\>/cufo_namespace/g
s/\<cufo_attr_s\>/cufo_attr/g
s/\<cufo_attrbind_s\>/cufo_attrbind/g
s/\<cufo_termface_s\>/cufo_termface/g
s/\<cufo_termstyle_s\>/cufo_termstyle/g
s/\<cufo_textsink_s\>/cufo_textsink/g
s/\<cufo_textstyle_s\>/cufo_textstyle/g
s/\<cufo_textstyler_s\>/cufo_textstyler/g
s/\<cuflow_tstate_s\>/cuflow_tstate/g
s/\<cuflow_continuation_s\>/cuflow_continuation/g
s/\<cuflow_cacheconf_s\>/cuflow_cacheconf/g
s/\<cuflow_gflexq_s\>/cuflow_gflexq/g
s/\<cuflow_promise_s\>/cuflow_promise/g
s/\<cuflow_workq_s\>/cuflow_workq/g
s/\<cuflow_exeq_s\>/cuflow_exeq/g
s/\<cuflow_exeq_entry_s\>/cuflow_exeq_entry/g
s/\<cuflow_gflexq_entry_s\>/cuflow_gflexq_entry/g
s/\<cuflow_cache_s\>/cuflow_cache/g
s/\<cuflow_cacheobj_s\>/cuflow_cacheobj/g
s/\<cugra_graph_s\>/cugra_graph/g
s/\<cugra_vertex_s\>/cugra_vertex/g
s/\<cugra_adjlink_s\>/cugra_adjlink/g
s/\<cugra_arc_s\>/cugra_arc/g
s/\<cugra_graph_with_arcset_s\>/cugra_graph_with_arcset/g
s/\<cugra_walk_SCC_s\>/cugra_walk_SCC/g
s/\<cugra_walk_SCC_vt_s\>/cugra_walk_SCC_vt/g
s/\<cudyn_proto_s\>/cudyn_proto/g
s/\<cudyn_arrtype_s\>/cudyn_arrtype/g
s/\<cudyn_ctortype_s\>/cudyn_ctortype/g
s/\<cudyn_inltype_s\>/cudyn_inltype/g
s/\<cudyn_elmtype_s\>/cudyn_elmtype/g
s/\<cudyn_ptrtype_s\>/cudyn_ptrtype/g
s/\<cudyn_sngtype_s\>/cudyn_sngtype/g
s/\<cudyn_tuptype_s\>/cudyn_tuptype/g
s/\<cudyn_duntype_s\>/cudyn_duntype/g
s/\<cuex_monoid_s\>/cuex_monoid/g
s/\<cuex_tmonoid_s\>/cuex_tmonoid/g
s/\<cuex_monoid_itr_s\>/cuex_monoid_itr/g
s/\<cuex_tmonoid_itr_s\>/cuex_tmonoid_itr/g
s/\<cuex_intf_compound_s\>/cuex_intf_compound/g
s/\<cuex_ltree_itr_s\>/cuex_ltree_itr/g
s/\<cuex_fpvar_s\>/cuex_fpvar/g
s/\<cuex_gvar_s\>/cuex_gvar/g
s/\<cuex_occurtree_s\>/cuex_occurtree/g
s/\<cuex_opn_s\>/cuex_opn/g
s/\<cuex_opn_source_s\>/cuex_opn_source/g
s/\<cuex_oprinfo_s\>/cuex_oprinfo/g
s/\<cuex_pvar_s\>/cuex_pvar/g
s/\<cuex_subst_s\>/cuex_subst/g
s/\<cuex_veqv_s\>/cuex_veqv/g
s/\<cuex_tpvar_s\>/cuex_tpvar/g
s/\<cuex_tvar_s\>/cuex_tvar/g
s/\<cuex_var_s\>/cuex_var/g
s/\<cuex_ssfn_s\>/cuex_ssfn/g
s/\<cuex_ssfn_find_it_s\>/cuex_ssfn_find_it/g
s/\<cuex_ssfn_matchinfo_s\>/cuex_ssfn_matchinfo/g
s/\<cuex_ssfn_node_s\>/cuex_ssfn_node/g
s/\<cuex_stats_s\>/cuex_stats/g
s/\<cuex_ltree_node_s\>/cuex_ltree_node/g
s/\<cuex_xvarops_s\>/cuex_xvarops/g
s/\<cu_upcast\>/cu_to/g
s/\<cu_downcast\>/cu_from/g
