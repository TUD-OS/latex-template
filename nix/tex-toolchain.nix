{ pkgs }:
with pkgs;
texlive.combine {
  inherit (texlive) scheme-small
    biber
    biblatex
    csquotes
    detex # Only for Makefile convenience scripts; not for the actual build
    hyphenat
    lastpage
    latexmk
    siunitx
    todonotes
    xpatch;
}
