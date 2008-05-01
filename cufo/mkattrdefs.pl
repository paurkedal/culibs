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

my $out_path;
GetOptions('o=s' => \$out_path) and $out_path or die "Bad usage";

my @attrdefs;
my $current_enum = undef;
while (<>) {
    if (/^#define cufoA_(\w+)\b.*\bCUFO_ATTR_(CSTR|INT|ENUM)\b/) {
	my $name = $1;
	my $type = lc($2);
	if ($type eq 'enum') {
	    $current_enum = [];
	    push(@attrdefs, [$name, $type, $current_enum]);
	} else {
	    $current_enum = undef;
	    push(@attrdefs, [$name, $type]);
	}
    } elsif (/^\s*\/\*:enum\s+\"(.*)\"\s+(\w+)\s*\*\//) {
	die "No previous enum" unless $current_enum;
	push(@$current_enum, [$1, $2]);
    }
}

open O, '>', $out_path or die $!;
print O "/* This file is generated by $0. */\n\n";
print O "#include <cufo/attrdefs.h>\n";
print O "#include <cufo/tag.h>\n";
print O "#include <cu/logging.h>\n\n";
foreach my $attrdef (@attrdefs) {
    my ($name, $type, $extra) = @$attrdef;
    print O "struct cufo_attr_s cufoP_attr_${name};\n";
    if ($type eq 'enum') {
	print O "char const *cufoP_attrvals_${name}(int i)\n{\n";
	print O "    switch (i) {\n";
	for my $val (@$extra) {
	    print O "\tcase $$val[1]: return \"$$val[0]\";\n";
	}
	print O "\tdefault: return NULL;\n    }\n}\n";
    }
}
print O "\nvoid\ncufoP_attrdefs_init(void)\n{\n";
print O "\tcufo_namespace_t ns = cufo_culibs_namespace();\n";
foreach my $attrdef (@attrdefs) {
    my ($name, $type, $extra) = @$attrdef;
    my $xml_name = $name; $xml_name =~ tr/_/-/;
    if ($type eq 'cstr' or $type eq 'int') {
	print O "\tcufo_attr_init(\&cufoP_attr_${name}, ns, \"${xml_name}\", "
	       ."cufo_attrtype_$type);\n";
    } elsif ($type eq 'enum') {
	print O "\tcufo_attr_init_enum(\&cufoP_attr_${name}, ns, "
	       ."\"${xml_name}\", cufoP_attrvals_${name});\n"
    }
}
print O "}\n";
