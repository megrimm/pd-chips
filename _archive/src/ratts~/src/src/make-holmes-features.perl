#!/usr/bin/perl -w

# Usage: $0 < phfeat.h > holmes-features.pd

while (<>) {
  next unless (/^\#define\s+(\w+)\s+(0x\d+)/);
  $phfeat{$1} = log(hex($2)) / log(2);
}

##-- output
print <<'EOF';
#N canvas 29 29 440 551 10;
#X text 20 10 holmes-features.pd : index constants for holmes-feat
;
#X text 120 28 Bryan Jurish <moocow@ling.uni-potsdam.de>;
#X obj 35 50 loadbang;
EOF


##-- objects
$i = $i0 = 2;
$x = 170;
$y = 50;
$yincr = 20;
$xincr = 35;
foreach (sort { $phfeat{$a} <=> $phfeat{$b} } keys %phfeat) {
  $i += 2;
  $y += $yincr;
  print
    ("#X obj $x $y f ", $phfeat{$_}+1, ";\n",
     "#X obj ", $x + $xincr, " $y v hf.$_;\n");
}

##-- connections
for ($c = $i0+1; $c < $i; $c += 2) {
  print
    ("#X connect $i0 0 $c 0;\n",
     "#X connect $c 0 ", $c+1, " 0;\n");
}
