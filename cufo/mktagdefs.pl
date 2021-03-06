#! /usr/bin/env perl
# Part of the culibs project, <http://www.eideticdew.org/culibs/>.
# Copyright (C) 2008  Petter Urkedal <urkedal@nbi.dk>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use Getopt::Long;
use Pod::Usage;

my $out_path;
GetOptions('o=s' => \$out_path) and $out_path or pod2usage();

my @tags;
while (<>) {
    if (/extern cufo_tag_t cufoT_(\w+);/) {
	my $name = $1;
	push(@tags, $name);
    }
}

open O, '>', $out_path or die $!;
print O "#include <cufo/tagdefs.h>\n\n";
foreach my $name (@tags) {
    print O "cufo_tag_t cufoT_${name};\n";
}
print O "\nvoid\ncufoP_tagdefs_init(void)\n{\n";
print O "\tcufo_namespace_t ns = cufo_culibs_namespace();\n";
foreach my $name (@tags) {
    my $xml_name = $name; $xml_name =~ tr/_/-/;
    print O "\tcufoT_${name} = cufo_tag(ns, \"$xml_name\");\n";
}
print O "}\n";

__END__

=head1 NAME

 mktagdefs.pl

=head1 SYNOPSIS

 mktagdefs.pl -o OUTPUT [INPUT...]
