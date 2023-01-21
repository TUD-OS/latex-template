{ pkgs }:
with pkgs;
texlive.combine {
  inherit (texlive) scheme-small
    biber
    biblatex
    csquotes
    hyphenat
    lastpage
    latexmk
    siunitx
    todonotes
    xpatch;
}
