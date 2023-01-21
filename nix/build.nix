{ stdenvNoCC, gitignoreSource, tex }:

stdenvNoCC.mkDerivation {
  pname = "latex-template";
  version = "1.0.0";

  src = gitignoreSource ../.;

  nativeBuildInputs = [
    tex
  ];

  doConfigure = false;

  # Avoid luatex failing due to non-writable cache.
  TEXMFVAR = "/tmp/texlive/";
  TEXTMFHOME = "/tmp/texlive/";

  buildPhase = ''
    make
  '';

  installPhase = ''
    mkdir -p $out
    install -m 0644 diplom.pdf $out/
  '';
}
