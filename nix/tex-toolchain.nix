# Returns a tex toolchain with all packages needed for this thesis.
# This is more lightweight than using `pkgs.texliveFull`.

{ pkgs }:
with pkgs;
texlive.combine {
  inherit (texlive)
    scheme-small
    biber
    biblatex
    csquotes
    detex # Only for Makefile convenience scripts; not for the actual build
    hyphenat
    lastpage
    latexmk
    siunitx
    todonotes
    xpatch
    ;
}
