# cudyn, cuoo, cu rearrangements
s;<cudyn/\(prop\|properties\);<cuoo/\1;g
s;cudyn\(P\)\?_\(compound\|[ho]alloc\|hctem\|[ho]new\|type_\|type\>\|is_type\>\|stdtype\|typekind\|prop_\|property\|properties\|hcmethod\|propkey\|raw_\|obj_\|sref\);cuoo\1_\2;g
s;CUDYN_\(OBJ_\|HCOBJ_\|ENABLE_KEYED_PROP\);CUOO_\1;g
s;\<CU_OBJ\>;CUOO_OBJ;g
s;\<CU_OBJ_;CUOO_OBJ_;g
s;\<CU_HCOBJ;CUOO_HCOBJ;g
s;\<CU_HC_;CUOO_HC_;g
s;#include <cu/\([ho]alloc\);#include <cuoo/\1;g

# cucon_layout -> cuoo_layout
s;#include <cucon/layout;#include <cuoo/layout;g
s;\<cucon_layout;cuoo_layout;g
s;\<CUCON_LAYOUT;CUOO_LAYOUT;g
