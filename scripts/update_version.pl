#!/usr/bin/perl
#---------------------
#
# Change directory to the source_dir, get information about the repo from git, and update a
# C file with that information when it is different from the existing information.
#
use strict vars;

my $c_file;
my $c_file_version;
my $describe;
my $source_dir;
my $version;

( $source_dir , $c_file ) = @ARGV;

chdir($source_dir);

$describe = `git describe --always --dirty --tags`;
chomp $describe;

$version = sprintf("const char *git_version=\"%s\";\n", $describe);

#
# Attempt to read the c_file to compare against existing string.
#
if ( open(C_FILE, "$c_file")) {
    $c_file_version = <C_FILE>;
    close C_FILE;
}
else {
    $c_file_version = "<file not found>";
}

if ($version ne $c_file_version) {
    open  C_FILE, ">$c_file";
    print C_FILE "$version";
    close C_FILE;
}

exit 0;
