#! /usr/bin/env perl
use strict;
use POSIX;

# IMPORTANT. If $unicode_max or $blk_size is changed, wccat.c must be updated!
my $unicode_max = 0x1fffff;
my $blk_size = 512;
my $blk_cnt = ($unicode_max + 1)/$blk_size;

my $unicode_data = "UnicodeData.txt";
my @general_category_arr;

while (<STDIN>) {
    my ( $codepoint,
	 $name,
	 $general_category,
	 $canonical_combinding_class,
	 $bidi_class,
	 $decomposition,
	 $numeric,
	 $bidi_mirrored,
	 $unicode_1_name,
	 $iso_comment,
	 $simple_uppercase_mapping,
	 $simple_lowercase_mapping,
	 $simple_titlecase_mapping
	 ) = split ";";
    my $i = hex $codepoint;
    if ($general_category) {
	$general_category_arr[$i] = $general_category;
    }
    else {
	$general_category_arr[$i] = "None";
    }
}

print "#include <stdlib.h>\n";
print "#include <cutext/wccat.h>\n";
print "#define C(x) CUTEXT_WCCAT_##x\n";
if (0) {
    print "char cupriv_ucs_general_category[] = {";
    my $i = 7;
    foreach (@general_category_arr) {
	if (++$i == 8) {
	    print "\n";
	    $i = 0;
	}
	if ($_) {
	    uc;
	    print "C($_),";
	}
	else {
	    print "0,";
	}
    }
    print "\n};\n";
}
else {
    my $char_cnt = $#general_category_arr + 1;
    my $nonuniform_cnt = 0;
    my @is_nonuniform_arr;
    for (my $i_blk = 0; $i_blk < $blk_cnt; ++$i_blk) {
	my $blk_cat = $general_category_arr[$i_blk*$blk_size];
	my $i;
	for ($i = 0; $i < $blk_size; ++$i) {
	    my $n = $i_blk*$blk_size + $i;
	    last unless $general_category_arr[$n] eq $blk_cat;
	}
	if ($i < $blk_size) {
	    ++$nonuniform_cnt;
	    $is_nonuniform_arr[$i_blk] = 1;
	    print "static char b_", $i_blk, "[] = {";
	    for ($i = 0; $i < $blk_size; ++$i) {
		my $n = $i_blk*$blk_size + $i;
		print "\n" if ($i % 8 == 0);
		my $gcat = $general_category_arr[$n];
		if ($gcat) {
		    print "C(", uc($gcat), "),";
		}
		else {
		    print "0,";
		}
	    }
	    print "};\n";
	}
    }
    my $nonunif_frac = $nonuniform_cnt/$blk_cnt;
    my $est = $blk_cnt*8 + $nonuniform_cnt*$blk_size;
    print STDERR "Non-uniform blocks: $nonuniform_cnt/$blk_cnt = $nonunif_frac\n";
    print STDERR "Table size est.: $est\n";
    print STDERR "Table size unblocked: $char_cnt\n";

    print "struct cutextP_wctype_table_entry_s { char cat; char *sub; };\n";
    print "struct cutextP_wctype_table_entry_s cutextP_wctype_table[] = {\n";
    for (my $i_blk = 0; $i_blk < $blk_cnt; ++$i_blk) {
	if ($is_nonuniform_arr[$i_blk]) {
	    print "{0,b_$i_blk},\n";
	}
	else {
	    my $gcat = $general_category_arr[$i_blk*$blk_size];
	    if ($gcat) {
		print "{C(", uc($gcat), "),NULL},\n";
	    }
	    else {
		print "{0,NULL},\n";
	    }
	}
    }
    print "};\n";
}
