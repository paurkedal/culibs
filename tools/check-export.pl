#! /usr/bin/env perl
use strict;

my %special_allowed_words;
$special_allowed_words{$_} = 1 foreach qw(
    __bss_start
    _edata
    _end
    _fini
    _init
);
my $header = "Disallowed symbols:\n";

sub check_lib($$)
{
    my ($lib, $allowed) = @_;
    $lib = $lib.".so";
    open NM, "-|", "nm", ".libs/$lib" or die $!;
    while (<NM>) {
	chomp;
	if (/([0-9a-f]+)?\s+(\w)\s+(\S+)/) {
	    my ($addr, $type, $symbol) = ($1, $2, $3);
	    if ($type =~ /[ABDT]/) {
		if (!($symbol =~ $allowed ||
		      $special_allowed_words{$symbol})) {
		    if ($header) {
			print $header;
			$header = undef;
		    }
		    print "\t$lib: $type $symbol\n";
		}
	    }
	} else { die "Can not parse line '$_'"; }
    }
}

check_lib("libcubase",
    qr/^(cuconf|CUCONF|cu|CU|cucon|CUCON|cudyn|CUDYN|cuex|CUEX)P?_/);
check_lib("libcuex", qr/^(cuex|CUEX|cudyn|CUDYN)P?_/);
check_lib("libcuos", qr/^(cuos|CUOS)P?_/);
check_lib("libcuflow", qr/^(cuflow|CUFLOW)P?_/);
check_lib("libcusto", qr/^(custo|CUSTO)P?_/);
check_lib("libcugra", qr/^(cugra|CUGRA)P?_/);

exit $header? 0 : 1;
