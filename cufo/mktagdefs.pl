#! /usr/bin/env perl
use Getopt::Long;
use Pod::Usage;

my $out_path;
GetOptions('o=s' => \$out_path) and $out_path or pod2usage();

while (<>) {
    if (/extern cufo_tag_t cufo_([ib])_(\w+);/) {
	push(@tags, [$1, $2]);
    }
}

open O, '>', $out_path;
print O "#include <cufo/tagdefs.h>\n\n";
foreach $tag (@tags) {
    my ($t, $n) = @$tag;
    print O "cufo_tag_t cufo_${t}_${n};\n";
}
print O "\nvoid\ncufo_tagdefs_init(void)\n{\n";
print O "\tcufo_tagspace_t ns = cufo_culibs_tagspace();\n";
foreach $tag (@tags) {
    my ($t, $n) = @$tag;
    if ($t eq 'i') { $tag_flags = 'CUFO_TAGHINT_INLINE'; }
    else           { $tag_flags = 'CUFO_TAGHINT_BLOCK'; }
    print O "\tcufo_${t}_${n} = cufo_tag(ns, \"$n\", $tag_flags);\n";
}
print O "}\n";

__END__

=head1 NAME

 mktagdefs.pl

=head1 SYNOPSIS

 mktagdefs.pl -o OUTPUT [INPUT...]
