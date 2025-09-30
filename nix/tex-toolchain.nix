# Returns a tex toolchain with all packages needed for this thesis.
# This is more lightweight than using `pkgs.texliveFull`.

{ pkgs }:
with pkgs;
texlive.combine {
  inherit (texlive)
    biber
    biblatex
    csquotes
    detex # Only for Makefile convenience scripts; not for the actual thesis
    hyphenat
    lastpage
    latexmk
    scheme-small
    scrhack
    lscapeenhanced
    setspaceenhanced
    siunitx
    todonotes
    xpatch
    tudscr
    newunicodechar
    ;
}
