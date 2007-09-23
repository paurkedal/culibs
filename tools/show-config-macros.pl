#! /usr/bin/env perl
use strict;

my $detect_prefix = 1;
my $detect_cond = 1;

my %macro_to_files;

open FL, "find cu cu??* -name '*.h' -o -name '*.c'|";
while (my $path = <FL>) {
    chop $path;
    next if $path =~ /(^|\/)scratch\//;
    open F, $path;
    while (<F>) {
	if ($detect_prefix) {
	    while (/(CUCONF_[A-Z_0-9]+)/) {
		${$macro_to_files{$1}}{$path} = 1;
		s/$1//g;
	    }
	}
	if ($detect_cond) {
	    if (/#[ \t]*ifn?def[ \t]+([a-zA-Z_0-9]+)/) {
		${$macro_to_files{$1}}{$path} = 1;
	    }
	    while (/.*defined\([ \t]*([a-zA-Z_0-9]+)[ \t]*\)/) {
		${$macro_to_files{$1}}{$path} = 1;
		s/$1//g;
	    }
	}
    }
    close F;
}

foreach (sort (keys %macro_to_files)) {
    next if /_H\z/ && not /_HAVE_/;
    print "$_\n";
    foreach my $f (sort (keys %{$macro_to_files{$_}})) {
	print "    $f\n";
    }
}

