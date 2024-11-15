{
  stdenvNoCC,
  nix-gitignore,
  tex,
  gnumake,
}:

stdenvNoCC.mkDerivation {
  pname = "latex-template";
  version = "1.0.0";

  src = nix-gitignore.gitignoreSource [ ] ../.;

  nativeBuildInputs = [
    gnumake
    tex
  ];

  # Avoid luatex failing due to non-writable cache.
  TEXMFVAR = "/tmp/texlive/";
  TEXTMFHOME = "/tmp/texlive/";

  installPhase = ''
    mkdir -p $out
    install -m 0644 diplom.pdf $out/
  '';
}
