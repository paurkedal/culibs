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

