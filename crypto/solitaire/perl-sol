#!/usr/bin/perl
# 
# Perl implementation of Bruce Schneier's card cipher, "Solitaire".
# Paul Crowley <paul@hedonism.demon.co.uk>, 1999
#
# This program is adapted from Ian Goldberg's Perl implementation; I
# place my modifications in the public domain if I can though I'm not
# sure of the copyright status of the original
#
# It only really exists to verify the correctness of the C version.
# 
# http://www.hedonism.demon.co.uk/paul/solitaire/

sub V {
    $v=ord(substr($D,$_[0]))-32;
    $v>53?53:$v;
}

sub cycle_deck {
    $D =~ s/(.*)U$/U$1/; $D =~ s/U(.)/$1U/;
    $D =~ s/(.*)V$/V$1/; $D =~ s/V(.)/$1V/;
    $D =~ s/(.*)V$/V$1/; $D =~ s/V(.)/$1V/;
    $D =~ s/(.*)([UV].*[UV])(.*)/$3$2$1/;
    $c=V(53);
    $D =~ s/(.{$c})(.*)(.)/$2$1$3/;
}


sub key_char {
    my $kc = shift;
    my $k = ord($kc) - 64;

    cycle_deck();
    $D =~ s/(.{$k})(.*)(.)/$2$1$3/;
    print $D, " after $kc\n" if $verbose;
}

sub encrypt_char {
    my $c = shift;
    my $prnd;
    do {
	cycle_deck();
	$prnd = V(V(0));
    } while( $prnd == 53 );
    my $ec = chr((ord($c)-13 + $prnd)%26+65);
    print $D, " $c -> $ec\n" if $verbose;
    return $ec;
}

$verbose = 0;

while( $ARGV[0] =~ /^-/ ) {
    $arg = shift;
    if ($arg eq '-v') {
	$verbose = 1;
    } else {
	die "Unrecognised flag $arg, stopped";
    }
}
$p = shift;
$o = shift;

$D = pack('C*',33..86);

$p =~ y/a-z/A-Z/;
$p =~ s/[A-Z]/key_char($&)/eg;

if ($o =~ /^\d+$/) {
    $o = 'A' x $o;
    $old_len = length($o);
    $o =~ s/./encrypt_char($&)/eg;
    $o =~ tr/A-Z//s;
    $cc = $old_len - length($o);
    $n = $old_len -1;
    #$np = $n / 26;
    #$sd = sqrt($np * (25 / 26));

    print "Coincidences: $cc / $n\n";
    #print( ($cc - $np)/$sd, " SDs from mean\n");
} else {
    $o =~ tr/a-z/A-Z/;
    $o =~ tr/A-Z//cd;
    $o .= 'X' while length($o)%5;
    $o =~ s/./encrypt_char($&)/eg;
    $o =~ s/.{5}/$& /g;
    print $o, "\n";
}
