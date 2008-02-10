#! /usr/bin/env perl

use strict;

my %idrs;

while (<>) {
    foreach (/(\b(cu|cucon|cuflow|cuex|cudyn|cutext|custo)_[a-zA-Z_0-9]+)/ig) {
	$idrs{$_} = 1;
    }
}

for (sort keys %idrs) {
    print "$_\n";
}
