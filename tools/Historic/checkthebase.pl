#!/bin/perl


# make sure the $CURENVBASE (passed as the first argument) is set somewhere in the
# current tree.

#$base = $ARGV[0];               # bug? I don't know why this doesn't work on NT
$base = $ENV{BASE};
$pwd = `pwd`;
chop($pwd);

chdir($base);			#  to account for  symlinks on Unix
$basedir = `pwd`;
chop($basedir);

print STDERR "base: $basedir\n";
print STDERR "pwd: $pwd\n";

if ($pwd =~ /^$basedir/)
{
    exit(1);
}

exit 0;
