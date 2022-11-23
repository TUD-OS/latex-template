# make sure that "makeglossaries" works with latexmk
# # https://tex.stackexchange.com/questions/1226/how-to-make-latexmk-use-makeglossaries/44316#44316

add_cus_dep('glo', 'gls', 0, 'run_makeglossaries');
add_cus_dep('acn', 'acr', 0, 'run_makeglossaries');

sub run_makeglossaries {
  if ( $silent ) {
    system "makeglossaries -q -s '$_[0].ist' '$_[0]'";
  }
  else {
    system "makeglossaries -s '$_[0].ist' '$_[0]'";
  };
}

push @generated_exts, 'glo', 'gls', 'glg';
push @generated_exts, 'acn', 'acr', 'alg';

# -----------------------------------

# Clean everything.
# https://tex.stackexchange.com/questions/84006
# https://tex.stackexchange.com/questions/83341
$clean_ext .= ' %R.ist %R.xdy %R.bbl %R.glsdefs %R.run.xml %R.lol %R.tdo';
