#!/opt/local/bin/perl

my $filelist = $ARGV[0];

my @files = (split(/\n/, `cat $filelist`));

sub bin2str {
    my @list = unpack( 'H2', $_[0] );
    my $result = sprintf( "0x%s", $list[0] );
    return $result;
}

open(OUT, ">", "src/ramdisk_files.c");

print OUT "\nramdisk_t ramdisk_files[] = {\n";

foreach $f (@files) {

    my $file = "$f.gz";

    my $struct = $f;
    $struct =~ s/\//_/g;
    $struct =~ s/\./_/g;

    my $size = $sizes{$f};
    my $orig_size = $orig_sizes{$f};

    print "  $f\n";

    print OUT "    {\n";
    print OUT "        \"$f\",\n";
    print OUT "        0,\n";
    print OUT "        0,\n";
    print OUT "        0,\n";
    print OUT "    },\n";

    close(FILE);
}

print OUT "    {0},\n";
print OUT "};\n";

close(OUT);
