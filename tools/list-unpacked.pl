#! /usr/bin/env perl
use strict;
use File::Basename qw(dirname);
my $srcdir = dirname(dirname($0));

my %manifest = ();

sub is_manifest($)
{
    my ($path) = @_;
    while ($path ne '.') {
	return 1 if $manifest{$path};
	$path = dirname($path);
	die "Unexpected absolute path $_[0]" if $_ eq '/';
    }
    return 0;
}

# Read in the manifest.
#
open MANIFEST, "MANIFEST" or die $!;
while (<MANIFEST>) {
    chomp;
    s/^$srcdir\/?//;
    $manifest{$_} = 1;
}
close MANIFEST;

# Scan source directory and match against manifest.
#
open FIND, "-|", "find", $srcdir, "-type", "f" or die $!;
while (<FIND>) {
    chomp;
    s/^$srcdir\/?//;
    next if /^(\.git|attic|autom4te\.cache)\//;
    next if /(^|\/)(\.gitignore|tmp\.)/;
    next if /(^|\/)\.[^\/]+\.swp$/;
    next if /(^|\/)scratch\//;
    next if /(~|\.bak)$/;
    next if is_manifest($_);
    print "$_\n";
}
close FIND;
